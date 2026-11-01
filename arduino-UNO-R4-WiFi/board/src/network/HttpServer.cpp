#include "HttpServer.h"

#include <Arduino.h>

namespace {

constexpr int kPort = 80;

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

  client.setTimeout(1000);
  const String requestLine = client.readStringUntil('\r');
  client.readStringUntil('\n');

  if (requestLine.length() == 0) {
    sendResponse(client, 400, "Bad Request", "{\"error\":\"Invalid request\"}");
  } else {
    Serial.println("Request: " + requestLine);
    const HttpResponse response = router_.dispatch(requestLine);
    sendResponse(client, response.status, response.statusText, response.body);
  }

  client.stop();
}

void HttpServer::sendResponse(WiFiClient& client, int status,
                              const char* statusText, const String& body) {
  client.print("HTTP/1.1 ");
  client.print(status);
  client.print(" ");
  client.println(statusText);
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println();
  client.print(body);
}
