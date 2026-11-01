#pragma once

#include "LEDController.h"

class LEDService {
 public:
  explicit LEDService(LEDController& controller);

  void begin();
  void update();
  void startBlinking(unsigned long intervalMs);
  void stopBlinking();

  bool isBlinking() const;
  bool isOn() const;
  unsigned long blinkInterval() const;

 private:
  LEDController& controller_;
  unsigned long blinkIntervalMs_ = 200;
  unsigned long lastToggleMs_ = 0;
  bool blinking_ = false;
  bool ledOn_ = false;
};
