#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

#include "credentials.h"

WebServer server(80);
void handleRoot();
void handleControl();
void handleToggleEndpoint();
Servo myservo;
Servo myservo2;
int servoPin = 20;
int servoPin2 = 21;
const int max_degree = 130;
int currentAngle1 = 0;
int currentAngle2 = max_degree;

// Servo control variables
const int servoStep = 13;        // 每次转动角度
const int stepDelay = 50;        // 步进间隔时间(ms)

// LED control variables
const int ledPin = 2;          // PWM capable pin for LED
const int fadeDelay = 5;       // 5ms per step delay
int ledBrightness = 255;         // Current LED brightness
unsigned long lastActionTime = 0;
const int cooldownPeriod = 1000; // 1 second cooldown

// Button control variables
int buttonPin = 1;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Shared toggle function
void handleToggle() {
  if (millis() - lastActionTime < cooldownPeriod) {
    return;
  }
  
  int newAngle1 = (currentAngle1 == 0) ? max_degree : 0;
  int newAngle2 = (currentAngle2 == max_degree) ? 0 : max_degree;
  
  while(currentAngle1 != newAngle1 || currentAngle2 != newAngle2) {
    int servoStep1 = (newAngle1 > currentAngle1) ? 1 : -1;
    int servoStep2 = (newAngle2 > currentAngle2) ? 1 : -1;
    
    currentAngle1 += servoStep1*servoStep;
    currentAngle2 += servoStep2*servoStep;
    
    myservo.write(currentAngle1);
    myservo2.write(currentAngle2);
    
    delay(stepDelay);
    
    currentAngle1 = constrain(currentAngle1, 0, max_degree);
    currentAngle2 = constrain(currentAngle2, 0, max_degree);
  }

  int target = (currentAngle1 == 0) ? 255 : 0;
  for(int brightness = ledBrightness; brightness != target; 
      brightness += (target > brightness) ? 1 : -1) {
    analogWrite(ledPin, brightness);
    ledBrightness = brightness;
    delay(fadeDelay);
  }
  
  lastActionTime = millis();
}

void setup() {
  Serial.begin(9600);
  delay(100);  // Allow serial port to initialize
  Serial.println("\n\nSerial communication initialized");
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nConnection failed. Status: " + String(WiFi.status()));
    Serial.println("Possible causes:");
    Serial.println("1. Incorrect WiFi credentials");
    Serial.println("2. Weak signal strength");
    Serial.println("3. Router not compatible");
    ESP.restart();
  }
  
  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  myservo.attach(servoPin);
  myservo2.attach(servoPin2);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);      // Initialize LED pin
  
  // Initialize servos
  myservo.write(currentAngle1);
  myservo2.write(currentAngle2);
  analogWrite(ledPin, ledBrightness); // Initialize LED

  // Configure web server routes
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/toggle", HTTP_POST, handleToggleEndpoint);
  server.begin();
  
  Serial.println("System ready");
}

// Web server handlers
void handleRoot() {
  server.send(200, "text/html", "<html><body style='text-align:center'><h1>Helmet Control</h1><a href='/control'>Control Panel</a></body></html>");
}

void handleControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", R"=====(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        .switch {position:relative;display:inline-block;width:60px;height:34px}
        .slider {position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:#ccc;transition:.4s}
        .slider:before {position:absolute;content:"";height:26px;width:26px;left:4px;bottom:4px;background:white;transition:.4s}
        input:checked + .slider {background: #2196F3}
        input:checked + .slider:before {transform:translateX(26px)}
        .status {margin-top:20px;padding:10px;border-radius:5px;color:white;font-weight:bold}
      </style>
    </head>
    <body style="text-align:center;padding:20px">
      <h1>IRONMAN HELMET CONTROL</h1>
      <label class="switch">
        <input type="checkbox" id="toggle" onchange="toggleHelmet()">
        <span class="slider"></span>
      </label>
      <div id="status" class="status"></div>
      <script>
        function toggleHelmet() {
          fetch('/toggle', {method: 'POST'})
            .then(r => r.json())
            .then(data => {
              document.getElementById('status').textContent = data.state;
              document.getElementById('status').style.backgroundColor = data.state === 'OPEN' ? '#4CAF50' : '#f44336';
            });
        }
      </script>
    </body>
    </html>
  )=====");
}

void handleToggleEndpoint() {
  handleToggle();
  String state = (currentAngle1 == 0) ? "CLOSED" : "OPEN";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"state\":\"" + state + "\"}");
}

void loop() {
  server.handleClient();
  
  // Handle button press
  int buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW) {
      int newAngle1 = (currentAngle1 == 0) ? max_degree : 0;
      int newAngle2 = (currentAngle2 == max_degree) ? 0 : max_degree;
      
      // 逐步移动到目标角度
      while(currentAngle1 != newAngle1 || currentAngle2 != newAngle2) {
        // 计算步进角度
        int servoStep1 = (newAngle1 > currentAngle1) ? 1 : -1;
        int servoStep2 = (newAngle2 > currentAngle2) ? 1 : -1;
        
        // 更新当前角度
        currentAngle1 += servoStep1*servoStep;
        currentAngle2 += servoStep2*servoStep;
        
        // 写入舵机角度
        myservo.write(currentAngle1);
        myservo2.write(currentAngle2);
        
        delay(stepDelay);
        
        // 防止超出范围
        currentAngle1 = constrain(currentAngle1, 0, max_degree);
        currentAngle2 = constrain(currentAngle2, 0, max_degree);
      }
      
      // 执行LED渐变
      int target = (currentAngle1 == 0) ? 255 : 0;
      for(int brightness = ledBrightness; brightness != target; 
          brightness += (target > brightness) ? 1 : -1) {
        analogWrite(ledPin, brightness);
        ledBrightness = brightness;
        delay(fadeDelay);
      }
      
      // Update cooldown timer
      lastActionTime = millis();
      Serial.print("Faceplate ");
      Serial.print((currentAngle1 == 0) ? "closed" : "open");
      Serial.println(" | LED transition completed");
    }
  }
  
  // Enforce cooldown period
  if (millis() - lastActionTime < cooldownPeriod) {
    return;
  }
  
  lastButtonState = buttonState;
}
