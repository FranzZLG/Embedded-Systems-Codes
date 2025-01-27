#define TRIG_PIN 9   // Pin connected to the Trigger of Ultrasonic Sensor
#define ECHO_PIN 8   // Pin connected to the Echo of Ultrasonic Sensor
#define BUZZER_PIN 12 // Pin connected to the passive buzzer
#define RED_LED_PIN 7    // Pin connected to Red LED
#define YELLOW_LED_PIN 5 // Pin connected to Yellow LED
#define GREEN_LED_PIN 3  // Pin connected to Green LED
#define BUTTON_PIN 4      // Pin connected to Button to switch modes

// Define distance thresholds (in cm) for LED and buzzer activation
#define GREEN_THRESHOLD 6  // Distance greater than 6 cm (Green LED activates)
#define YELLOW_THRESHOLD 2 // Distance between 2 cm and 6 cm (Yellow LED activates)
#define RED_THRESHOLD 2    // Distance less than or equal to 2 cm (Red LED activates)
#define NO_SOUND_THRESHOLD 10 // Distance greater than 10 cm (no sound or LED activation)

enum Mode {AUTO_MODE, STANDBY_MODE};  // Enum to define the two modes

Mode currentMode = AUTO_MODE;  // Default to Auto Mode
bool buttonPressed = false;   // To track the button state
unsigned long lastDebounceTime = 0;  // Debounce timer
unsigned long debounceDelay = 50;    // Debounce delay for button press

int consecutiveUnderThreshold = 0;  // Counter for consecutive distance detections < 2 cm
const int thresholdCount = 5;        // Number of consecutive detections to switch to Standby Mode

void setup() {
  pinMode(TRIG_PIN, OUTPUT);  // Set Trigger pin as an output to send ultrasonic pulses
  pinMode(ECHO_PIN, INPUT);   // Set Echo pin as an input to receive the pulse return time
  pinMode(BUZZER_PIN, OUTPUT); // Set Buzzer pin as an output to play sounds
  pinMode(RED_LED_PIN, OUTPUT);   // Set Red LED pin as an output
  pinMode(YELLOW_LED_PIN, OUTPUT); // Set Yellow LED pin as an output
  pinMode(GREEN_LED_PIN, OUTPUT);  // Set Green LED pin as an output
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set Button pin as an input with internal pull-up resistor

  Serial.begin(9600); // Start serial communication for debugging purposes
}

void loop() {
  // Check button state to toggle between modes
  checkButtonAndToggleMode();

  if (currentMode == AUTO_MODE) {
    // Auto Mode functionality
    autoModeFunctionality();
  } else if (currentMode == STANDBY_MODE) {
    // Standby Mode functionality (turn off everything unless distance > 6 cm)
    standbyModeFunctionality();
  }

  delay(100); // Small delay for stability
}

// Function to check button press and toggle between Auto Mode and Standby Mode with debounce
void checkButtonAndToggleMode() {
  int buttonState = digitalRead(BUTTON_PIN);
  static int lastButtonState = HIGH;  // Tracks the last button state

  // Check for button press (HIGH to LOW change)
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();  // Reset the debounce timer
  }

  // Only toggle mode if the button is pressed and debounce delay has passed
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && !buttonPressed) {
      // Toggle between Auto Mode and Standby Mode
      currentMode = (currentMode == AUTO_MODE) ? STANDBY_MODE : AUTO_MODE;
      buttonPressed = true;  // Mark the button as pressed

      // Print current mode to the serial monitor for debugging
      if (currentMode == AUTO_MODE) {
        Serial.println("Switched to Auto Mode");
      } else {
        Serial.println("Switched to Standby Mode");
      }
    }
  }

  // Reset buttonPressed flag when button is released
  if (buttonState == HIGH) {
    buttonPressed = false;
  }

  lastButtonState = buttonState;  // Update the last button state
}

// Auto Mode Functionality
void autoModeFunctionality() {
  // Send a pulse to the ultrasonic sensor to measure distance
  digitalWrite(TRIG_PIN, LOW);  // Set Trigger pin low for 2 microseconds to reset
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); // Set Trigger pin high for 10 microseconds to send the pulse
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);  // Set Trigger pin low again to end the pulse
  
  // Read the echo time from the Echo pin and calculate the distance
  long duration = pulseIn(ECHO_PIN, HIGH); // Measure the duration of the echo pulse
  long distance = (duration / 2) * 0.0344; // Calculate distance in cm (time * speed of sound)

  // Print the distance to the serial monitor for debugging
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // If distance is less than 2 cm, increment the counter
  if (distance <= RED_THRESHOLD) {
    consecutiveUnderThreshold++;  // Increment counter when distance is under 2 cm
    Serial.print("Consecutive under 2cm: ");
    Serial.println(consecutiveUnderThreshold);

    // If the distance is under 2 cm for 5 consecutive times, switch to Standby Mode
    if (consecutiveUnderThreshold >= thresholdCount) {
      Serial.println("Switching to Standby Mode due to consecutive detections under 2 cm.");
      currentMode = STANDBY_MODE;  // Switch to Standby Mode
    }

    // Trigger the Red LED and alert sound
    digitalWrite(GREEN_LED_PIN, LOW);   // Turn off Green LED
    digitalWrite(YELLOW_LED_PIN, LOW);  // Turn off Yellow LED
    digitalWrite(RED_LED_PIN, HIGH);    // Turn on Red LED
    alertSound(); // Call alert sound function for Red LED
  } else {
    // Reset counter if distance is above 2 cm
    consecutiveUnderThreshold = 0;  // Reset counter if distance is not under 2 cm

    // Control LEDs and buzzer based on distance
    if (distance > NO_SOUND_THRESHOLD) {
      // If distance is greater than 10 cm, turn off all LEDs and buzzer
      digitalWrite(GREEN_LED_PIN, LOW);  // Turn off Green LED
      digitalWrite(YELLOW_LED_PIN, LOW); // Turn off Yellow LED
      digitalWrite(RED_LED_PIN, LOW);    // Turn off Red LED
      noTone(BUZZER_PIN);  // Turn off buzzer
    }
    else if (distance > GREEN_THRESHOLD) {
      // If distance is between 6 cm and 10 cm, activate Green LED with faster beep
      digitalWrite(GREEN_LED_PIN, HIGH);  // Turn on Green LED
      digitalWrite(YELLOW_LED_PIN, LOW);  // Turn off Yellow LED
      digitalWrite(RED_LED_PIN, LOW);     // Turn off Red LED
      fasterBeep(); // Call the faster beeping sound function for Green LED
    }
    else if (distance > YELLOW_THRESHOLD && distance <= GREEN_THRESHOLD) {
      // If distance is between 2 cm and 6 cm, activate Yellow LED with slower beep
      digitalWrite(GREEN_LED_PIN, LOW);   // Turn off Green LED
      digitalWrite(YELLOW_LED_PIN, HIGH); // Turn on Yellow LED
      digitalWrite(RED_LED_PIN, LOW);     // Turn off Red LED
      confirmationSound(); // Call the slower beep function for Yellow LED
    }
  }

  delay(100); // Short delay for stability between iterations
}

// Standby Mode Functionality (turn off everything unless distance > 6 cm)
void standbyModeFunctionality() {
  Serial.println("Standby Mode: Checking distance...");

  // Send a pulse to the ultrasonic sensor to measure distance
  digitalWrite(TRIG_PIN, LOW);  // Set Trigger pin low for 2 microseconds to reset
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); // Set Trigger pin high for 10 microseconds to send the pulse
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);  // Set Trigger pin low again to end the pulse

  // Read the echo time from the Echo pin and calculate the distance
  long duration = pulseIn(ECHO_PIN, HIGH); // Measure the duration of the echo pulse
  long distance = (duration / 2) * 0.0344; // Calculate distance in cm (time * speed of sound)

  // Print the distance to the serial monitor for debugging
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 8) {
    // If distance is greater than 8 cm, switch back to Auto Mode
    Serial.println("Distance is greater than 8 cm. Switching back to Auto Mode.");
    currentMode = AUTO_MODE;  // Switch back to Auto Mode
  }
  
  // In Standby Mode, turn off everything
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  noTone(BUZZER_PIN);  // Turn off buzzer

  delay(100); // Short delay for stability
}

// Function to produce an alert sound (Red LED)
void alertSound() {
  tone(BUZZER_PIN, 1000, 250); // Beep at 1000 Hz for 250 ms
  delay(250); // Short delay before next alert
}

// Function to produce a faster beeping sound (Green LED)
void fasterBeep() {
  tone(BUZZER_PIN, 1000, 100);  // High tone at 1000 Hz for 100 ms
  delay(100);                   // Short delay before next tone
  tone(BUZZER_PIN, 500, 100);   // Low tone at 500 Hz for 100 ms
  delay(100);                   // Short delay before the next loop
}

// Function to produce a confirmation sound (Yellow LED)
void confirmationSound() {
  tone(BUZZER_PIN, 500, 300); // Beep at 500 Hz for 300 ms
  delay(250); // Short delay before next confirmation
}
