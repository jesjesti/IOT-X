#pragma once

#include "HttpResponse.h"
#include "WiFiManager.h"

class WiFiHttpHandler {
 public:
  explicit WiFiHttpHandler(WiFiManager& wifiManager);

  bool handle(const String& requestLine, const String& body,
              HttpResponse& response);

 private:
  HttpResponse statusResponse() const;
  HttpResponse credentialsResponse(const String& body);

  WiFiManager& wifiManager_;
};
