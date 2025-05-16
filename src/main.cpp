#include <ESP32Servo.h>

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

void setup() {
  Serial.begin(9600);
  myservo.attach(servoPin);
  myservo2.attach(servoPin2);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);      // Initialize LED pin
  
  // Initialize servos
  myservo.write(currentAngle1);
  myservo2.write(currentAngle2);
  analogWrite(ledPin, ledBrightness); // Initialize LED
  Serial.println("System ready");

}

void loop() {
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
