/*
 * OpenGemTester - BLE Interface Header
 *
 * Exposes measurement results and device status over Bluetooth Low Energy.
 * A companion app (phone/tablet) can connect to receive real-time results,
 * view raw decay curves, and remotely trigger tests or calibration.
 *
 * Service: custom UUID for the OpenGemTester
 * Characteristics:
 *   - Material Result (read/notify): JSON with material name, confidence, k, R
 *   - Raw Thermal Data (read/notify): decay curve samples
 *   - Command (write): trigger test or calibration
 *   - Status (read/notify): current device state string
 */

#ifndef BLE_H
#define BLE_H

#include <Arduino.h>

// Forward declarations for ESP32 BLE types
class BLEServer;
class BLECharacteristic;
class BLEService;

class GemTesterBLE {
public:
    GemTesterBLE();

    /* Initialize BLE stack, create service and characteristics. */
    void init();

    /* Start advertising so clients can discover us. */
    void startAdvertising();

    /* Update the material result characteristic and notify connected clients. */
    void updateResult(const char* materialName, float confidence,
                      float thermalK, float resistance);

    /*
     * Update the raw thermal data characteristic with decay curve samples.
     * Data is sent as a compact binary array of (time_ms, temp_C) pairs.
     * Only the first maxPairs pairs are sent (BLE MTU limits apply).
     */
    void updateRawData(float* timestamps, float* temperatures,
                       int count, int maxPairs);

    /* Update the status characteristic with the current state name. */
    void updateStatus(const char* status);

    /* Check if a client is connected. */
    bool isConnected();

    /*
     * Check for and return any pending command from a BLE client.
     * Returns 0 if no command, 1 for "start test", 2 for "start calibration".
     */
    int getPendingCommand();

private:
    BLEServer* _server;
    BLEService* _service;
    BLECharacteristic* _resultChar;
    BLECharacteristic* _rawDataChar;
    BLECharacteristic* _commandChar;
    BLECharacteristic* _statusChar;

    bool _deviceConnected;
    int _pendingCommand;
};

#endif // BLE_H
