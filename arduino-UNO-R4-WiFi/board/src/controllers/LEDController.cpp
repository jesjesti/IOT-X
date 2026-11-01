#include "LEDController.h"

void LEDController::begin() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void LEDController::setOn(bool on) {
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}
