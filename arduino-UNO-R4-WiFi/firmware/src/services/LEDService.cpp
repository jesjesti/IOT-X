#include "LEDService.h"

#include <Arduino.h>

LEDService::LEDService(LEDController& controller) : controller_(controller) {}

void LEDService::begin() {
  controller_.begin();
  lastToggleMs_ = millis();
}

void LEDService::update() {
  if (!blinking_) {
    return;
  }

  const unsigned long now = millis();
  if (now - lastToggleMs_ < blinkIntervalMs_) {
    return;
  }

  lastToggleMs_ = now;
  ledOn_ = !ledOn_;
  controller_.setOn(ledOn_);
}

void LEDService::startBlinking(unsigned long intervalMs) {
  if (intervalMs == 0) {
    return;
  }

  blinkIntervalMs_ = intervalMs;
  blinking_ = true;
  lastToggleMs_ = millis();
}

void LEDService::stopBlinking() {
  blinking_ = false;
  ledOn_ = false;
  controller_.setOn(false);
}

bool LEDService::isBlinking() const {
  return blinking_;
}

bool LEDService::isOn() const {
  return ledOn_;
}

unsigned long LEDService::blinkInterval() const {
  return blinkIntervalMs_;
}
