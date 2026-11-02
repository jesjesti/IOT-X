#pragma once

#include "HttpResponse.h"
#include "LEDHttpHandler.h"
#include "WiFiHttpHandler.h"

class ApiRouter {
 public:
  ApiRouter(LEDService& ledService, WiFiManager& wifiManager);

  HttpResponse dispatch(const String& requestLine, const String& body);

 private:
  LEDHttpHandler ledHandler_;
  WiFiHttpHandler wifiHandler_;
};
