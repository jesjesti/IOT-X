#pragma once

#include <WiFiS3.h>

#include "ApiRouter.h"

class HttpServer {
 public:
  explicit HttpServer(ApiRouter& router);

  void begin();
  void handleClient();

 private:
  void sendResponse(WiFiClient& client, int status,
                    const char* statusText, const String& body);

  ApiRouter& router_;
  WiFiServer server_;
};
