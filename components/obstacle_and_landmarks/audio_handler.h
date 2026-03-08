#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "config.h"

static HardwareSerial dfSerial(2);
static DFRobotDFPlayerMini dfPlayer;
static bool dfPlayerReady = false;

static uint32_t _lastPlayedCrosswalk = 0;
static uint32_t _lastPlayedTraffic   = 0;

bool audio_init() {
  Serial.println("[AUDIO] Starting DFPlayer init...");

  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
  delay(1000);

  if (!dfPlayer.begin(dfSerial, false, false)) {
    Serial.println("[AUDIO] DFPlayer Mini not found!");
    dfPlayerReady = false;
    return false;
  }

  delay(200);
  dfPlayer.volume(DFPLAYER_VOLUME);
  delay(100);
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  delay(100);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  delay(200);

  dfPlayerReady = true;

  int fileCount = dfPlayer.readFileCounts();
  Serial.println("[AUDIO] DFPlayer Mini ready!");
  Serial.print("[AUDIO] Volume: ");
  Serial.println(DFPLAYER_VOLUME);
  Serial.print("[AUDIO] Files on SD: ");
  Serial.println(fileCount);

  return true;
}

void audio_play_track(uint8_t track) {
  if (!dfPlayerReady) return;
  dfPlayer.play(track);
}

void audio_play_crosswalk() {
  if (!dfPlayerReady) return;
  Serial.println("[AUDIO] Crosswalk ahead");
  dfPlayer.play(TRACK_CROSSWALK);
}

void audio_play_traffic() {
  if (!dfPlayerReady) return;
  Serial.println("[AUDIO] Traffic light ahead");
  dfPlayer.play(TRACK_TRAFFIC);
}

void audio_play_obstacle() {
  if (!dfPlayerReady) return;
  Serial.println("[AUDIO] Obstacle detected");
  dfPlayer.play(TRACK_OBSTACLE);
}

#endif
