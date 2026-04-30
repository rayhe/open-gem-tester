/*
 * OpenGemTester - Thermal Probe Header
 *
 * Manages the heated probe tip, NTC thermistor reading via ADS1115,
 * and thermal decay curve measurement + exponential fitting.
 */

#ifndef THERMAL_H
#define THERMAL_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <DallasTemperature.h>

class ThermalProbe {
public:
    ThermalProbe();

    /*
     * Initialize with pointers to shared hardware.
     * ads       - pointer to ADS1115 (must already be begin()'d)
     * heaterPin - GPIO controlling heater MOSFET gate
     * ds        - pointer to DallasTemperature (ambient sensor)
     */
    void init(Adafruit_ADS1115* ads, int heaterPin, DallasTemperature* ds);

    /* Turn heater MOSFET on/off directly. */
    void setHeater(bool on);

    /*
     * Read the NTC thermistor on ADS1115 AIN0 and return temperature in Celsius.
     * Uses the B-parameter equation for NTC 100K 3950.
     * Voltage divider: 3.3V -- 100K ref -- AIN0 -- NTC -- GND
     */
    float getProbeTemp();

    /*
     * Read ambient temperature from DS18B20.
     * Falls back to an estimate from the NTC if DS18B20 is absent.
     */
    float getAmbientTemp();

    /*
     * Sample the decay curve at maximum rate for the given duration.
     * Fills timestamps (in milliseconds from start) and temperatures arrays.
     * Returns the actual number of samples captured.
     * maxSamples must not exceed the caller's buffer size.
     */
    int measureDecay(float* timestamps, float* temperatures,
                     int maxSamples, int durationMs);

    /*
     * Fit an exponential decay curve to the sampled data.
     *
     * Model: T(t) = T_amb + (T_0 - T_amb) * exp(-t / tau)
     *
     * Uses linearized least-squares regression:
     *   ln(T - T_amb) = ln(T_0 - T_amb) - t / tau
     *   => y = a + b*t  where b = -1/tau
     *
     * Returns tau in milliseconds (positive), or -1 on failure.
     * ambientTemp is the baseline temperature in Celsius.
     */
    float fitExponentialDecay(float* timestamps, float* temperatures,
                              int count, float ambientTemp);

private:
    Adafruit_ADS1115* _ads;
    DallasTemperature* _ds;
    int _heaterPin;
    bool _ds18b20Present;

    /*
     * NTC thermistor conversion functions.
     * Circuit: 3.3V -- R_ref (100K) -- node (AIN0) -- R_ntc -- GND
     */

    /* Convert ADC voltage to NTC resistance in ohms. */
    float voltageToResistance(float voltage);

    /* Convert NTC resistance to temperature in Celsius using B-parameter equation. */
    float resistanceToTempC(float resistance);

    /* Read raw voltage from ADS1115 channel 0. */
    float readADS1115Voltage(int channel);
};

#endif // THERMAL_H
