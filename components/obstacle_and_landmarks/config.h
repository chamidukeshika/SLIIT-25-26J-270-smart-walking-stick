/*
 * Configuration File
 * 
 * All settings and pin definitions for the TinyML Vision System
 * Change settings here instead of main code!
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// I2C PINS FOR OLED DISPLAY
// ============================================

#define I2C_SDA 14  // GPIO 14 for data
#define I2C_SCL 15  // GPIO 15 for clock

// ============================================
// OLED DISPLAY SETTINGS
// ============================================

#define SCREEN_WIDTH 128    // OLED width in pixels
#define SCREEN_HEIGHT 64    // OLED height in pixels
#define SCREEN_ADDRESS 0x3C // I2C address (or 0x3D for some displays)

// ============================================
// CAMERA PINS (AI-Thinker ESP32-CAM)
// ============================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ============================================
// CAMERA SETTINGS
// ============================================

// Frame size for inference (should match Edge Impulse model)
// FRAMESIZE_96X96 for 96x96 model
// FRAMESIZE_QQVGA for 160x120 model
// FRAMESIZE_QVGA for 320x240 model
#define CAMERA_FRAME_SIZE FRAMESIZE_96X96  // Match Edge Impulse model input

// JPEG quality (lower = better quality, higher file size)
// Range: 0-63 (higher number = lower quality but smaller size)
// Increased to reduce buffer overflow
#define JPEG_QUALITY 12  // Slightly lower quality for stability

// Frame buffer count (1 buffer forces fresh capture every time)
#define FRAME_BUFFER_COUNT 1  // Keep 1 buffer for stability

// ============================================
// AI MODEL SETTINGS
// ============================================

// Detection confidence threshold (0.0 to 1.0)
// Higher = more confident detections (fewer false positives)
// Lower = more detections (might have false positives)
// Recommended: 0.85 (85% confidence) - increased to reduce false detections
#define DETECTION_THRESHOLD 0.60

// Minimum time between detections (milliseconds)
// Prevents rapid switching between detections
#define DETECTION_COOLDOWN 500

// ============================================
// DISPLAY UPDATE SETTINGSr
// ============================================

// How long to show each detection (milliseconds)
#define DISPLAY_UPDATE_TIME 1000

// How long to show error messages (milliseconds)
#define ERROR_DISPLAY_TIME 2000

// ============================================
// SERIAL DEBUG SETTINGS
// ============================================

// Serial baud rate
#define SERIAL_BAUD 115200

// Enable verbose debug output (true/false)
#define DEBUG_MODE true

// ============================================
// TIMING SETTINGS
// ============================================

// Delay between inference cycles (milliseconds)
// Smaller = faster detection but uses more power
// Larger = slower detection but saves power
// Recommended: 100-500ms
#define INFERENCE_DELAY 300

// ============================================
// LABEL DISPLAY NAMES
// ============================================

// You can customize how labels appear on the display
// Keep them short to fit on small OLED screen!

#define LABEL_CROSSWALK "CROSSWALK"
#define LABEL_RED_LIGHT "RED LIGHT"
#define LABEL_YELLOW_LIGHT "YELLOW LIGHT"
#define LABEL_GREEN_LIGHT "GREEN LIGHT"

// ============================================
// SYSTEM INFO
// ============================================

#define SYSTEM_NAME "TinyML Vision"
#define SYSTEM_VERSION "1.0"

#endif // CONFIG_H
