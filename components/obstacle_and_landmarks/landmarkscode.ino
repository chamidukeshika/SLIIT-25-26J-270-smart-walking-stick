#include "esp_camera.h"
#include "esp_system.h"
#include "esp_log.h"

#include <TinyML-Vision-System_inferencing.h>

#include "config.h"
#include "camera_handler.h"
#include "audio_handler.h"
#include "ultrasonic_handler.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

String lastDetectionLabel = "";
unsigned long lastDetectionTime = 0;

static const char *BLE_DEVICE_NAME  = "ESP32-VISION";
static const char *BLE_SERVICE_UUID = "12345678-1234-1234-1234-1234567890ab";
static const char *BLE_CHAR_UUID    = "abcd1234-5678-1234-5678-abcdefabcdef";

static BLECharacteristic *g_notifyChar = nullptr;
static bool g_bleConnected = false;

static const uint32_t ANNOUNCE_COOLDOWN_MS = VISION_AUDIO_COOLDOWN_MS;
static uint32_t lastAnnounceCrosswalk = 0;
static uint32_t lastAnnounceTraffic   = 0;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    g_bleConnected = true;
  }
  void onDisconnect(BLEServer *pServer) override {
    g_bleConnected = false;
    BLEDevice::startAdvertising();
  }
};

static void ble_init()
{
  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(BLE_SERVICE_UUID);
  g_notifyChar = service->createCharacteristic(
    BLE_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  g_notifyChar->addDescriptor(new BLE2902());
  g_notifyChar->setValue("READY");

  service->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(BLE_SERVICE_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started: ESP32-VISION");
}

static float smoothed_scores[EI_CLASSIFIER_LABEL_COUNT] = {0};
static String stableLabel = "";
static uint8_t stableCount = 0;

static const float THRESH_CROSSWALK     = 0.60f;
static const float THRESH_TRAFFIC_LIGHT = 0.45f;
static const float SMOOTH_ALPHA         = 0.35f;
static const uint8_t STABLE_FRAMES_REQUIRED = 3;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("TinyML Vision System v2.0 Starting");

  ultrasonic_init();

  Serial.println("Initializing DFPlayer Mini...");
  if (!audio_init()) {
    Serial.println("[WARN] Audio unavailable");
  }

  Serial.println("Initializing camera...");
  if (!initCamera()) {
    Serial.println("Camera initialization FAILED!");
    while (1) { delay(1000); }
  }
  Serial.println("Camera initialized!");

  ble_init();

  Serial.print("Classes: ");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print(ei_classifier_inferencing_categories[ix]);
    if (ix < EI_CLASSIFIER_LABEL_COUNT - 1) Serial.print(", ");
  }
  Serial.println();

  delay(500);
  audio_play_track(TRACK_CROSSWALK);

  Serial.println("System ready!");
  delay(1000);
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
  size_t pixel_ix  = offset * 3;
  size_t pixels_left = length;
  size_t out_ptr_ix  = 0;

  while (pixels_left != 0) {
    out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) +
                          (snapshot_buf[pixel_ix + 1] << 8) +
                          snapshot_buf[pixel_ix];
    out_ptr_ix++;
    pixel_ix += 3;
    pixels_left--;
  }
  return 0;
}

void loop() {

  ultrasonic_update();

  camera_fb_t *fb = captureImage();
  if (fb == NULL) {
    Serial.println("Failed to capture image");
    delay(1000);
    return;
  }

  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &ei_camera_get_data;

  {
    const size_t pixel_count = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    uint8_t minY = 255, maxY = 0;
    uint32_t sumY = 0;
    size_t step = pixel_count / 128;
    if (step < 1) step = 1;
    size_t n = 0;
    for (size_t i = 0; i < pixel_count; i += step) {
      size_t ix = i * 3;
      uint8_t r = snapshot_buf[ix + 0];
      uint8_t g = snapshot_buf[ix + 1];
      uint8_t b = snapshot_buf[ix + 2];
      uint8_t y = (uint8_t)((uint16_t)r * 30 / 100 + (uint16_t)g * 59 / 100 + (uint16_t)b * 11 / 100);
      if (y < minY) minY = y;
      if (y > maxY) maxY = y;
      sumY += y;
      n++;
    }
    uint8_t meanY = (n > 0) ? (uint8_t)(sumY / n) : 0;
    Serial.print("Y stats: min="); Serial.print(minY);
    Serial.print(" mean=");        Serial.print(meanY);
    Serial.print(" max=");         Serial.println(maxY);

    uint32_t h = 2166136261u;
    const size_t bytes = pixel_count * 3;
    size_t stepB = bytes / 64;
    if (stepB < 1) stepB = 1;
    for (size_t j = 0; j < bytes; j += stepB) {
      h ^= snapshot_buf[j];
      h *= 16777619u;
    }
    h ^= snapshot_buf[bytes - 1];
    h *= 16777619u;
    Serial.print("snapshot hash: 0x");
    Serial.println(h, HEX);
  }

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  (void)fb;

  if (res != EI_IMPULSE_OK) {
    Serial.printf("ERR: Failed to run classifier (%d)\n", res);
    delay(1000);
    return;
  }

  Serial.println("--- All Class Scores ---");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    float v = result.classification[ix].value;
    Serial.print("  ");
    Serial.print(ei_classifier_inferencing_categories[ix]);
    Serial.print(": ");
    Serial.print((int)(v * 100));
    Serial.println("%");
    smoothed_scores[ix] = (smoothed_scores[ix] * (1.0f - SMOOTH_ALPHA)) + (v * SMOOTH_ALPHA);
  }
  Serial.println("------------------------");

  float max_confidence    = 0.0f;
  float second_confidence = 0.0f;
  int   max_index         = -1;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    float v = smoothed_scores[ix];
    if (v > max_confidence) {
      second_confidence = max_confidence;
      max_confidence    = v;
      max_index         = ix;
    } else if (v > second_confidence) {
      second_confidence = v;
    }
  }

  float confidence_gap = max_confidence - second_confidence;

  String topLabel    = (max_index >= 0) ? String(ei_classifier_inferencing_categories[max_index]) : String("");
  bool   isBackground = (topLabel == "background");

  if (!isBackground && topLabel == stableLabel) {
    if (stableCount < 255) stableCount++;
  } else {
    stableLabel = topLabel;
    stableCount = (!isBackground && topLabel.length() > 0) ? 1 : 0;
  }

  float threshold = DETECTION_THRESHOLD;
  if      (topLabel == "crosswalk")     threshold = THRESH_CROSSWALK;
  else if (topLabel == "traffic_light") threshold = THRESH_TRAFFIC_LIGHT;

  if (!isBackground &&
      max_confidence >= threshold &&
      stableCount    >= STABLE_FRAMES_REQUIRED &&
      confidence_gap >= 0.15f &&
      max_index      >= 0) {

    String label      = String(ei_classifier_inferencing_categories[max_index]);
    int    confidence = (int)(max_confidence * 100);

    if (max_confidence >= ANNOUNCE_MIN_CONF_VISION) {
      uint32_t nowMs = millis();

      if (label == "crosswalk") {
        uint32_t elapsed = nowMs - lastAnnounceCrosswalk;
        if (elapsed >= ANNOUNCE_COOLDOWN_MS) {
          if (g_notifyChar && g_bleConnected) {
            g_notifyChar->setValue("CROSSWALK");
            g_notifyChar->notify();
            Serial.println("BLE notify: CROSSWALK");
          }
          audio_play_crosswalk();
          lastAnnounceCrosswalk = nowMs;
        } else {
          Serial.print("[AUDIO] Crosswalk cooldown: ");
          Serial.print((ANNOUNCE_COOLDOWN_MS - elapsed) / 1000);
          Serial.println("s remaining");
        }
      }
      else if (label == "traffic_light") {
        uint32_t elapsed = nowMs - lastAnnounceTraffic;
        if (elapsed >= ANNOUNCE_COOLDOWN_MS) {
          if (g_notifyChar && g_bleConnected) {
            g_notifyChar->setValue("TRAFFIC_LIGHT");
            g_notifyChar->notify();
            Serial.println("BLE notify: TRAFFIC_LIGHT");
          }
          audio_play_traffic();
          lastAnnounceTraffic = nowMs;
        } else {
          Serial.print("[AUDIO] Traffic cooldown: ");
          Serial.print((ANNOUNCE_COOLDOWN_MS - elapsed) / 1000);
          Serial.println("s remaining");
        }
      }
    } else {
      Serial.print("[AUDIO] Confidence too low: ");
      Serial.print((int)(max_confidence * 100));
      Serial.println("%");
    }

    unsigned long now = millis();
    if (label != lastDetectionLabel || (now - lastDetectionTime) >= DETECTION_COOLDOWN) {
      Serial.print("[DETECTED] ");
      Serial.print(label);
      Serial.print(" (");
      Serial.print(confidence);
      Serial.println("%)");
      lastDetectionLabel = label;
      lastDetectionTime  = now;
    }

  } else {
    if (millis() - lastDetectionTime > 5000) {
      lastDetectionLabel = "";
    }
  }

  delay(INFERENCE_DELAY);
}