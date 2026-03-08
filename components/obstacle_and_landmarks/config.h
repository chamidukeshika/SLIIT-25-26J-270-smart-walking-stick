#ifndef CONFIG_H
#define CONFIG_H

#define DFPLAYER_RX_PIN 13
#define DFPLAYER_TX_PIN 12

#define TRACK_CROSSWALK    1
#define TRACK_TRAFFIC      3
#define TRACK_OBSTACLE     2

#define DFPLAYER_VOLUME    25

#define ULTRASONIC_TRIG_PIN 14
#define ULTRASONIC_ECHO_PIN 15

#define OBSTACLE_THRESHOLD_CM  100
#define ULTRASONIC_POLL_MS     100
#define OBSTACLE_AUDIO_COOLDOWN_MS  3000

#define VIBRATION_MOTOR_PIN  2

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

#define CAMERA_FRAME_SIZE FRAMESIZE_96X96
#define JPEG_QUALITY 12
#define FRAME_BUFFER_COUNT 1

#define DETECTION_THRESHOLD      0.60
#define DETECTION_COOLDOWN       500
#define ANNOUNCE_MIN_CONF_VISION 0.45f
#define VISION_AUDIO_COOLDOWN_MS 30000

#define SERIAL_BAUD  115200
#define DEBUG_MODE   true
#define INFERENCE_DELAY 300

#define SYSTEM_NAME    "TinyML Vision"
#define SYSTEM_VERSION "2.0"

#endif
