#pragma once

#include "HttpResponse.h"
#include "LEDService.h"

class LEDHttpHandler {
 public:
  explicit LEDHttpHandler(LEDService& ledService);

  bool handle(const String& requestLine, HttpResponse& response);

 private:
  HttpResponse commandResponse() const;
  HttpResponse statusResponse() const;

  LEDService& ledService_;
};
