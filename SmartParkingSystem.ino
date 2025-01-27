#define TRIG_PIN 9
#define ECHO_PIN 8
#define BUZZER_PIN 12
#define RED_LED_PIN 7
#define YELLOW_LED_PIN 5
#define GREEN_LED_PIN 3
#define BUTTON_PIN 4

#define GREEN_THRESHOLD 6
#define YELLOW_THRESHOLD 2
#define RED_THRESHOLD 2
#define NO_SOUND_THRESHOLD 10

enum Mode {AUTO_MODE, STANDBY_MODE};

Mode currentMode = AUTO_MODE;
bool buttonPressed = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int consecutiveUnderThreshold = 0;
const int thresholdCount = 5;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  checkButtonAndToggleMode();

  if (currentMode == AUTO_MODE) {
    autoModeFunctionality();
  } else if (currentMode == STANDBY_MODE) {
    standbyModeFunctionality();
  }

  delay(100);
}

void checkButtonAndToggleMode() {
  int buttonState = digitalRead(BUTTON_PIN);
  static int lastButtonState = HIGH;

  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && !buttonPressed) {
      currentMode = (currentMode == AUTO_MODE) ? STANDBY_MODE : AUTO_MODE;
      buttonPressed = true;

      if (currentMode == AUTO_MODE) {
        Serial.println("Switched to Auto Mode");
      } else {
        Serial.println("Switched to Standby Mode");
      }
    }
  }

  if (buttonState == HIGH) {
    buttonPressed = false;
  }

  lastButtonState = buttonState;
}

void autoModeFunctionality() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = (duration / 2) * 0.0344;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance <= RED_THRESHOLD) {
    consecutiveUnderThreshold++;
    Serial.print("Consecutive under 2cm: ");
    Serial.println(consecutiveUnderThreshold);

    if (consecutiveUnderThreshold >= thresholdCount) {
      Serial.println("Switching to Standby Mode due to consecutive detections under 2 cm.");
      currentMode = STANDBY_MODE;
    }

    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, HIGH);
    alertSound();
  } else {
    consecutiveUnderThreshold = 0;

    if (distance > NO_SOUND_THRESHOLD) {
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, LOW);
      noTone(BUZZER_PIN);
    } else if (distance > GREEN_THRESHOLD) {
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(RED_LED_PIN, LOW);
      fasterBeep();
    } else if (distance > YELLOW_THRESHOLD && distance <= GREEN_THRESHOLD) {
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, HIGH);
      digitalWrite(RED_LED_PIN, LOW);
      confirmationSound();
    }
  }

  delay(100);
}

void standbyModeFunctionality() {
  Serial.println("Standby Mode: Checking distance...");

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = (duration / 2) * 0.0344;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 8) {
    Serial.println("Distance is greater than 8 cm. Switching back to Auto Mode.");
    currentMode = AUTO_MODE;
  }

  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  noTone(BUZZER_PIN);

  delay(100);
}

void alertSound() {
  tone(BUZZER_PIN, 1000, 250);
  delay(250);
}

void fasterBeep() {
  tone(BUZZER_PIN, 1000, 100);
  delay(100);
  tone(BUZZER_PIN, 500, 100);
  delay(100);
}

void confirmationSound() {
  tone(BUZZER_PIN, 500, 300);
  delay(250);
}
