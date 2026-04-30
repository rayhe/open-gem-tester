/*
 * OpenGemTester - Calibration Manager Header
 *
 * Handles calibration data storage (ESP32 NVS via Preferences),
 * and the mapping from measured decay time constant (tau) to
 * estimated thermal conductivity.
 *
 * Calibration uses known reference materials:
 *   Air:     k = 0.026 W/mK (very slow decay, large tau)
 *   Glass:   k = 1.0 W/mK   (slow decay, moderate tau)
 *   Diamond: k = 2200 W/mK  (fast decay, small tau) [optional]
 *
 * The tau-to-k mapping uses an inverse relationship:
 *   k = A / tau + B
 * where A and B are determined from the calibration points.
 * This reflects the physics: higher conductivity pulls heat faster,
 * giving a shorter time constant.
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>

class CalibrationManager {
public:
    CalibrationManager();

    /* Load calibration from NVS. Returns true if valid calibration found. */
    bool loadCalibration();

    /* Save current calibration to NVS. */
    void saveCalibration();

    /* Check if a valid calibration exists. */
    bool isCalibrated();

    /* Set the measured tau for air (baseline). */
    void setAirTau(float tau_ms);

    /* Set the measured tau for glass reference. */
    void setGlassTau(float tau_ms);

    /* Set the measured tau for diamond reference (optional). */
    void setDiamondTau(float tau_ms);

    /*
     * Build the tau-to-k mapping coefficients from the reference points.
     * Must be called after setting at least air and glass taus.
     */
    void buildMapping();

    /*
     * Convert a measured tau (ms) to estimated thermal conductivity (W/mK).
     * Uses the calibration mapping: k = A / tau + B
     */
    float tauToThermalConductivity(float tau_ms);

    /*
     * Get the age of the calibration in seconds.
     * Returns 0 if not calibrated.
     */
    unsigned long getCalibrationAge();

private:
    // Measured tau values for reference materials
    float _tauAir;
    float _tauGlass;
    float _tauDiamond;
    bool _hasDiamond;

    // Mapping coefficients: k = _coeffA / tau + _coeffB
    float _coeffA;
    float _coeffB;

    // Calibration metadata
    bool _calibrated;
    unsigned long _calibTimestamp;  // millis() when calibration was performed

    // NVS namespace
    static const char* NVS_NAMESPACE;
};

#endif // CALIBRATION_H
