/*
 * OpenGemTester - Main Entry Point
 * Open-source ESP32-S3 gemstone/diamond tester
 *
 * Hardware: ESP32-S3 DevKitC-1, ADS1115 ADC, SSD1306 OLED,
 *           DS18B20 temp sensor, NTC thermistor probe, piezo buzzer,
 *           WS2812 NeoPixel, heater MOSFET, electrical test MOSFET.
 *
 * Operation: heats a probe tip, touches it to a stone, measures the
 * thermal decay curve to extract thermal conductivity, then performs
 * an electrical conductivity test. Combines both to identify the material.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#include "thermal.h"
#include "electrical.h"
#include "display.h"
#include "materials.h"
#include "calibration.h"
#include "ble.h"

// ---- Pin Assignments ----
static const int PIN_I2C_SDA        = 8;
static const int PIN_I2C_SCL        = 9;
static const int PIN_DS18B20        = 4;
static const int PIN_HEATER_MOSFET  = 10;
static const int PIN_ELEC_MOSFET    = 11;
static const int PIN_BUTTON         = 5;
static const int PIN_BUZZER         = 6;
static const int PIN_NEOPIXEL       = 48;

// ---- Constants ----
static const float SERIES_RESISTANCE_OHMS = 10e6f;  // 10M ohm for electrical test
static const float HEATING_TARGET_ABOVE_AMBIENT = 50.0f;  // heat 50C above ambient
static const float CONTACT_DROP_THRESHOLD = 2.0f;  // degrees C drop to detect stone contact
static const int   DECAY_SAMPLE_DURATION_MS = 2000; // sample decay for 2 seconds
static const unsigned long LONG_PRESS_MS = 2000;    // 2s for calibration mode
static const unsigned long DEBOUNCE_MS   = 50;

// ---- State Machine ----
enum DeviceState {
    STATE_IDLE,
    STATE_HEATING,
    STATE_READY,
    STATE_MEASURING_THERMAL,
    STATE_MEASURING_ELECTRICAL,
    STATE_RESULT,
    STATE_CALIBRATING
};

static const char* stateNames[] = {
    "IDLE", "HEATING", "READY", "MEASURING_THERMAL",
    "MEASURING_ELECTRICAL", "RESULT", "CALIBRATING"
};

// ---- Global Objects ----
static Adafruit_ADS1115 ads;
static OneWire oneWire(PIN_DS18B20);
static DallasTemperature ds18b20(&oneWire);
static Adafruit_NeoPixel pixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

static ThermalProbe thermalProbe;
static ElectricalProbe electricalProbe;
static GemDisplay display;
static CalibrationManager calibration;
static GemTesterBLE bleMgr;

static DeviceState currentState = STATE_IDLE;
static int calibrationStep = 0;

// ---- Measurement results (retained for display and BLE) ----
static float measuredTau = 0.0f;
static float measuredThermalK = 0.0f;
static float measuredResistance = 0.0f;
static MaterialResult identifiedMaterial;

// ---- Decay curve buffer (static, no heap allocation) ----
static const int MAX_DECAY_SAMPLES = 2048;
static float decayTimestamps[MAX_DECAY_SAMPLES];
static float decayTemperatures[MAX_DECAY_SAMPLES];

// ---- Button handling ----
static bool lastButtonState = HIGH;
static unsigned long buttonPressTime = 0;
static bool buttonHandled = false;

// ---- Forward declarations ----
void setState(DeviceState newState);
void handleIdle();
void handleHeating();
void handleReady();
void handleMeasuringThermal();
void handleMeasuringElectrical();
void handleResult();
void handleCalibrating();
void setPixelColor(uint8_t r, uint8_t g, uint8_t b);
void beep(int freqHz, int durationMs);
void beepPattern(int count, int freqHz, int durationMs, int pauseMs);

// ---- Button utility: returns 0=none, 1=short press, 2=long press ----
int checkButton() {
    bool reading = digitalRead(PIN_BUTTON);

    if (reading == LOW && lastButtonState == HIGH) {
        // Button just pressed
        buttonPressTime = millis();
        buttonHandled = false;
    }

    if (reading == LOW && !buttonHandled) {
        if (millis() - buttonPressTime >= LONG_PRESS_MS) {
            buttonHandled = true;
            lastButtonState = reading;
            return 2;  // long press
        }
    }

    if (reading == HIGH && lastButtonState == LOW) {
        // Button released
        lastButtonState = reading;
        if (!buttonHandled && (millis() - buttonPressTime >= DEBOUNCE_MS)) {
            return 1;  // short press
        }
        return 0;
    }

    lastButtonState = reading;
    return 0;
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("OpenGemTester v1.0 starting...");

    // I2C bus
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);  // 400 kHz fast mode

    // GPIO setup
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_HEATER_MOSFET, OUTPUT);
    pinMode(PIN_ELEC_MOSFET, OUTPUT);
    digitalWrite(PIN_HEATER_MOSFET, LOW);
    digitalWrite(PIN_ELEC_MOSFET, LOW);

    // NeoPixel
    pixel.begin();
    pixel.setBrightness(30);
    setPixelColor(0, 0, 50);  // dim blue = booting

    // OLED display
    if (!display.init()) {
        Serial.println("ERROR: SSD1306 display not found!");
        setPixelColor(50, 0, 0);
        while (1) { delay(1000); }
    }
    display.showSplash();
    delay(1500);

    // ADS1115 16-bit ADC
    if (!ads.begin(0x48)) {
        Serial.println("ERROR: ADS1115 not found at 0x48!");
        display.showError("ADC not found");
        setPixelColor(50, 0, 0);
        while (1) { delay(1000); }
    }
    // Set to maximum continuous sample rate: 860 SPS
    // (practical effective rate ~475 SPS accounting for I2C overhead)
    ads.setDataRate(RATE_ADS1115_860SPS);
    ads.setGain(GAIN_ONE);  // +/- 4.096V range, ~0.125 mV per bit

    // DS18B20 ambient temperature sensor
    ds18b20.begin();
    if (ds18b20.getDeviceCount() == 0) {
        Serial.println("WARNING: DS18B20 not found, using NTC for ambient estimate");
    } else {
        ds18b20.setResolution(12);
        ds18b20.setWaitForConversion(false);
    }

    // Subsystem initialization
    thermalProbe.init(&ads, PIN_HEATER_MOSFET, &ds18b20);
    electricalProbe.init(&ads, PIN_ELEC_MOSFET, SERIES_RESISTANCE_OHMS);
    calibration.loadCalibration();

    // BLE
    bleMgr.init();
    bleMgr.startAdvertising();

    // Startup beep
    beep(1000, 100);

    // Check calibration
    if (!calibration.isCalibrated()) {
        Serial.println("Device not calibrated. Long-press button for calibration.");
        display.showError("Not calibrated!\nLong-press to cal");
        delay(2000);
    }

    setState(STATE_IDLE);
    Serial.println("Setup complete. Ready.");
}

void loop() {
    int btn = checkButton();

    // Long press always enters calibration (unless already calibrating)
    if (btn == 2 && currentState != STATE_CALIBRATING) {
        calibrationStep = 0;
        setState(STATE_CALIBRATING);
        return;
    }

    switch (currentState) {
        case STATE_IDLE:
            handleIdle();
            break;
        case STATE_HEATING:
            handleHeating();
            break;
        case STATE_READY:
            handleReady();
            break;
        case STATE_MEASURING_THERMAL:
            handleMeasuringThermal();
            break;
        case STATE_MEASURING_ELECTRICAL:
            handleMeasuringElectrical();
            break;
        case STATE_RESULT:
            handleResult();
            break;
        case STATE_CALIBRATING:
            handleCalibrating();
            break;
    }

    delay(10);  // small yield to avoid WDT
}

// ---- State Handlers ----

void handleIdle() {
    int btn = checkButton();
    if (btn == 1) {
        // Short press: start test
        if (!calibration.isCalibrated()) {
            display.showError("Calibrate first!\nLong-press button");
            beepPattern(3, 500, 100, 100);
            delay(1500);
            display.showReady();
            return;
        }
        setState(STATE_HEATING);
    }
}

void handleHeating() {
    // Read ambient temperature
    float ambient = thermalProbe.getAmbientTemp();
    float targetTemp = ambient + HEATING_TARGET_ABOVE_AMBIENT;
    float probeTemp = thermalProbe.getProbeTemp();

    display.showHeating(probeTemp, targetTemp);
    setPixelColor(50, 20, 0);  // orange = heating

    // Start heating
    thermalProbe.setHeater(true);

    unsigned long startTime = millis();
    unsigned long timeout = 30000;  // 30 second timeout

    while (true) {
        probeTemp = thermalProbe.getProbeTemp();
        display.showHeating(probeTemp, targetTemp);

        if (probeTemp >= targetTemp) {
            thermalProbe.setHeater(false);
            setState(STATE_READY);
            return;
        }

        if (millis() - startTime > timeout) {
            thermalProbe.setHeater(false);
            display.showError("Heating timeout!");
            beepPattern(3, 300, 200, 100);
            delay(2000);
            setState(STATE_IDLE);
            return;
        }

        // Check for button press to cancel
        if (digitalRead(PIN_BUTTON) == LOW) {
            delay(50);
            if (digitalRead(PIN_BUTTON) == LOW) {
                thermalProbe.setHeater(false);
                setState(STATE_IDLE);
                return;
            }
        }

        delay(50);
    }
}

void handleReady() {
    display.showTouchPrompt();
    setPixelColor(0, 50, 0);  // green = ready
    beep(2000, 150);

    float initialTemp = thermalProbe.getProbeTemp();
    unsigned long readyStart = millis();
    unsigned long readyTimeout = 30000;  // 30 seconds to touch stone

    while (true) {
        float temp = thermalProbe.getProbeTemp();

        // Detect contact: temperature drops faster than natural cooling
        if ((initialTemp - temp) > CONTACT_DROP_THRESHOLD) {
            setState(STATE_MEASURING_THERMAL);
            return;
        }

        if (millis() - readyStart > readyTimeout) {
            display.showError("Timeout - no contact");
            beepPattern(2, 400, 150, 100);
            delay(2000);
            setState(STATE_IDLE);
            return;
        }

        // Cancel
        if (digitalRead(PIN_BUTTON) == LOW) {
            delay(50);
            if (digitalRead(PIN_BUTTON) == LOW) {
                setState(STATE_IDLE);
                return;
            }
        }

        delay(5);
    }
}

void handleMeasuringThermal() {
    display.showMeasuring("Thermal", 0.0f);
    setPixelColor(0, 0, 50);  // blue = measuring

    float ambient = thermalProbe.getAmbientTemp();

    // High-speed decay sampling
    int sampleCount = thermalProbe.measureDecay(
        decayTimestamps, decayTemperatures, MAX_DECAY_SAMPLES,
        DECAY_SAMPLE_DURATION_MS
    );

    if (sampleCount < 20) {
        display.showError("Too few samples!");
        beepPattern(2, 400, 150, 100);
        delay(2000);
        setState(STATE_IDLE);
        return;
    }

    display.showMeasuring("Fitting curve", 0.5f);

    // Fit exponential decay to extract tau
    measuredTau = thermalProbe.fitExponentialDecay(
        decayTimestamps, decayTemperatures, sampleCount, ambient
    );

    if (measuredTau <= 0.0f || measuredTau > 60000.0f) {
        display.showError("Bad decay fit");
        beepPattern(2, 400, 150, 100);
        delay(2000);
        setState(STATE_IDLE);
        return;
    }

    // Convert tau to thermal conductivity using calibration
    measuredThermalK = calibration.tauToThermalConductivity(measuredTau);

    Serial.printf("Thermal: tau=%.1f ms, k=%.1f W/mK, %d samples\n",
                  measuredTau, measuredThermalK, sampleCount);

    display.showMeasuring("Thermal", 1.0f);
    delay(200);

    setState(STATE_MEASURING_ELECTRICAL);
}

void handleMeasuringElectrical() {
    display.showMeasuring("Electrical", 0.0f);
    setPixelColor(50, 0, 50);  // purple = electrical test

    // Switch to electrical mode (disconnect heater, connect conductivity circuit)
    electricalProbe.enableElectricalMode();
    delay(100);  // settling time

    display.showMeasuring("Electrical", 0.3f);

    // Take averaged resistance measurement (median of 20 readings)
    measuredResistance = electricalProbe.averageResistance(20);

    display.showMeasuring("Electrical", 0.8f);

    // Return to thermal mode
    electricalProbe.disableElectricalMode();

    ConductivityClass cc = electricalProbe.classifyConductivity(measuredResistance);
    const char* ccName = (cc == COND_INSULATOR) ? "Insulator" :
                         (cc == COND_SEMICONDUCTOR) ? "Semiconductor" : "Conductor";

    Serial.printf("Electrical: R=%.0f ohms, class=%s\n", measuredResistance, ccName);

    display.showMeasuring("Electrical", 1.0f);
    delay(200);

    setState(STATE_RESULT);
}

void handleResult() {
    // Identify material from combined measurements
    identifiedMaterial = identifyMaterial(measuredThermalK, measuredResistance);

    display.showResult(
        identifiedMaterial.name,
        identifiedMaterial.confidence,
        measuredThermalK,
        measuredResistance
    );

    // Color-code the result LED by confidence
    if (identifiedMaterial.confidence >= 0.8f) {
        setPixelColor(0, 50, 0);  // green = high confidence
        beep(2000, 300);
    } else if (identifiedMaterial.confidence >= 0.5f) {
        setPixelColor(50, 50, 0);  // yellow = medium confidence
        beepPattern(2, 1500, 150, 100);
    } else {
        setPixelColor(50, 20, 0);  // orange = low confidence
        beepPattern(3, 800, 100, 100);
    }

    // Update BLE
    bleMgr.updateResult(
        identifiedMaterial.name,
        identifiedMaterial.confidence,
        measuredThermalK,
        measuredResistance
    );
    bleMgr.updateStatus("RESULT");

    // Wait for button press to return to idle
    while (true) {
        int btn = checkButton();
        if (btn == 1) {
            setState(STATE_IDLE);
            return;
        }
        if (btn == 2) {
            calibrationStep = 0;
            setState(STATE_CALIBRATING);
            return;
        }
        delay(50);
    }
}

void handleCalibrating() {
    /*
     * Calibration flow:
     * Step 0: Prompt "Heat probe, then hold in air"
     * Step 1: Measure air baseline (very slow decay = low thermal conductivity)
     * Step 2: Prompt "Place probe on glass"
     * Step 3: Measure glass reference
     * Step 4: Prompt "Place on diamond (optional, press to skip)"
     * Step 5: Measure diamond (or skip)
     * Step 6: Save and done
     */

    setPixelColor(50, 50, 50);  // white = calibrating

    // Step 0: Heat for air calibration
    display.showCalibrationStep(1, "Heating probe\nfor air baseline...");
    {
        float ambient = thermalProbe.getAmbientTemp();
        float target = ambient + HEATING_TARGET_ABOVE_AMBIENT;
        thermalProbe.setHeater(true);

        unsigned long t0 = millis();
        while (thermalProbe.getProbeTemp() < target) {
            display.showHeating(thermalProbe.getProbeTemp(), target);
            if (millis() - t0 > 30000) {
                thermalProbe.setHeater(false);
                display.showError("Heating timeout");
                delay(2000);
                setState(STATE_IDLE);
                return;
            }
            delay(50);
        }
        thermalProbe.setHeater(false);
    }

    // Step 1: Measure air decay
    display.showCalibrationStep(1, "Hold probe in air\nMeasuring...");
    beep(2000, 150);
    delay(500);
    {
        float ambient = thermalProbe.getAmbientTemp();
        int count = thermalProbe.measureDecay(
            decayTimestamps, decayTemperatures, MAX_DECAY_SAMPLES, 3000
        );
        float tauAir = thermalProbe.fitExponentialDecay(
            decayTimestamps, decayTemperatures, count, ambient
        );
        if (tauAir <= 0) {
            display.showError("Air cal failed");
            delay(2000);
            setState(STATE_IDLE);
            return;
        }
        calibration.setAirTau(tauAir);
        Serial.printf("Calibration: air tau = %.1f ms\n", tauAir);
    }

    // Step 2: Heat again for glass
    display.showCalibrationStep(2, "Heating probe\nfor glass cal...");
    {
        float ambient = thermalProbe.getAmbientTemp();
        float target = ambient + HEATING_TARGET_ABOVE_AMBIENT;
        thermalProbe.setHeater(true);

        unsigned long t0 = millis();
        while (thermalProbe.getProbeTemp() < target) {
            display.showHeating(thermalProbe.getProbeTemp(), target);
            if (millis() - t0 > 30000) {
                thermalProbe.setHeater(false);
                display.showError("Heating timeout");
                delay(2000);
                setState(STATE_IDLE);
                return;
            }
            delay(50);
        }
        thermalProbe.setHeater(false);
    }

    // Step 3: Measure glass
    display.showCalibrationStep(3, "Touch probe to\nwindow glass now");
    beep(2000, 150);
    {
        // Wait for contact
        float initTemp = thermalProbe.getProbeTemp();
        unsigned long t0 = millis();
        while (initTemp - thermalProbe.getProbeTemp() < CONTACT_DROP_THRESHOLD) {
            if (millis() - t0 > 30000) {
                display.showError("No glass contact");
                delay(2000);
                setState(STATE_IDLE);
                return;
            }
            delay(5);
        }

        float ambient = thermalProbe.getAmbientTemp();
        int count = thermalProbe.measureDecay(
            decayTimestamps, decayTemperatures, MAX_DECAY_SAMPLES, 2000
        );
        float tauGlass = thermalProbe.fitExponentialDecay(
            decayTimestamps, decayTemperatures, count, ambient
        );
        if (tauGlass <= 0) {
            display.showError("Glass cal failed");
            delay(2000);
            setState(STATE_IDLE);
            return;
        }
        calibration.setGlassTau(tauGlass);
        Serial.printf("Calibration: glass tau = %.1f ms\n", tauGlass);
    }

    // Step 4: Optional diamond calibration
    display.showCalibrationStep(4, "Have a diamond?\nPress=yes, hold=skip");
    {
        unsigned long waitStart = millis();
        bool doDiamond = false;
        while (millis() - waitStart < 10000) {
            int btn = checkButton();
            if (btn == 1) { doDiamond = true; break; }
            if (btn == 2) { break; }  // skip
            delay(50);
        }

        if (doDiamond) {
            // Heat for diamond
            display.showCalibrationStep(5, "Heating for\ndiamond cal...");
            float ambient = thermalProbe.getAmbientTemp();
            float target = ambient + HEATING_TARGET_ABOVE_AMBIENT;
            thermalProbe.setHeater(true);
            unsigned long t0 = millis();
            while (thermalProbe.getProbeTemp() < target) {
                display.showHeating(thermalProbe.getProbeTemp(), target);
                if (millis() - t0 > 30000) {
                    thermalProbe.setHeater(false);
                    display.showError("Heating timeout");
                    delay(2000);
                    setState(STATE_IDLE);
                    return;
                }
                delay(50);
            }
            thermalProbe.setHeater(false);

            display.showCalibrationStep(5, "Touch diamond now");
            beep(2000, 150);

            float initTemp = thermalProbe.getProbeTemp();
            t0 = millis();
            while (initTemp - thermalProbe.getProbeTemp() < CONTACT_DROP_THRESHOLD) {
                if (millis() - t0 > 30000) {
                    display.showError("No diamond contact");
                    delay(2000);
                    setState(STATE_IDLE);
                    return;
                }
                delay(5);
            }

            int count = thermalProbe.measureDecay(
                decayTimestamps, decayTemperatures, MAX_DECAY_SAMPLES, 2000
            );
            float tauDiamond = thermalProbe.fitExponentialDecay(
                decayTimestamps, decayTemperatures, count, ambient
            );
            if (tauDiamond > 0) {
                calibration.setDiamondTau(tauDiamond);
                Serial.printf("Calibration: diamond tau = %.1f ms\n", tauDiamond);
            }
        }
    }

    // Step 6: Save calibration
    calibration.buildMapping();
    calibration.saveCalibration();

    display.showCalibrationStep(6, "Calibration done!");
    beepPattern(2, 2000, 200, 100);
    delay(2000);

    setState(STATE_IDLE);
}

// ---- Helpers ----

void setState(DeviceState newState) {
    currentState = newState;
    Serial.printf("State -> %s\n", stateNames[(int)newState]);
    bleMgr.updateStatus(stateNames[(int)newState]);

    switch (newState) {
        case STATE_IDLE:
            setPixelColor(0, 20, 0);  // dim green
            display.showReady();
            break;
        default:
            break;
    }
}

void setPixelColor(uint8_t r, uint8_t g, uint8_t b) {
    pixel.setPixelColor(0, pixel.Color(r, g, b));
    pixel.show();
}

void beep(int freqHz, int durationMs) {
    tone(PIN_BUZZER, freqHz, durationMs);
    delay(durationMs);
    noTone(PIN_BUZZER);
}

void beepPattern(int count, int freqHz, int durationMs, int pauseMs) {
    for (int i = 0; i < count; i++) {
        beep(freqHz, durationMs);
        if (i < count - 1) delay(pauseMs);
    }
}
