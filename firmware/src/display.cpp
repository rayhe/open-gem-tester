/*
 * OpenGemTester - Display Implementation
 *
 * All UI screens for the 128x64 SSD1306 OLED.
 * Uses Adafruit GFX primitives for text, lines, rectangles, and bars.
 */

#include "display.h"

static const int SCREEN_WIDTH  = 128;
static const int SCREEN_HEIGHT = 64;
static const int OLED_ADDR     = 0x3C;
static const char* FW_VERSION  = "v1.0";

GemDisplay::GemDisplay()
    : _oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool GemDisplay::init() {
    if (!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        return false;
    }
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
    _oled.display();
    return true;
}

void GemDisplay::showSplash() {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    // Centered title
    _oled.setCursor(10, 8);
    _oled.print("OpenGemTester");

    _oled.setCursor(46, 22);
    _oled.print(FW_VERSION);

    // Decorative line
    _oled.drawLine(10, 34, 118, 34, SSD1306_WHITE);

    _oled.setCursor(8, 42);
    _oled.print("Diamond & Gem Tester");

    _oled.setCursor(22, 54);
    _oled.print("Open Source HW");

    _oled.display();
}

void GemDisplay::showReady() {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    _oled.setCursor(4, 4);
    _oled.print("OpenGemTester Ready");
    _oled.drawLine(0, 14, 128, 14, SSD1306_WHITE);

    _oled.setTextSize(1);
    _oled.setCursor(8, 24);
    _oled.print("Short press: Test");

    _oled.setCursor(8, 38);
    _oled.print("Long press:  Calibrate");

    // Small status bar at bottom
    _oled.drawLine(0, 54, 128, 54, SSD1306_WHITE);
    _oled.setCursor(4, 56);
    _oled.print("BLE active");

    _oled.display();
}

void GemDisplay::showHeating(float currentTemp, float targetTemp) {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    _oled.setCursor(4, 4);
    _oled.print("Heating Probe...");

    // Current temperature
    _oled.setCursor(4, 20);
    _oled.printf("Now:    %.1f C", currentTemp);

    _oled.setCursor(4, 32);
    _oled.printf("Target: %.1f C", targetTemp);

    // Progress bar
    float progress = 0.0f;
    if (targetTemp > currentTemp) {
        // Estimate progress based on how far we've heated from ambient
        // Assume we started roughly at ambient (targetTemp - 50)
        float startTemp = targetTemp - 50.0f;
        if (currentTemp > startTemp) {
            progress = (currentTemp - startTemp) / (targetTemp - startTemp);
        }
    } else {
        progress = 1.0f;
    }
    drawBarGraph(progress, 0.0f, 1.0f, 48);

    _oled.display();
}

void GemDisplay::showTouchPrompt() {
    _oled.clearDisplay();

    _oled.setTextSize(2);
    _oled.setCursor(14, 8);
    _oled.print("TOUCH");

    _oled.setCursor(8, 28);
    _oled.print("STONE!");

    _oled.setTextSize(1);
    _oled.setCursor(4, 52);
    _oled.print("Probe is hot - ready");

    _oled.display();
}

void GemDisplay::showMeasuring(const char* phase, float progress) {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    _oled.setCursor(4, 4);
    _oled.print("Measuring...");

    _oled.setCursor(4, 20);
    _oled.print(phase);

    // Percentage
    _oled.setCursor(4, 34);
    _oled.printf("%d%%", (int)(progress * 100.0f));

    // Progress bar
    drawBarGraph(progress, 0.0f, 1.0f, 48);

    _oled.display();
}

void GemDisplay::showResult(const char* materialName, float confidence,
                             float thermalK, float resistance) {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    // Material name (larger if short enough)
    if (strlen(materialName) <= 10) {
        _oled.setTextSize(2);
        _oled.setCursor(4, 0);
        _oled.print(materialName);
        _oled.setTextSize(1);
    } else {
        _oled.setCursor(4, 0);
        _oled.print(materialName);
    }

    // Confidence bar
    _oled.setCursor(4, 18);
    _oled.printf("Conf: %d%%", (int)(confidence * 100.0f));
    drawBarGraph(confidence, 0.0f, 1.0f, 28);

    // Thermal conductivity
    _oled.setCursor(4, 40);
    _oled.printf("k=%.0f W/mK", thermalK);

    // Electrical resistance with unit
    char rBuf[20];
    formatResistance(resistance, rBuf, sizeof(rBuf));
    _oled.setCursor(4, 52);
    _oled.printf("R=%s", rBuf);

    _oled.display();
}

void GemDisplay::showCalibrationStep(int step, const char* instruction) {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    _oled.setCursor(4, 0);
    _oled.printf("CAL Step %d", step);
    _oled.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // Word-wrap the instruction text manually
    _oled.setCursor(4, 16);
    _oled.print(instruction);

    _oled.display();
}

void GemDisplay::showError(const char* message) {
    _oled.clearDisplay();
    _oled.setTextSize(1);

    _oled.setCursor(4, 4);
    _oled.print("!! ERROR !!");
    _oled.drawLine(0, 14, 128, 14, SSD1306_WHITE);

    _oled.setCursor(4, 20);
    _oled.print(message);

    _oled.display();
}

void GemDisplay::drawBarGraph(float value, float minVal, float maxVal, int y) {
    /*
     * Horizontal progress bar, full width with 2px margin.
     * Outline rectangle with filled portion proportional to value.
     */
    int barX = 4;
    int barW = SCREEN_WIDTH - 8;
    int barH = 8;

    // Clamp
    if (value < minVal) value = minVal;
    if (value > maxVal) value = maxVal;

    float frac = (value - minVal) / (maxVal - minVal);
    int fillW = (int)(frac * (float)(barW - 2));

    _oled.drawRect(barX, y, barW, barH, SSD1306_WHITE);
    if (fillW > 0) {
        _oled.fillRect(barX + 1, y + 1, fillW, barH - 2, SSD1306_WHITE);
    }
}

void GemDisplay::formatResistance(float ohms, char* buf, int bufLen) {
    if (ohms >= 1e9f) {
        snprintf(buf, bufLen, "%.1fG", ohms / 1e9f);
    } else if (ohms >= 1e6f) {
        snprintf(buf, bufLen, "%.1fM", ohms / 1e6f);
    } else if (ohms >= 1e3f) {
        snprintf(buf, bufLen, "%.1fK", ohms / 1e3f);
    } else {
        snprintf(buf, bufLen, "%.0f", ohms);
    }
}
