#pragma once

#include <Arduino.h>

struct HttpResponse {
  int status;
  const char* statusText;
  String body;
};
