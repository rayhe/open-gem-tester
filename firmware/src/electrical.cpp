/*
 * OpenGemTester - Electrical Probe Implementation
 *
 * Physics:
 * The probe tip is connected to 3.3V through a 10M ohm series resistor.
 * The stone sits on a grounded metal plate. The ADS1115 AIN1 measures the
 * voltage at the junction between the series resistor and the probe tip.
 *
 * Voltage divider:  3.3V -- R_series (10M) -- AIN1/probe -- stone -- GND plate
 *
 * If the stone is a perfect insulator (R >> R_series):
 *   V_read approaches 3.3V (no current flows)
 *
 * If the stone is a conductor (R << R_series):
 *   V_read approaches 0V (all voltage drops across R_series)
 *
 * Stone resistance: R_stone = R_series * V_read / (V_supply - V_read)
 *
 * This allows distinguishing:
 *   Diamond: insulator, R > 1G ohm, V_read near 3.3V
 *   Moissanite: semiconductor, R ~ 100K-10M ohm, V_read noticeably drops
 *   Metal: conductor, R < 1K ohm, V_read near 0V
 */

#include "electrical.h"

static const float V_SUPPLY = 3.3f;
static const float ADS_VOLTS_PER_BIT = 0.000125f;  // gain=1, +/-4.096V

// Conductivity thresholds (ohms)
static const float CONDUCTOR_THRESHOLD     = 100000.0f;     // 100K ohm
static const float INSULATOR_THRESHOLD     = 1000000000.0f; // 1G ohm

// Maximum buffer for median calculation
static const int MAX_ELEC_SAMPLES = 64;
static float elecSampleBuf[MAX_ELEC_SAMPLES];

ElectricalProbe::ElectricalProbe()
    : _ads(nullptr), _switchPin(-1), _seriesR(10e6f) {}

void ElectricalProbe::init(Adafruit_ADS1115* ads, int switchPin, float seriesResistance) {
    _ads = ads;
    _switchPin = switchPin;
    _seriesR = seriesResistance;

    pinMode(_switchPin, OUTPUT);
    digitalWrite(_switchPin, LOW);  // default: electrical mode off
}

void ElectricalProbe::enableElectricalMode() {
    digitalWrite(_switchPin, HIGH);
}

void ElectricalProbe::disableElectricalMode() {
    digitalWrite(_switchPin, LOW);
}

float ElectricalProbe::readVoltage() {
    if (!_ads) return 0.0f;
    int16_t raw = _ads->readADC_SingleEnded(1);  // AIN1
    return raw * ADS_VOLTS_PER_BIT;
}

float ElectricalProbe::measureResistance() {
    float v = readVoltage();

    /*
     * Voltage divider math:
     *   V_read = V_supply * R_stone / (R_series + R_stone)
     *   R_stone = R_series * V_read / (V_supply - V_read)
     *
     * Edge cases:
     *   V_read very close to V_supply: stone is open circuit (insulator)
     *   V_read very close to 0: stone is short circuit (conductor)
     */

    float vDiff = V_SUPPLY - v;

    // Protect against division by zero or near-zero
    if (vDiff < 0.001f) {
        // Essentially open circuit: return a very high resistance
        return 1e10f;
    }

    if (v < 0.001f) {
        // Essentially short circuit
        return 0.1f;
    }

    return _seriesR * v / vDiff;
}

float ElectricalProbe::averageResistance(int samples) {
    if (samples <= 0) return 0.0f;
    if (samples > MAX_ELEC_SAMPLES) samples = MAX_ELEC_SAMPLES;

    // Collect readings
    for (int i = 0; i < samples; i++) {
        elecSampleBuf[i] = measureResistance();
        delay(5);  // small delay between readings for settling
    }

    // Sort for median calculation
    sortFloats(elecSampleBuf, samples);

    /*
     * Outlier rejection using interquartile range (IQR).
     * Keep only values within [Q1 - 2*IQR, Q3 + 2*IQR].
     * Then return the median of the remaining values.
     */
    int q1Idx = samples / 4;
    int q3Idx = (3 * samples) / 4;
    float q1 = elecSampleBuf[q1Idx];
    float q3 = elecSampleBuf[q3Idx];
    float iqr = q3 - q1;
    float loBound = q1 - 2.0f * iqr;
    float hiBound = q3 + 2.0f * iqr;

    // Collect inliers
    float inliers[MAX_ELEC_SAMPLES];
    int inlierCount = 0;
    for (int i = 0; i < samples; i++) {
        if (elecSampleBuf[i] >= loBound && elecSampleBuf[i] <= hiBound) {
            inliers[inlierCount++] = elecSampleBuf[i];
        }
    }

    if (inlierCount == 0) {
        // All rejected? Just return the raw median
        return elecSampleBuf[samples / 2];
    }

    // Return median of inliers (already sorted since original was sorted)
    return inliers[inlierCount / 2];
}

ConductivityClass ElectricalProbe::classifyConductivity(float resistance) {
    if (resistance < CONDUCTOR_THRESHOLD) {
        return COND_CONDUCTOR;
    } else if (resistance > INSULATOR_THRESHOLD) {
        return COND_INSULATOR;
    } else {
        return COND_SEMICONDUCTOR;
    }
}

void ElectricalProbe::sortFloats(float* arr, int count) {
    // Insertion sort, fine for small N (<64)
    for (int i = 1; i < count; i++) {
        float key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}
