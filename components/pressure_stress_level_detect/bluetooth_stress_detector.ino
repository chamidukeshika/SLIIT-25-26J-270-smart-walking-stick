#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-5678-1234-abcdefabcdef"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
String lastPrediction = "WAITING";

bool parseData(String data, float features[7]) {
  int index = 0;
  int start = 0;
  
  for (int i = 0; i < 7; i++) {
    int commaIndex = data.indexOf(',', start);
    
    if (i == 6) {
      features[i] = data.substring(start).toFloat();
    } else if (commaIndex == -1) {
      Serial.println("ERROR: Not enough values in data string");
      return false;
    } else {
      features[i] = data.substring(start, commaIndex).toFloat();
      start = commaIndex + 1;
    }
  }
  
  return true;
}

String runPrediction(float features[7]) {
  float ax = features[0];
  float ay = features[1];
  float az = features[2];
  float gx = features[3];
  float gy = features[4];
  float gz = features[5];
  float grip = features[6];
  
  int stress_score = 0;
  
  if (abs(ax) > 1.5) stress_score++;
  if (abs(ay) > 2.0) stress_score++;
  if (az < 8.0) stress_score++;
  if (abs(gx) > 1.0) stress_score++;
  if (abs(gy) > 1.5) stress_score++;
  if (abs(gz) > 1.0) stress_score++;
  if (grip > 500) stress_score++;
  
  if (stress_score >= 4) {
    return "STRESS";
  } else {
    return "CALM";
  }
}

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
    BLEDevice::startAdvertising();
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    
    if (value.length() > 0) {
      Serial.println("========================================");
      Serial.print("Received data: ");
      Serial.println(value);
      
      // Parse the incoming data
      float features[7];
      if (parseData(value, features)) {
        // Print parsed values
        Serial.println("Parsed values:");
        Serial.print("  ax: "); Serial.println(features[0], 3);
        Serial.print("  ay: "); Serial.println(features[1], 3);
        Serial.print("  az: "); Serial.println(features[2], 3);
        Serial.print("  gx: "); Serial.println(features[3], 3);
        Serial.print("  gy: "); Serial.println(features[4], 3);
        Serial.print("  gz: "); Serial.println(features[5], 3);
        Serial.print("  grip: "); Serial.println(features[6], 1);
        
        // Run prediction
        String prediction = runPrediction(features);
        lastPrediction = prediction;
        
        Serial.print("PREDICTION: ");
        Serial.println(prediction);
        Serial.println("========================================");
        
        // Send prediction back to frontend
        pCharacteristic->setValue(prediction.c_str());
        pCharacteristic->notify();
      } else {
        Serial.println("ERROR: Failed to parse data");
        pCharacteristic->setValue("ERROR");
        pCharacteristic->notify();
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 Stress Detection System...");
  
  BLEDevice::init("ESP32-Stress-Detector");
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start();
  
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Server started. Waiting for connections...");
  Serial.println("Device name: ESP32-Stress-Detector");
}

void loop() {
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 5000) {
    Serial.print("Status: ");
    if (deviceConnected) {
      Serial.print("Connected | Last prediction: ");
      Serial.println(lastPrediction);
    } else {
      Serial.println("Waiting for connection...");
    }
    lastStatusPrint = millis();
  }
  
  delay(100);
}
