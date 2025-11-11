#include <WiFi.h>
#include <WebServer.h>

// Create Access Point credentials
const char* ssid = "ESP32_Control_AP";
const char* password = "12345678";

// Create web server on port 80
WebServer server(80);

// Handle root request
void handleRoot() {
  server.send(200, "text/plain", "ESP32 WiFi AP is working!");
}

// Example endpoints
void handleForward() {
  server.send(200, "text/plain", "Moving Forward...");
}

void handleBackward() {
  server.send(200, "text/plain", "Moving Backward...");
}

void handleRotateRight() {
  server.send(200, "text/plain", "Rotating Right...");
}

void handleRotateLeft() {
  server.send(200, "text/plain", "Rotating Left...");
}

void setup() {
  Serial.begin(115200);

  // Create Access Point
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();

  Serial.println("ESP32 Access Point Started");
  Serial.print("SSID: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  Serial.print("Access Point IP: "); Serial.println(IP);

  // Define routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/rotateRight", handleRotateRight);
  server.on("/rotateLeft", handleRotateLeft);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
