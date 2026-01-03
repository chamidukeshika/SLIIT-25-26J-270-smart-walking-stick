/*
 * Camera Handler
 * 
 * Functions to initialize and use the ESP32-CAM camera
 */

#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include "esp_camera.h"
#include "config.h"
// Edge Impulse image helpers (crop/resize)
#include "edge-impulse-sdk/dsp/image/image.hpp"

// Match the official EI example: capture QVGA JPEG, then convert to RGB888
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3

static uint8_t *snapshot_buf = nullptr; // RGB888 buffer after conversion + crop/resize

// ============================================
// CAMERA INITIALIZATION
// ============================================

bool initCamera() {
  // Configure camera settings
  camera_config_t config;
  
  // Pin configuration for AI-Thinker ESP32-CAM
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  
  // Clock and format settings
  // Match EI example settings
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.ledc_timer = LEDC_TIMER_0;      // Specify timer
  config.ledc_channel = LEDC_CHANNEL_0;  // Specify channel
  
  // Capture at QVGA like the EI example, then we crop+resize to the model input
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  
  // Use PSRAM if available (ESP32-CAM has 4MB PSRAM)
  if (psramFound()) {
    config.fb_location = CAMERA_FB_IN_PSRAM;
    Serial.println("PSRAM found - using it for frame buffer");
  } else {
    config.fb_location = CAMERA_FB_IN_DRAM;
    Serial.println("PSRAM not found - using DRAM");
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }
  
  // Get camera sensor for adjustments
  sensor_t *s = esp_camera_sensor_get();
  
  if (s == NULL) {
    Serial.println("Failed to get camera sensor");
    return false;
  }
  
  // Adjust camera settings for better performance
  s->set_brightness(s, 0);      // -2 to 2
  s->set_contrast(s, 0);        // -2 to 2
  s->set_saturation(s, 0);      // -2 to 2
  s->set_special_effect(s, 0);  // 0 to 6 (0 = no effect)
  s->set_whitebal(s, 1);        // 0 = disable, 1 = enable
  s->set_awb_gain(s, 1);        // 0 = disable, 1 = enable
  s->set_wb_mode(s, 0);         // 0 to 4
  s->set_exposure_ctrl(s, 1);   // 0 = disable, 1 = enable
  s->set_aec2(s, 0);            // 0 = disable, 1 = enable
  s->set_ae_level(s, 0);        // -2 to 2
  s->set_aec_value(s, 300);     // 0 to 1200
  s->set_gain_ctrl(s, 1);       // 0 = disable, 1 = enable
  s->set_agc_gain(s, 0);        // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);             // 0 = disable, 1 = enable (black pixel correction)
  s->set_wpc(s, 1);             // 0 = disable, 1 = enable (white pixel correction)
  s->set_raw_gma(s, 1);         // 0 = disable, 1 = enable
  s->set_lenc(s, 1);            // 0 = disable, 1 = enable (lens correction)
  s->set_hmirror(s, 0);         // 0 = disable, 1 = enable (horizontal mirror)
  s->set_vflip(s, 0);           // 0 = disable, 1 = enable (vertical flip)
  s->set_dcw(s, 1);             // 0 = disable, 1 = enable (downsize)
  s->set_colorbar(s, 0);        // 0 = disable, 1 = enable (test pattern)
  
  return true;
}

// ============================================
// CAPTURE IMAGE
// ============================================

camera_fb_t* captureImage() {
  // Capture fresh image from camera (JPEG, QVGA)
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return NULL;
  }
  if (fb->len == 0) {
    Serial.println("Captured image is empty");
    esp_camera_fb_return(fb);
    return NULL;
  }

  // Allocate RGB888 buffer once
  const size_t raw_size = EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE;
  if (!snapshot_buf) {
    snapshot_buf = (uint8_t*)malloc(raw_size);
    if (!snapshot_buf) {
      Serial.println("ERR: Failed to allocate snapshot buffer");
      esp_camera_fb_return(fb);
      return NULL;
    }
  }

  // Convert JPEG -> RGB888 into snapshot_buf
  bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
  esp_camera_fb_return(fb);

  if (!converted) {
    Serial.println("ERR: JPEG to RGB888 conversion failed");
    return NULL;
  }

  // Crop + resize in-place to model input size (still RGB888)
  if (EI_CLASSIFIER_INPUT_WIDTH != EI_CAMERA_RAW_FRAME_BUFFER_COLS ||
      EI_CLASSIFIER_INPUT_HEIGHT != EI_CAMERA_RAW_FRAME_BUFFER_ROWS) {
    ei::image::processing::crop_and_interpolate_rgb888(
      snapshot_buf,
      EI_CAMERA_RAW_FRAME_BUFFER_COLS,
      EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
      snapshot_buf,
      EI_CLASSIFIER_INPUT_WIDTH,
      EI_CLASSIFIER_INPUT_HEIGHT);
  }

  // Return non-null sentinel; caller uses global snapshot_buf via ei_camera_get_data()
  return (camera_fb_t*)0x1;
}

// ============================================
// RELEASE IMAGE BUFFER
// ============================================

void releaseImage(camera_fb_t *fb) {
  if (fb != NULL) {
    esp_camera_fb_return(fb);
  }
}

#endif // CAMERA_HANDLER_H
