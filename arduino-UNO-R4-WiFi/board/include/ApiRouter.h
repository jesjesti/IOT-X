#pragma once

#include "HttpResponse.h"
#include "LEDHttpHandler.h"

class ApiRouter {
 public:
  explicit ApiRouter(LEDService& ledService);

  HttpResponse dispatch(const String& requestLine);

 private:
  LEDHttpHandler ledHandler_;
};
