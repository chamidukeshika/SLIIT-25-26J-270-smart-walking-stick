
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Blind_Stick_Stress_Detection_inferencing.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-5678-1234-abcdefabcdef"

BLECharacteristic *pCharacteristic;
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
String receivedData = "";
bool dataComplete = false;

void processData();

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() == 0) return;

    if (value.indexOf("###END###") >= 0) {
      dataComplete = true;
      Serial.println("\n========================================");
      Serial.println("✅ Data reception complete!");
      Serial.print("Total data length: ");
      Serial.println(receivedData.length());
      processData();
      receivedData = "";
      dataComplete = false;
      return;
    }

    receivedData += value;
    Serial.print("📦 Received chunk (");
    Serial.print(value.length());
    Serial.print(" bytes), total: ");
    Serial.println(receivedData.length());
  }
};

void processData() {
  Serial.println("🔄 Processing time-series data...");
  int featureIndex = 0;
  int startPos = 0;
  for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
    int commaPos = receivedData.indexOf(',', startPos);
    if (commaPos == -1) {
      if (startPos < receivedData.length()) {
        features[i] = receivedData.substring(startPos).toFloat();
        featureIndex++;
      }
      break;
    } else {
      features[i] = receivedData.substring(startPos, commaPos).toFloat();
      startPos = commaPos + 1;
      featureIndex++;
    }
  }
  Serial.print("✅ Parsed ");
  Serial.print(featureIndex);
  Serial.print(" features (expected ");
  Serial.print(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.println(")");
  if (featureIndex != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
    Serial.println("❌ ERROR: Feature count mismatch!");
    Serial.print("Expected: ");
    Serial.print(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    Serial.print(" (102 samples × 7 features)");
    Serial.print(" Received: ");
    Serial.println(featureIndex);
    pCharacteristic->setValue("ERROR");
    Serial.println("========================================\n");
    return;
  }
  Serial.println("\n📊 First sample:");
  Serial.print("  ax="); Serial.print(features[0], 3);
  Serial.print(" ay="); Serial.print(features[1], 3);
  Serial.print(" az="); Serial.print(features[2], 3);
  Serial.print(" gx="); Serial.print(features[3], 3);
  Serial.print(" gy="); Serial.print(features[4], 3);
  Serial.print(" gz="); Serial.print(features[5], 3);
  Serial.print(" grip="); Serial.println(features[6], 0);
  Serial.println("📊 Last sample:");
  int lastIdx = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 7;
  Serial.print("  ax="); Serial.print(features[lastIdx + 0], 3);
  Serial.print(" ay="); Serial.print(features[lastIdx + 1], 3);
  Serial.print(" az="); Serial.print(features[lastIdx + 2], 3);
  Serial.print(" gx="); Serial.print(features[lastIdx + 3], 3);
  Serial.print(" gy="); Serial.print(features[lastIdx + 4], 3);
  Serial.print(" gz="); Serial.print(features[lastIdx + 5], 3);
  Serial.print(" grip="); Serial.println(features[lastIdx + 6], 0);
  Serial.println("\n🧠 Running Edge Impulse classifier...");
  signal_t signal;
  signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
  signal.get_data = [](size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
  };
  ei_impulse_result_t result;
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    Serial.print("❌ ERROR: Classifier failed (code ");
    Serial.print(res);
    Serial.println(")");
    pCharacteristic->setValue("ERROR");
    Serial.println("========================================\n");
    return;
  }
  float calm   = result.classification[0].value;
  float stress = result.classification[1].value;
  Serial.println("\n📈 Classification results:");
  Serial.print("  CALM:   ");
  Serial.print(calm * 100, 1);
  Serial.println("%");
  Serial.print("  STRESS: ");
  Serial.print(stress * 100, 1);
  Serial.println("%");
  const char* output = (stress > calm) ? "STRESS" : "CALM";
  pCharacteristic->setValue(output);
  Serial.print("\n🎯 PREDICTION: ");
  Serial.println(output);
  Serial.println("========================================\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n========================================");
  Serial.println("🚀 ESP32 Stress Detector - Time Series");
  Serial.println("========================================");
  Serial.print("Expected input: 102 samples × 7 features = ");
  Serial.print(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.println(" values");
  Serial.println("========================================\n");
  BLEDevice::init("ESP32-Stress-Detector");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(SERVICE_UUID);
  pCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  service->start();
  BLEDevice::getAdvertising()->start();
  Serial.println("✅ BLE Server ready!");
  Serial.println("📱 Device name: ESP32-Stress-Detector");
  Serial.println("⏳ Waiting for connection...\n");
}

void loop() {
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 10000) {
    if (receivedData.length() > 0 && !dataComplete) {
      Serial.print("⏳ Receiving data... (");
      Serial.print(receivedData.length());
      Serial.println(" bytes so far)");
    }
    lastStatusPrint = millis();
  }
  delay(100);
}
