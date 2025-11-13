#include <WiFi.h>
#include <WebServer.h>

// Create Access Point credentials
const char* ssid = "ESP32_Control_AP";
const char* password = "12345678";

// ==== Motor driver pins ====
#define IN1 12
#define IN2 26
#define IN3 27
#define IN4 14
#define ENA 32   // PWM pin for left motors
#define ENB 33   // PWM pin for right motors

// ===== Servo setup =====
#define SERVO_PIN 21        // PWM signal wire to SG90
#define SERVO_CHANNEL 2     // Use a new LEDC channel for servo
#define SERVO_FREQ 50       // 50 Hz for servo
#define SERVO_RES 16        // 16-bit resolution

int motorSpeed = 100; // Default speed (0–255)

// Create web server on port 80
WebServer server(80);

// ===== Utility: Servo angle → duty =====
uint32_t angleToDuty(int angle) {
  int minUs = 500;
  int maxUs = 2400;
  int us = map(angle, 0, 180, minUs, maxUs);
  uint32_t value = (uint32_t)((float)us / 20000.0 * ((1 << SERVO_RES) - 1));
  return value;
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(0, 0);
  ledcWrite(1, 0);
}


void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(0, motorSpeed);
  ledcWrite(1, motorSpeed);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(0, motorSpeed);
  ledcWrite(1, motorSpeed);
}

void rotateLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  ledcWrite(0, motorSpeed);
  ledcWrite(1, motorSpeed);
}

void rotateRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  ledcWrite(0, motorSpeed);
  ledcWrite(1, motorSpeed);
}


// Movement APIs (press/hold control)
void handleForward() {
  String state = server.arg("state"); // "start" or "stop"
  if (state == "start") {
    moveForward();
    server.send(200, "text/plain", "Moving Forward");
  } else {
    stopMotors();
    server.send(200, "text/plain", "Stopped");
  }
}

void handleBackward() {
  String state = server.arg("state");
  if (state == "start") {
    moveBackward();
    server.send(200, "text/plain", "Moving Backward");
  } else {
    stopMotors();
    server.send(200, "text/plain", "Stopped");
  }
}

void handleRotateLeft() {
  String state = server.arg("state");
  if (state == "start") {
    rotateLeft();
    server.send(200, "text/plain", "Rotating Left");
  } else {
    stopMotors();
    server.send(200, "text/plain", "Stopped");
  }
}

void handleRotateRight() {
  String state = server.arg("state");
  if (state == "start") {
    rotateRight();
    server.send(200, "text/plain", "Rotating Right");
  } else {
    stopMotors();
    server.send(200, "text/plain", "Stopped");
  }
}

// Dynamic speed update
void handleSpeed() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    motorSpeed = constrain(val, 0, 255);
    server.send(200, "text/plain", "Speed updated to " + String(motorSpeed));
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter");
  }
}

// ===== Servo control =====
void handleServo() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    angle = constrain(angle, 0, 180);
    uint32_t duty = angleToDuty(angle);
    ledcWrite(SERVO_CHANNEL, duty);
    server.send(200, "text/plain", "Servo moved to " + String(angle) + "°");
    Serial.printf("Servo moved to %d°\n", angle);
  } else {
    server.send(400, "text/plain", "Missing 'angle' parameter");
  }
}

// Root status
void handleRoot() {
  server.send(200, "text/plain", "ESP32 Robot Controller Active");
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  // Pin setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // PWM setup
  ledcAttach(ENA, 1000, 8);  // channel 0 auto-assigned
  ledcAttach(ENB, 1000, 8);  // channel 1 auto-assigned
  ledcAttach(SERVO_PIN, SERVO_FREQ, SERVO_RES);  // Servo → channel 2 (50Hz, 16-bit)

  stopMotors();

  // Wi-Fi AP setup
  WiFi.softAP(ssid, password);
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());

  // Routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/rotateLeft", handleRotateLeft);
  server.on("/rotateRight", handleRotateRight);
  server.on("/speed", handleSpeed);
  server.on("/servo", handleServo);

  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  server.handleClient();
}