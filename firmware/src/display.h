/*
 * OpenGemTester - Display Header
 *
 * Wrapper around SSD1306 128x64 OLED for all UI screens.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

class GemDisplay {
public:
    GemDisplay();

    /* Initialize the OLED. Returns false if display not found. */
    bool init();

    /* Splash screen with project name and firmware version. */
    void showSplash();

    /* Idle screen: "Press button to test" */
    void showReady();

    /* Heating progress: current temp, target temp, progress bar. */
    void showHeating(float currentTemp, float targetTemp);

    /* Prompt user to touch the probe to the stone. */
    void showTouchPrompt();

    /* Measurement in progress: phase name and progress bar (0.0 - 1.0). */
    void showMeasuring(const char* phase, float progress);

    /*
     * Result screen: material name, confidence percentage,
     * thermal conductivity, and electrical resistance.
     */
    void showResult(const char* materialName, float confidence,
                    float thermalK, float resistance);

    /* Calibration step indicator and instruction text. */
    void showCalibrationStep(int step, const char* instruction);

    /* Error screen with message. */
    void showError(const char* message);

    /* Draw a horizontal bar graph at the given Y position. */
    void drawBarGraph(float value, float minVal, float maxVal, int y);

private:
    Adafruit_SSD1306 _oled;

    /* Helper: format resistance with appropriate unit suffix (ohm, K, M, G). */
    void formatResistance(float ohms, char* buf, int bufLen);
};

#endif // DISPLAY_H
