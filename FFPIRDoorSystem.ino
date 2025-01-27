#include <Servo.h>

int servoPin = 3;
int buttonPin = 4;
int pirPin = 2;

Servo doorServo;

bool doorOpen = false;
unsigned long lastMotionTime = 0;
unsigned long motionTimeout = 3000;
unsigned long manualOverrideTime = 0;
unsigned long manualOverrideDuration = 3000;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(pirPin, INPUT);
  doorServo.attach(servoPin);
  doorServo.write(0);
  Serial.begin(9600);
}

void loop() {
  bool buttonState = digitalRead(buttonPin);
  bool pirState = digitalRead(pirPin);

  if (pirState == HIGH && (millis() - manualOverrideTime > manualOverrideDuration)) {
    Serial.println("Motion Detected");
    lastMotionTime = millis();
    if (!doorOpen) {
      Serial.println("Opening Door due to Motion.");
      doorServo.write(180);
      doorOpen = true;
    }
  } else {
    Serial.println("No Motion Detected");
  }

  if (buttonState == LOW) {
    delay(50);
    if (doorOpen) {
      Serial.println("Closing Door due to Button.");
      doorServo.write(0);
      doorOpen = false;
    } else {
      Serial.println("Opening Door due to Button.");
      doorServo.write(180);
      doorOpen = true;
    }
    manualOverrideTime = millis();
    while (digitalRead(buttonPin) == LOW) {}
    delay(50);
  }

  if ((millis() - lastMotionTime >= motionTimeout) && doorOpen && (millis() - manualOverrideTime > manualOverrideDuration)) {
    Serial.println("Closing Door due to Timeout.");
    doorServo.write(0);
    doorOpen = false;
  }

  delay(500);
}
