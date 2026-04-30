/*
 * OpenGemTester - BLE Interface Implementation
 *
 * Uses the ESP32 BLE Arduino library (bundled with ESP-IDF/Arduino).
 * Creates a GATT server with one service and four characteristics.
 *
 * UUIDs follow a common base with different suffixes for each characteristic.
 * The service UUID is chosen to avoid collision with standard Bluetooth SIG profiles.
 *
 * BLE MTU is typically 512 bytes on ESP32-S3, which limits the raw data
 * characteristic payload. We pack decay samples as 4-byte float pairs
 * (timestamp_ms, temperature_C) for efficiency.
 */

#include "ble.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Custom UUIDs for OpenGemTester service and characteristics
#define OGT_SERVICE_UUID        "4f475400-6765-6d74-6573-746572000001"
#define OGT_RESULT_CHAR_UUID    "4f475400-6765-6d74-6573-746572000002"
#define OGT_RAWDATA_CHAR_UUID   "4f475400-6765-6d74-6573-746572000003"
#define OGT_COMMAND_CHAR_UUID   "4f475400-6765-6d74-6573-746572000004"
#define OGT_STATUS_CHAR_UUID    "4f475400-6765-6d74-6573-746572000005"

// Maximum JSON result string length
static const int MAX_RESULT_JSON = 256;

// Maximum raw data payload (BLE MTU - overhead)
// Each sample pair = 8 bytes (2 x float). 512 / 8 = 64 pairs max.
static const int MAX_BLE_SAMPLE_PAIRS = 60;

// ---- BLE Server Callbacks ----
// These use a global pointer to update the connection state in the GemTesterBLE instance.
static bool* g_connectedFlag = nullptr;

class OGTServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) override {
        if (g_connectedFlag) *g_connectedFlag = true;
        Serial.println("BLE: client connected");
    }

    void onDisconnect(BLEServer* server) override {
        if (g_connectedFlag) *g_connectedFlag = false;
        Serial.println("BLE: client disconnected");
        // Restart advertising so another client can connect
        BLEDevice::startAdvertising();
    }
};

// ---- Command Characteristic Callbacks ----
static int* g_pendingCommand = nullptr;

class OGTCommandCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) override {
        String value = characteristic->getValue();
        if (value.length() > 0 && g_pendingCommand) {
            uint8_t cmd = value[0];
            switch (cmd) {
                case 0x01:
                    *g_pendingCommand = 1;  // start test
                    Serial.println("BLE: received command - start test");
                    break;
                case 0x02:
                    *g_pendingCommand = 2;  // start calibration
                    Serial.println("BLE: received command - start calibration");
                    break;
                default:
                    Serial.printf("BLE: unknown command 0x%02X\n", cmd);
                    break;
            }
        }
    }
};

// ---- Implementation ----

GemTesterBLE::GemTesterBLE()
    : _server(nullptr)
    , _service(nullptr)
    , _resultChar(nullptr)
    , _rawDataChar(nullptr)
    , _commandChar(nullptr)
    , _statusChar(nullptr)
    , _deviceConnected(false)
    , _pendingCommand(0)
{}

void GemTesterBLE::init() {
    // Set up global callback pointers
    g_connectedFlag = &_deviceConnected;
    g_pendingCommand = &_pendingCommand;

    BLEDevice::init("OpenGemTester");
    BLEDevice::setMTU(512);

    _server = BLEDevice::createServer();
    _server->setCallbacks(new OGTServerCallbacks());

    _service = _server->createService(OGT_SERVICE_UUID);

    // Material result characteristic (read + notify)
    _resultChar = _service->createCharacteristic(
        OGT_RESULT_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _resultChar->addDescriptor(new BLE2902());
    _resultChar->setValue("{}");

    // Raw thermal data characteristic (read + notify)
    _rawDataChar = _service->createCharacteristic(
        OGT_RAWDATA_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _rawDataChar->addDescriptor(new BLE2902());

    // Command characteristic (write only)
    _commandChar = _service->createCharacteristic(
        OGT_COMMAND_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _commandChar->setCallbacks(new OGTCommandCallbacks());

    // Status characteristic (read + notify)
    _statusChar = _service->createCharacteristic(
        OGT_STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _statusChar->addDescriptor(new BLE2902());
    _statusChar->setValue("INIT");

    _service->start();

    Serial.println("BLE: service started");
}

void GemTesterBLE::startAdvertising() {
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(OGT_SERVICE_UUID);
    advertising->setScanResponse(true);
    // Helps with iPhone connectivity
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    Serial.println("BLE: advertising started");
}

void GemTesterBLE::updateResult(const char* materialName, float confidence,
                                 float thermalK, float resistance) {
    if (!_resultChar) return;

    // Build JSON result string
    char json[MAX_RESULT_JSON];
    snprintf(json, sizeof(json),
        "{\"material\":\"%s\",\"confidence\":%.2f,\"thermal_k\":%.1f,\"resistance\":%.0f}",
        materialName, confidence, thermalK, resistance
    );

    _resultChar->setValue(json);
    if (_deviceConnected) {
        _resultChar->notify();
    }

    Serial.printf("BLE: result updated - %s\n", json);
}

void GemTesterBLE::updateRawData(float* timestamps, float* temperatures,
                                  int count, int maxPairs) {
    if (!_rawDataChar || count <= 0) return;

    // Limit to BLE payload capacity
    int pairs = count;
    if (pairs > maxPairs) pairs = maxPairs;
    if (pairs > MAX_BLE_SAMPLE_PAIRS) pairs = MAX_BLE_SAMPLE_PAIRS;

    /*
     * Pack as binary: [float time_ms, float temp_C] repeated.
     * If we have more samples than fit, downsample evenly.
     */
    int step = 1;
    if (count > pairs) {
        step = count / pairs;
    }

    // Static buffer: 60 pairs * 8 bytes = 480 bytes (within 512 MTU)
    static uint8_t buf[MAX_BLE_SAMPLE_PAIRS * 8];
    int offset = 0;
    int written = 0;

    for (int i = 0; i < count && written < pairs; i += step) {
        memcpy(buf + offset, &timestamps[i], sizeof(float));
        offset += sizeof(float);
        memcpy(buf + offset, &temperatures[i], sizeof(float));
        offset += sizeof(float);
        written++;
    }

    _rawDataChar->setValue(buf, offset);
    if (_deviceConnected) {
        _rawDataChar->notify();
    }

    Serial.printf("BLE: raw data sent, %d pairs (%d bytes)\n", written, offset);
}

void GemTesterBLE::updateStatus(const char* status) {
    if (!_statusChar) return;

    _statusChar->setValue(status);
    if (_deviceConnected) {
        _statusChar->notify();
    }
}

bool GemTesterBLE::isConnected() {
    return _deviceConnected;
}

int GemTesterBLE::getPendingCommand() {
    int cmd = _pendingCommand;
    _pendingCommand = 0;  // consume the command
    return cmd;
}
