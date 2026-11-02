#include "HttpServer.h"

#include <Arduino.h>

namespace {

constexpr int kPort = 80;
constexpr size_t kMaxRequestBodySize = 512;

}  // namespace

HttpServer::HttpServer(ApiRouter& router) : router_(router), server_(kPort) {}

void HttpServer::begin() {
  server_.begin();
  Serial.println("HTTP server started on port 80.");
}

void HttpServer::handleClient() {
  WiFiClient client = server_.available();
  if (!client) {
    return;
  }

  client.setTimeout(15000);
  const String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n');

  if (requestLine.length() == 0) {
    sendResponse(client, 400, "Bad Request", "{\"error\":\"Invalid request\"}");
  } else {
    Serial.println("Request: " + requestLine);
    size_t contentLength = 0;
    bool invalidContentLength = false;
    while (client.connected()) {
      String headerLine = client.readStringUntil('\n');
      if (headerLine == "\r" || headerLine.length() == 0) {
        break;
      }
      if (headerLine.endsWith("\r")) {
        headerLine.remove(headerLine.length() - 1);
      }
      const int separator = headerLine.indexOf(':');
      if (separator < 0) {
        continue;
      }
      String headerName = headerLine.substring(0, separator);
      headerName.toLowerCase();
      if (headerName == "content-length") {
        String headerValue = headerLine.substring(separator + 1);
        headerValue.trim();
        const long parsedLength = headerValue.toInt();
        if (parsedLength < 0 || String(parsedLength) != headerValue) {
          invalidContentLength = true;
        } else {
          contentLength = static_cast<size_t>(parsedLength);
        }
      }
    }

    if (invalidContentLength) {
      sendResponse(client, 400, "Bad Request",
                   "{\"error\":\"Invalid content length\"}");
    } else if (contentLength > kMaxRequestBodySize) {
      sendResponse(client, 413, "Payload Too Large",
                   "{\"error\":\"Request body is too large\"}");
    } else {
      char bodyBuffer[kMaxRequestBodySize];
      const size_t bytesRead =
          contentLength == 0 ? 0 : client.readBytes(bodyBuffer, contentLength);
      if (bytesRead != contentLength) {
        sendResponse(client, 400, "Bad Request",
                     "{\"error\":\"Incomplete request body\"}");
      } else {
        String body;
        body.reserve(bytesRead);
        for (size_t index = 0; index < bytesRead; ++index) {
          body += bodyBuffer[index];
        }
        const HttpResponse response = router_.dispatch(requestLine, body);
        sendResponse(client, response.status, response.statusText,
                     response.body);
      }
    }
  }

  client.flush();
  client.stop();
}

void HttpServer::sendResponse(WiFiClient& client, int status,
                              const char* statusText, const String& body) {
  client.print("HTTP/1.1 ");
  client.print(status);
  client.print(" ");
  client.println(statusText);
  client.println("Content-Type: application/json");
  client.println("Cache-Control: no-store");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println();
  client.print(body);
}
