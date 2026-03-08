#ifndef CAMERA_HANDLER_H
#define CAMERA_HANDLER_H

#include "esp_camera.h"
#include "config.h"
#include "edge-impulse-sdk/dsp/image/image.hpp"

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3

static uint8_t *snapshot_buf = nullptr;

bool initCamera() {
  camera_config_t config;

  config.pin_d0    = Y2_GPIO_NUM;
  config.pin_d1    = Y3_GPIO_NUM;
  config.pin_d2    = Y4_GPIO_NUM;
  config.pin_d3    = Y5_GPIO_NUM;
  config.pin_d4    = Y6_GPIO_NUM;
  config.pin_d5    = Y7_GPIO_NUM;
  config.pin_d6    = Y8_GPIO_NUM;
  config.pin_d7    = Y9_GPIO_NUM;
  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.ledc_timer   = LEDC_TIMER_0;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count     = 1;
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

  if (psramFound()) {
    config.fb_location = CAMERA_FB_IN_PSRAM;
    Serial.println("PSRAM found");
  } else {
    config.fb_location = CAMERA_FB_IN_DRAM;
    Serial.println("PSRAM not found - using DRAM");
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s == NULL) {
    Serial.println("Failed to get camera sensor");
    return false;
  }

  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_ae_level(s, 0);
  s->set_aec_value(s, 300);
  s->set_gain_ctrl(s, 1);
  s->set_agc_gain(s, 0);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_hmirror(s, 0);
  s->set_vflip(s, 0);
  s->set_dcw(s, 1);
  s->set_colorbar(s, 0);

  return true;
}

camera_fb_t* captureImage() {
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

  const size_t raw_size = EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE;
  if (!snapshot_buf) {
    snapshot_buf = (uint8_t*)malloc(raw_size);
    if (!snapshot_buf) {
      Serial.println("ERR: Failed to allocate snapshot buffer");
      esp_camera_fb_return(fb);
      return NULL;
    }
  }

  bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
  esp_camera_fb_return(fb);

  if (!converted) {
    Serial.println("ERR: JPEG to RGB888 conversion failed");
    return NULL;
  }

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

  return (camera_fb_t*)0x1;
}

void releaseImage(camera_fb_t *fb) {
  if (fb != NULL) {
    esp_camera_fb_return(fb);
  }
}

#endif
