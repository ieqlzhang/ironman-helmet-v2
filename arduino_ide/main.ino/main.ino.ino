#include <ESP32Servo.h>

Servo myservo;
Servo myservo2;
int servoPin = 13;
int servoPin2 = 14;
const int max_degree = 100;
int currentAngle = 0;
int buttonPin = 12;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(9600);
  myservo.attach(servoPin);
  myservo2.attach(servoPin2);
  pinMode(buttonPin, INPUT_PULLUP);
  myservo.write(0);     // Servo 1 starts at 0°
  myservo2.write(max_degree);  // Servo 2 starts at max_degree
}

void loop() {
  int buttonState = digitalRead(buttonPin);
  
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW) {
      // Toggle servo position
      int newAngle1 = (currentAngle == 0) ? max_degree : 0;
      int newAngle2 = (newAngle1 == 0) ? max_degree : 0;
      currentAngle = newAngle1;
      myservo.write(newAngle1);
      myservo2.write(newAngle2);
      Serial.print("Servo 1: ");
      Serial.print(newAngle1);
      Serial.print("°, Servo 2: ");
      Serial.print(newAngle2);
      Serial.println("°");
      delay(300); // Simple cooldown between presses
    }
  }
  
  lastButtonState = buttonState;
}
