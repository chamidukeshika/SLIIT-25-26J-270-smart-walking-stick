#ifndef ULTRASONIC_HANDLER_H
#define ULTRASONIC_HANDLER_H

#include "config.h"
#include "audio_handler.h"

static uint32_t _ultrasonicLastPoll = 0;
static uint32_t _obstacleLastAudio  = 0;
static bool     _obstacleActive     = false;
static float    _lastDistanceCm     = 9999.0f;

void ultrasonic_init() {
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  pinMode(VIBRATION_MOTOR_PIN, OUTPUT);
  digitalWrite(VIBRATION_MOTOR_PIN, LOW);
  Serial.println("[ULTRASONIC] Ready");
}

static float _measure_distance_cm() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return 9999.0f;
  return (float)duration * 0.034f / 2.0f;
}

void ultrasonic_update() {
  uint32_t now = millis();
  if (now - _ultrasonicLastPoll < ULTRASONIC_POLL_MS) return;
  _ultrasonicLastPoll = now;

  _lastDistanceCm = _measure_distance_cm();

  Serial.print("[ULTRASONIC] Distance: ");
  Serial.print(_lastDistanceCm, 1);
  Serial.println(" cm");

  if (_lastDistanceCm < (float)OBSTACLE_THRESHOLD_CM) {
    if (!_obstacleActive) {
      _obstacleActive = true;
      Serial.println("[ULTRASONIC] OBSTACLE DETECTED - motor ON");
    }
    digitalWrite(VIBRATION_MOTOR_PIN, HIGH);
    if (now - _obstacleLastAudio >= OBSTACLE_AUDIO_COOLDOWN_MS) {
      audio_play_obstacle();
      _obstacleLastAudio = now;
    }
  } else {
    if (_obstacleActive) {
      _obstacleActive = false;
      Serial.println("[ULTRASONIC] Path clear - motor OFF");
    }
    digitalWrite(VIBRATION_MOTOR_PIN, LOW);
  }
}

float ultrasonic_get_distance() {
  return _lastDistanceCm;
}

bool ultrasonic_obstacle_active() {
  return _obstacleActive;
}

#endif
