/*
 * TinyML Vision System - Final Code
 * 
 * Complete system for detecting crosswalks and traffic lights
 * using Edge Impulse trained model on ESP32-CAM
 * 
 * Hardware:
 * - ESP32-CAM (AI-Thinker with OV2640 camera)
 * - 0.96" OLED Display (128x64, I2C)
 * 
 * Features:
 * - On-device AI inference
 * - Real-time object detection
 * - OLED display output
 * - Serial Monitor output
 * - Completely offline operation
 * 
 * Classes detected:
 * - crosswalk
 * - traffic_light
 */

// ============================================
// INCLUDES
// ============================================

// Camera and ESP32
#include "esp_camera.h"
#include "esp_system.h"
#include "esp_log.h"

// Edge Impulse library (generated from your trained model)
// Make sure you installed the library from Step 4!
// The filename will be different - it will be your project name
// Example: #include <your-project-name_inferencing.h>
// IMPORTANT: After installing library, uncomment the line below and change to your library name!
#include <TinyML-Vision-System_inferencing.h>

// Configuration and handlers
#include "config.h"
#include "camera_handler.h"

// ============================================
// GLOBAL OBJECTS
// ============================================

// EI example style: we keep a global RGB888 snapshot buffer in camera_handler.h

// Last detection bookkeeping to avoid repeated logs
String lastDetectionLabel = "";
unsigned long lastDetectionTime = 0;

// Simple smoothing & stability (helps reduce flicker)
static float smoothed_scores[EI_CLASSIFIER_LABEL_COUNT] = {0};
static String stableLabel = "";
static uint8_t stableCount = 0;

// Per-label thresholds (tune these)
static const float THRESH_CROSSWALK = 0.60f;
static const float THRESH_TRAFFIC_LIGHT = 0.45f;
static const float SMOOTH_ALPHA = 0.35f; // 0..1 (higher reacts faster)
static const uint8_t STABLE_FRAMES_REQUIRED = 3;

// ============================================
// SETUP
// ============================================

void setup() {
  // Start Serial communication
  Serial.begin(115200);
  delay(2000);  // Wait for Serial
  
  // Print startup message
  Serial.println();
  Serial.println("========================================");
  Serial.println("   TinyML Vision System Starting...");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Hardware:");
  Serial.println("  - ESP32-CAM (AI-Thinker)");
  Serial.println();
  Serial.println("Detection Classes:");
  Serial.println("  - Crosswalk");
  Serial.println("  - Traffic Light");
  Serial.println();
  Serial.println("========================================");
  Serial.println();
  
  // Initialize camera
  Serial.println("Initializing camera...");
  if (!initCamera()) {
    Serial.println("❌ Camera initialization FAILED!");
    Serial.println("System halted. Check camera connection.");
    // Can't show error on display yet (display not initialized)
    while (1) { delay(1000); }  // Stop here
  }
  Serial.println("✓ Camera initialized successfully!");
  Serial.println();
  
  // OLED removed: ESP32-CAM + Serial only
  
  // Check Edge Impulse model
  Serial.println("Loading AI model...");
  
  Serial.print("Model: ");
  Serial.println(EI_CLASSIFIER_PROJECT_NAME);
  Serial.print("Model version: ");
  Serial.println(EI_CLASSIFIER_PROJECT_DEPLOY_VERSION);
  Serial.print("Input size: ");
  Serial.print(EI_CLASSIFIER_INPUT_WIDTH);
  Serial.print("x");
  Serial.println(EI_CLASSIFIER_INPUT_HEIGHT);
  Serial.print("Label count: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);
  Serial.print("Classes: ");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print(ei_classifier_inferencing_categories[ix]);
    if (ix < EI_CLASSIFIER_LABEL_COUNT - 1) Serial.print(", ");
  }
  Serial.println();
  
  Serial.println("✓ Model loaded successfully!");
  Serial.println();
  Serial.println("========================================");
  Serial.println("System ready! Starting detection...");
  Serial.println("========================================");
  Serial.println();
  
  delay(1000);
}

// ============================================
// MAIN LOOP
// ============================================

// Edge Impulse camera callback (matches EI esp32_camera example)
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
  static uint32_t call_count = 0;
  call_count++;

  // offset/length are in pixels; snapshot_buf is RGB888
  size_t pixel_ix = offset * 3;
  size_t pixels_left = length;
  size_t out_ptr_ix = 0;

  while (pixels_left != 0) {
    // Swap BGR to RGB here due to esp32-camera known issue
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
  // Capture image from camera
  camera_fb_t *fb = captureImage();
  
  if (fb == NULL) {
    Serial.println("❌ Failed to capture image");
    delay(1000);
    return;
  }
  
  // Edge Impulse signal served by ei_camera_get_data().
  // NOTE: In the official ESP32 camera example, total_length is pixels (W*H).
  ei::signal_t signal;
  signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
  signal.get_data = &ei_camera_get_data;

  // DEBUG: compute simple luminance stats over a sample of pixels to prove signal changes
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
    Serial.print(" mean="); Serial.print(meanY);
    Serial.print(" max="); Serial.println(maxY);

    // Also print a tiny checksum over snapshot_buf (sampled)
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
  
  // Run classifier
  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  // fb is a sentinel now; snapshot_buf is reused
  (void)fb;
  
  // Check if inference was successful
  if (res != EI_IMPULSE_OK) {
    Serial.printf("ERR: Failed to run classifier (%d)\n", res);
    delay(1000);
    return;
  }
  
  // DEBUG: Print ALL class scores to see what model is thinking
  Serial.println("--- All Class Scores ---");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print("  ");
    Serial.print(ei_classifier_inferencing_categories[ix]);
    Serial.print(": ");
    Serial.print((int)(result.classification[ix].value * 100));
    Serial.println("%");

    // Update smoothing for each label
    float v = result.classification[ix].value;
    smoothed_scores[ix] = (smoothed_scores[ix] * (1.0f - SMOOTH_ALPHA)) + (v * SMOOTH_ALPHA);
  }
  Serial.println("------------------------");

  // Print smoothed scores (more stable)
  Serial.println("--- Smoothed Scores ---");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    Serial.print("  ");
    Serial.print(ei_classifier_inferencing_categories[ix]);
    Serial.print(": ");
    Serial.print((int)(smoothed_scores[ix] * 100));
    Serial.println("%");
  }
  Serial.println("-----------------------");
  
  // Find highest confidence prediction using SMOOTHED scores
  float max_confidence = 0.0;
  float second_confidence = 0.0;
  int max_index = -1;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    float v = smoothed_scores[ix];
    if (v > max_confidence) {
      second_confidence = max_confidence;
      max_confidence = v;
      max_index = ix;
    } else if (v > second_confidence) {
      second_confidence = v;
    }
  }

  float confidence_gap = max_confidence - second_confidence;
  
  // Only show detection if:
  // 1. Confidence is above threshold (85% - very sure!)
  // 2. AND there's a clear winner (at least 40% difference)
  // This helps reduce false positives
  String topLabel = (max_index >= 0) ? String(ei_classifier_inferencing_categories[max_index]) : String("");
  bool isBackground = (topLabel == "background");

  // Update stability counter
  if (!isBackground && topLabel == stableLabel) {
    if (stableCount < 255) stableCount++;
  } else {
    stableLabel = topLabel;
    stableCount = (!isBackground && topLabel.length() > 0) ? 1 : 0;
  }

  // Pick per-label threshold
  float threshold = DETECTION_THRESHOLD;
  if (topLabel == "crosswalk") threshold = THRESH_CROSSWALK;
  else if (topLabel == "traffic_light") threshold = THRESH_TRAFFIC_LIGHT;

  if (!isBackground &&
      max_confidence >= threshold &&
      stableCount >= STABLE_FRAMES_REQUIRED &&
      confidence_gap >= 0.15f &&
      max_index >= 0) {
    
    String label = String(ei_classifier_inferencing_categories[max_index]);
    int confidence = (int)(max_confidence * 100);

  // Throttle repeated identical logs: only print when label changed or cooldown passed
  unsigned long now = millis();
  if (label != lastDetectionLabel || (now - lastDetectionTime) >= DETECTION_COOLDOWN) {
      // Print to Serial (only when detected and allowed by cooldown)
      Serial.print("✓ [DETECTED] ");
      Serial.print(label);
      Serial.print(" (confidence: ");
      Serial.print(confidence);
      Serial.println("%)");

      // Update last detection
      lastDetectionLabel = label;
      lastDetectionTime = now;

      // Display on OLED
      // OLED removed
    }
    
  } else {
    // No confident detection (or background)

    // If nothing detected for a while, clear lastDetectionLabel so same label can be reported again later
    if (millis() - lastDetectionTime > 5000) {
      lastDetectionLabel = "";
    }
  }
  
  // Timing information (commented out - only show when detected)
  // Serial.print("Inference time: ");
  // Serial.print(result.timing.classification);
  // Serial.println(" ms");
  // Serial.println();
  
  // Longer delay to let camera fully reset between captures
  delay(INFERENCE_DELAY);
}

// ============================================
// HELPER FUNCTIONS
// ============================================
