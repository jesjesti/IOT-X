#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>


// Create Access Point credentials
const char* ssid = "ESP32_Control_AP";
const char* password = "12345678";

// ==== Motor driver pins ====
#define IN1 12
#define IN2 26
#define IN3 27
#define IN4 14

// ===== Servo setup =====
#define ROTOR_SERVO_PIN 21   

// Create web server on port 80
WebServer server(80);

Servo rotorServo;


void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void rotateLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void rotateRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
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

void handleRotorServo() {
  int rotateDegree = server.arg("angle").toInt();
  if(rotateDegree <=180 || rotateDegree >=0)
  {
    rotorServo.write(rotateDegree);
  }
}

// Root status
void handleRoot() {
  server.send(200, "text/plain", "ESP32 Robot Controller Active");
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  //Servo setup
  rotorServo.attach(ROTOR_SERVO_PIN);

  // Pin setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

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
  server.on("/rotor/servo", handleRotorServo);

  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  server.handleClient();
}