#pragma once

#include <Arduino.h>

class LEDController {
 public:
  void begin();
  void setOn(bool on);
};
