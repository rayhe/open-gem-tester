/*
 * OpenGemTester - Calibration Manager Implementation
 *
 * Maps measured thermal decay time constant (tau in ms) to estimated
 * thermal conductivity (k in W/mK) using known reference points.
 *
 * Physics background:
 *   The probe tip cools by conducting heat into the stone. The cooling
 *   follows an exponential decay with time constant tau. Higher thermal
 *   conductivity materials draw heat faster, yielding a smaller tau.
 *
 *   The relationship between tau and k is approximately inverse:
 *     k ~ C / tau
 *   where C depends on probe geometry, contact area, and thermal mass.
 *
 *   We use a two-parameter model: k = A / tau + B
 *   Fit from two or three known reference points (air, glass, diamond).
 *
 * Calibration data is stored in ESP32 NVS (non-volatile storage) using
 * the Preferences library so it survives power cycles.
 */

#include "calibration.h"
#include <Preferences.h>

// Known thermal conductivities of reference materials (W/mK)
static const float K_AIR     = 0.026f;
static const float K_GLASS   = 1.0f;
static const float K_DIAMOND = 2200.0f;

const char* CalibrationManager::NVS_NAMESPACE = "gemcal";

CalibrationManager::CalibrationManager()
    : _tauAir(0.0f)
    , _tauGlass(0.0f)
    , _tauDiamond(0.0f)
    , _hasDiamond(false)
    , _coeffA(0.0f)
    , _coeffB(0.0f)
    , _calibrated(false)
    , _calibTimestamp(0)
{}

bool CalibrationManager::loadCalibration() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);  // read-only

    _calibrated = prefs.getBool("valid", false);
    if (!_calibrated) {
        prefs.end();
        Serial.println("Calibration: no valid data in NVS");
        return false;
    }

    _tauAir     = prefs.getFloat("tau_air", 0.0f);
    _tauGlass   = prefs.getFloat("tau_glass", 0.0f);
    _tauDiamond = prefs.getFloat("tau_diamond", 0.0f);
    _hasDiamond = prefs.getBool("has_diamond", false);
    _coeffA     = prefs.getFloat("coeff_a", 0.0f);
    _coeffB     = prefs.getFloat("coeff_b", 0.0f);
    _calibTimestamp = prefs.getULong("timestamp", 0);

    prefs.end();

    Serial.printf("Calibration loaded: air=%.1f ms, glass=%.1f ms, A=%.2f, B=%.4f\n",
                  _tauAir, _tauGlass, _coeffA, _coeffB);

    if (_hasDiamond) {
        Serial.printf("  Diamond reference: tau=%.1f ms\n", _tauDiamond);
    }

    return true;
}

void CalibrationManager::saveCalibration() {
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, false);  // read-write

    prefs.putBool("valid", _calibrated);
    prefs.putFloat("tau_air", _tauAir);
    prefs.putFloat("tau_glass", _tauGlass);
    prefs.putFloat("tau_diamond", _tauDiamond);
    prefs.putBool("has_diamond", _hasDiamond);
    prefs.putFloat("coeff_a", _coeffA);
    prefs.putFloat("coeff_b", _coeffB);
    prefs.putULong("timestamp", millis());

    prefs.end();

    Serial.println("Calibration saved to NVS");
}

bool CalibrationManager::isCalibrated() {
    return _calibrated && _coeffA != 0.0f;
}

void CalibrationManager::setAirTau(float tau_ms) {
    _tauAir = tau_ms;
    Serial.printf("Calibration: air tau set to %.1f ms\n", tau_ms);
}

void CalibrationManager::setGlassTau(float tau_ms) {
    _tauGlass = tau_ms;
    Serial.printf("Calibration: glass tau set to %.1f ms\n", tau_ms);
}

void CalibrationManager::setDiamondTau(float tau_ms) {
    _tauDiamond = tau_ms;
    _hasDiamond = true;
    Serial.printf("Calibration: diamond tau set to %.1f ms\n", tau_ms);
}

void CalibrationManager::buildMapping() {
    /*
     * Fit the model: k = A / tau + B
     *
     * With two points (air and glass):
     *   k_air   = A / tau_air   + B
     *   k_glass = A / tau_glass + B
     *
     * Subtracting:
     *   k_glass - k_air = A * (1/tau_glass - 1/tau_air)
     *   A = (k_glass - k_air) / (1/tau_glass - 1/tau_air)
     *   B = k_air - A / tau_air
     *
     * With three points (including diamond), we use a least-squares fit
     * to minimize error across all three reference materials.
     */

    if (_tauAir <= 0.0f || _tauGlass <= 0.0f) {
        Serial.println("Calibration: missing air or glass reference, cannot build mapping");
        return;
    }

    if (_hasDiamond && _tauDiamond > 0.0f) {
        // Three-point least squares fit for k = A / tau + B
        // Let x_i = 1/tau_i, y_i = k_i
        // Linear regression: y = A*x + B

        float x[3] = { 1.0f / _tauAir, 1.0f / _tauGlass, 1.0f / _tauDiamond };
        float y[3] = { K_AIR, K_GLASS, K_DIAMOND };

        float sumX = 0, sumY = 0, sumXX = 0, sumXY = 0;
        for (int i = 0; i < 3; i++) {
            sumX  += x[i];
            sumY  += y[i];
            sumXX += x[i] * x[i];
            sumXY += x[i] * y[i];
        }

        float n = 3.0f;
        float denom = n * sumXX - sumX * sumX;
        if (fabsf(denom) < 1e-20f) {
            Serial.println("Calibration: degenerate fit, falling back to two-point");
            goto two_point;
        }

        _coeffA = (n * sumXY - sumX * sumY) / denom;
        _coeffB = (sumY - _coeffA * sumX) / n;
    } else {
two_point:
        // Two-point fit using air and glass
        float invDiff = (1.0f / _tauGlass) - (1.0f / _tauAir);
        if (fabsf(invDiff) < 1e-20f) {
            Serial.println("Calibration: air and glass taus are too similar!");
            return;
        }

        _coeffA = (K_GLASS - K_AIR) / invDiff;
        _coeffB = K_AIR - _coeffA / _tauAir;
    }

    _calibrated = true;
    _calibTimestamp = millis();

    Serial.printf("Calibration mapping built: k = %.4f / tau + %.6f\n", _coeffA, _coeffB);

    // Sanity check: verify reference points
    float kAirCheck = tauToThermalConductivity(_tauAir);
    float kGlassCheck = tauToThermalConductivity(_tauGlass);
    Serial.printf("  Verify: air -> %.4f W/mK (expect %.4f)\n", kAirCheck, K_AIR);
    Serial.printf("  Verify: glass -> %.4f W/mK (expect %.4f)\n", kGlassCheck, K_GLASS);

    if (_hasDiamond && _tauDiamond > 0.0f) {
        float kDiamondCheck = tauToThermalConductivity(_tauDiamond);
        Serial.printf("  Verify: diamond -> %.1f W/mK (expect %.1f)\n", kDiamondCheck, K_DIAMOND);
    }
}

float CalibrationManager::tauToThermalConductivity(float tau_ms) {
    if (!_calibrated || tau_ms <= 0.0f) {
        return 0.0f;
    }

    float k = _coeffA / tau_ms + _coeffB;

    // Clamp to physically reasonable values
    if (k < 0.0f) k = 0.0f;
    if (k > 5000.0f) k = 5000.0f;  // nothing natural exceeds ~3000 W/mK

    return k;
}

unsigned long CalibrationManager::getCalibrationAge() {
    if (!_calibrated || _calibTimestamp == 0) {
        return 0;
    }
    return (millis() - _calibTimestamp) / 1000;
}
