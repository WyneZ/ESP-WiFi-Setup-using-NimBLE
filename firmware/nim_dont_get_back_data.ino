#include <NimBLEDevice.h>
#include <Preferences.h>
#include <stdbool.h>
#include <WiFi.h>


#define ledPin 2

Preferences prefs;
NimBLEServer* pServer;
NimBLEService* pService;
NimBLECharacteristic* pCharacteristic;
NimBLECharacteristic* wifiStatusChar; 
String ssid;
String password;
String wifiStatus = "false";
bool gotSSID = false;
bool gotPassword = false;


bool checkWiFi() {
  Serial.println("Stored SSID: " + ssid);
  Serial.println("Stored Password: " + password);

  if (ssid != "none" && password != "none" ) {
    Serial.println("WiFi is connecting");
    if (setup_wifi() == true) {
      wifiStatus = "true";
      prefs.putString("status", wifiStatus);
      return true;
    }
    else {
      Serial.println("\nWiFi can't connect.");
      wifiStatus = "false";
      prefs.putString("status", wifiStatus);
      return false;
    }
  }
  else {
    Serial.println("WiFi need to setup!!!");
    return false;
  }
}

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
      ssid = String(value.c_str());
      prefs.putString("ssid", ssid);
      gotSSID = true;
    }
    else if (type == "pw") {
      Serial.println("Password set to: " + String(value.c_str()));
      password = String(value.c_str());
      prefs.putString("password", password);
      gotPassword = true;
    }

    if (gotSSID && gotPassword) {
      gotSSID = false;
      gotPassword = false;
      if (checkWiFi() == true) {
        Serial.println("70-Connected!");
        wifiStatusChar->setValue("true");
        wifiStatusChar->notify();
      }
      else {
        Serial.println("70-Failed to connect WiFi!");
        wifiStatusChar->setValue("false");
        digitalWrite(ledPin, HIGH);
        wifiStatusChar->notify();
      }
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

bool setup_wifi() {
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");

  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);

  if (WiFi.getMode() != WIFI_OFF) {
    WiFi.disconnect(true, true);   // Disconnect and erase old configs
    delay(100);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  for (int i=0; i<10; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    else {
      Serial.println("\nWiFi connected");
      Serial.println(WiFi.localIP());
      return true;
    }
  }
  
  Serial.println("\nWiFi connection failed.");
  return false;
}

String device_id = getID();
String uuid = "00000000-0000-0000-0000-" + device_id;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  prefs.begin("wifi_config", false);

  NimBLEDevice::init("ESP32_NimBLE_1");
  pServer = NimBLEDevice::createServer();

  pService = pServer->createService(uuid.c_str());

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

  wifiStatusChar = pService->createCharacteristic(
    "42345678-5678-90ab-cdef-1234567890ab",
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
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
  
  checkWiFi();
}

void loop() {
}
