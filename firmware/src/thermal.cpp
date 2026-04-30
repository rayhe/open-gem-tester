/*
 * OpenGemTester - Thermal Probe Implementation
 *
 * Core physics:
 * - NTC thermistor (100K, B=3950) in voltage divider with 100K reference resistor
 * - B-parameter equation: R(T) = R25 * exp(B * (1/T - 1/298.15))
 * - Exponential decay fitting via linearized least squares
 * - Thermal conductivity extracted from the decay time constant (tau)
 *
 * Higher thermal conductivity stones (diamond) pull heat away faster,
 * resulting in a shorter tau. Lower conductivity stones (glass, CZ)
 * allow slower cooling, yielding a longer tau.
 */

#include "thermal.h"

// NTC parameters
static const float NTC_B_COEFF    = 3950.0f;   // B-parameter for NTC 3950
static const float NTC_R25        = 100000.0f;  // resistance at 25C in ohms
static const float NTC_T25_K      = 298.15f;    // 25C in Kelvin
static const float R_REF          = 100000.0f;  // reference resistor in ohms
static const float V_SUPPLY       = 3.3f;       // supply voltage

// ADS1115 at gain=1: full scale +/- 4.096V, 16-bit signed
static const float ADS_VOLTS_PER_BIT = 0.000125f;  // 4.096 / 32768

ThermalProbe::ThermalProbe()
    : _ads(nullptr), _ds(nullptr), _heaterPin(-1), _ds18b20Present(false) {}

void ThermalProbe::init(Adafruit_ADS1115* ads, int heaterPin, DallasTemperature* ds) {
    _ads = ads;
    _heaterPin = heaterPin;
    _ds = ds;

    pinMode(_heaterPin, OUTPUT);
    digitalWrite(_heaterPin, LOW);

    // Check if DS18B20 is present
    if (_ds && _ds->getDeviceCount() > 0) {
        _ds18b20Present = true;
        _ds->requestTemperatures();
    }
}

void ThermalProbe::setHeater(bool on) {
    digitalWrite(_heaterPin, on ? HIGH : LOW);
}

float ThermalProbe::readADS1115Voltage(int channel) {
    if (!_ads) return 0.0f;
    int16_t raw = _ads->readADC_SingleEnded(channel);
    return raw * ADS_VOLTS_PER_BIT;
}

float ThermalProbe::voltageToResistance(float voltage) {
    /*
     * Voltage divider: V_supply -- R_ref -- node -- R_ntc -- GND
     * V_node = V_supply * R_ntc / (R_ref + R_ntc)
     * Solving for R_ntc:
     * R_ntc = R_ref * V_node / (V_supply - V_node)
     */
    if (voltage >= V_SUPPLY - 0.001f) return 1e9f;   // open circuit
    if (voltage <= 0.001f) return 0.1f;                // short circuit
    return R_REF * voltage / (V_SUPPLY - voltage);
}

float ThermalProbe::resistanceToTempC(float resistance) {
    /*
     * Simplified B-parameter (beta) equation for NTC thermistors:
     *   R(T) = R25 * exp(B * (1/T - 1/T25))
     *
     * Solving for T:
     *   1/T = 1/T25 + (1/B) * ln(R / R25)
     *   T = 1 / (1/T25 + ln(R/R25)/B)
     *
     * Returns temperature in Celsius.
     */
    if (resistance <= 0.0f) return -273.15f;

    float lnRatio = logf(resistance / NTC_R25);
    float invT = (1.0f / NTC_T25_K) + (lnRatio / NTC_B_COEFF);

    if (invT <= 0.0f) return -273.15f;

    return (1.0f / invT) - 273.15f;
}

float ThermalProbe::getProbeTemp() {
    float voltage = readADS1115Voltage(0);  // AIN0 = NTC divider
    float resistance = voltageToResistance(voltage);
    return resistanceToTempC(resistance);
}

float ThermalProbe::getAmbientTemp() {
    if (_ds18b20Present && _ds) {
        _ds->requestTemperatures();
        float t = _ds->getTempCByIndex(0);
        // DS18B20 returns -127 on read error
        if (t > -100.0f && t < 200.0f) {
            return t;
        }
    }

    /*
     * Fallback: use the NTC itself as an ambient estimate.
     * This is only valid before the probe is heated.
     * In practice, the DS18B20 should always be present.
     */
    return getProbeTemp();
}

int ThermalProbe::measureDecay(float* timestamps, float* temperatures,
                                int maxSamples, int durationMs) {
    /*
     * High-speed sampling of the thermal decay curve.
     * ADS1115 at 860 SPS gives ~1.16 ms per sample; I2C overhead
     * reduces effective rate to roughly 475 SPS (~2.1 ms per sample).
     * We sample as fast as possible and record timestamps.
     */
    if (!_ads || maxSamples <= 0) return 0;

    unsigned long startMicros = micros();
    unsigned long endMicros = startMicros + ((unsigned long)durationMs * 1000UL);
    int count = 0;

    while (count < maxSamples && micros() < endMicros) {
        unsigned long now = micros();
        float voltage = readADS1115Voltage(0);
        float resistance = voltageToResistance(voltage);
        float tempC = resistanceToTempC(resistance);

        timestamps[count] = (float)(now - startMicros) / 1000.0f;  // ms
        temperatures[count] = tempC;
        count++;

        // No explicit delay; ADC read is the bottleneck (~2 ms)
    }

    Serial.printf("Decay sampled: %d pts in %lu ms\n",
                  count, (micros() - startMicros) / 1000UL);
    return count;
}

float ThermalProbe::fitExponentialDecay(float* timestamps, float* temperatures,
                                         int count, float ambientTemp) {
    /*
     * Fit: T(t) = T_amb + (T_0 - T_amb) * exp(-t / tau)
     *
     * Linearize by taking the natural log of the temperature excess:
     *   deltaT = T(t) - T_amb
     *   ln(deltaT) = ln(T_0 - T_amb) + (-1/tau) * t
     *
     * This is a simple y = a + b*x linear regression where:
     *   y = ln(deltaT)
     *   x = t (timestamp in ms)
     *   slope b = -1/tau
     *   tau = -1/b
     *
     * We skip samples where deltaT <= 0 (noise below ambient).
     * We also skip the first few samples to avoid initial transient.
     */

    if (count < 10) return -1.0f;

    // Minimum temperature excess to include in the fit (avoids log(0) and noise)
    const float MIN_DELTA_T = 0.5f;

    // Skip the first 5 samples (initial contact transient)
    int startIdx = 5;

    // Accumulate sums for linear regression
    float sumX = 0.0f, sumY = 0.0f, sumXX = 0.0f, sumXY = 0.0f;
    int n = 0;

    for (int i = startIdx; i < count; i++) {
        float deltaT = temperatures[i] - ambientTemp;
        if (deltaT <= MIN_DELTA_T) continue;

        float x = timestamps[i];
        float y = logf(deltaT);

        sumX  += x;
        sumY  += y;
        sumXX += x * x;
        sumXY += x * y;
        n++;
    }

    if (n < 5) return -1.0f;  // not enough valid points

    // Linear regression: slope = (n*sumXY - sumX*sumY) / (n*sumXX - sumX*sumX)
    float denom = (float)n * sumXX - sumX * sumX;
    if (fabsf(denom) < 1e-10f) return -1.0f;

    float slope = ((float)n * sumXY - sumX * sumY) / denom;

    // slope = -1/tau, so tau = -1/slope
    if (slope >= 0.0f) return -1.0f;  // curve should be decaying (negative slope)

    float tau = -1.0f / slope;

    Serial.printf("Decay fit: n=%d valid pts, slope=%.6f, tau=%.1f ms\n",
                  n, slope, tau);

    return tau;
}
