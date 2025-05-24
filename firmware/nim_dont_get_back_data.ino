#define CONFIG_NIMBLE_DEBUG_LEVEL 4
#include <NimBLEDevice.h>


#define ledPin 2

NimBLECharacteristic* pCharacteristic;
NimBLEServer* pServer;

class UniversalCallback : public NimBLECharacteristicCallbacks {
  String type;
public:
  UniversalCallback(const String& t) : type(t) {}

  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    std::string value = pCharacteristic->getValue();
    if (type == "led") {
      if (value == "ON") digitalWrite(ledPin, HIGH);
      else if (value == "OFF") digitalWrite(ledPin, LOW);
      Serial.println("LED: " + String(value.c_str()));
    }
    else if (type == "ssid") {
      Serial.println("SSID set to: " + String(value.c_str()));
    }
    else if (type == "pw") {
      Serial.println("Password set to: " + String(value.c_str()));
    }

    NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
    pServer->getAdvertising()->start();
    Serial.println("ReAdvertising");
  }
};

String getID() {
  uint64_t chipid = ESP.getEfuseMac();  // 64-bit unique ID
  char chipString[17];
  sprintf(chipString, "%012llX", chipid);
  String deviceID = String(chipString);
  return deviceID;
}

String device_id = getID();
String uuid = "00000000-0000-0000-0000-" + device_id;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  NimBLEDevice::init("ESP32_NimBLE_1");
  NimBLEServer* pServer = NimBLEDevice::createServer();

  NimBLEService* pService = pServer->createService(uuid.c_str());

  NimBLECharacteristic* ledChar = pService->createCharacteristic(
    "12345678-5678-90ab-cdef-1234567890ab",
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ
  );

  NimBLECharacteristic* ssidChar = pService->createCharacteristic(
    "22345678-5678-90ab-cdef-1234567890ab",
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ
  );

  NimBLECharacteristic* pwChar = pService->createCharacteristic(
    "32345678-5678-90ab-cdef-1234567890ab",
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ
  );

  ledChar->setValue(uuid.c_str());
  ledChar->setCallbacks(new UniversalCallback("led"));
  ssidChar->setCallbacks(new UniversalCallback("ssid"));
  pwChar->setCallbacks(new UniversalCallback("pw"));

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData scanData;
  scanData.setName("ESP32");

  pAdvertising->setScanResponseData(scanData);
  pAdvertising->start();

  Serial.println("ESP32 ready for BLE writes");
  Serial.println(device_id);
}

void loop() {
}
