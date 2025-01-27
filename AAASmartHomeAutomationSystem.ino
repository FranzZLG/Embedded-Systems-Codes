#include <DHT.h>              // Include the DHT library
#include <IRremote.h>         // Include the IRremote library

// Define the pin numbers for components
#define TEMP_SENSOR_PIN 4      // KY-015 Temperature sensor pin (digital pin)
#define RELAY_PIN 5            // KY-019 Relay pin (connected to fan)
#define BUTTON_PIN_1 9         // Button 1 (manual fan control)
#define BUTTON_PIN_2 11        // Button 2 (mode toggle: Auto/Manual)
#define LED_PIN 6              // LED pin (connected to LED for visual fan status)
#define IR_RECEIVER_PIN 13      // KY-022 IR Receiver pin (connected to D10)

// IR receiver object
IRrecv irrecv(IR_RECEIVER_PIN);
decode_results results;        // Variable to store received IR data

#define TEMP_THRESHOLD 25       // Temperature threshold to turn on the fan (in °C)
#define DHTTYPE DHT11           // Define the DHT sensor type (DHT11 for KY-015)

// Variables for tracking mode and button state
bool autoMode = true;         // Default to Auto mode
bool fanState = false;         // Fan state (on/off)

// Variables for button debounce
unsigned long lastDebounceTime_1 = 0;
unsigned long lastDebounceTime_2 = 0;
unsigned long debounceDelay = 20;   // Delay for button debouncing
bool lastButtonState_1 = LOW;       // Previous state of Button 1
bool lastButtonState_2 = LOW;       // Previous state of Button 2

DHT dht(TEMP_SENSOR_PIN, DHTTYPE);  // Initialize the DHT sensor

void setup() {
  // Set the pin modes
  pinMode(RELAY_PIN, OUTPUT);     // Relay pin controls the fan
  pinMode(LED_PIN, OUTPUT);       // LED pin to show fan status
  pinMode(BUTTON_PIN_1, INPUT);   // Button 1 for manual control
  pinMode(BUTTON_PIN_2, INPUT);   // Button 2 for mode toggle

  // Initialize the relay and LED (turn off both initially)
  digitalWrite(RELAY_PIN, LOW);   
  digitalWrite(LED_PIN, LOW);     // LED off initially
  
  // Initialize the IR receiver
  irrecv.enableIRIn();           // Start the IR receiver

  // Start serial communication for debugging
  Serial.begin(9600);
  dht.begin();  // Initialize the DHT sensor
  Serial.println("System Initialized");
}

void loop() {
  // Check for button presses (debounced)
  checkButtons();

  // Check for IR remote commands
  if (irrecv.decode(&results)) {
    long int decCode = results.value;      // Get the received value
    Serial.print("IR Code: ");
    Serial.println(decCode);               // Print the received IR code for debugging
    irrecv.resume();                       // Receive the next value

    // Define actions based on IR codes (modify these to your remote's codes)
    if (decCode == 0xFF30CF) {            // Example IR code for "Power" (can change to your remote code)
      fanState = !fanState;               // Toggle fan state (on/off)
      Serial.println("IR Remote: Fan state toggled");
    } 
    if (decCode == 0xFF18E7) {            // Example IR code for "Mode" (can change to your remote code)
      autoMode = !autoMode;               // Toggle between Auto and Manual mode
      if (autoMode) {
        Serial.println("IR Remote: Switched to Auto mode");
      } else {
        Serial.println("IR Remote: Switched to Manual mode");
      }
    }
  }

  // Manual mode: Fan state is controlled by Button 1
  if (!autoMode) {
    Serial.println("Manual mode active");
    if (fanState) {
      digitalWrite(RELAY_PIN, HIGH);  // Turn on the fan
      digitalWrite(LED_PIN, HIGH);    // Turn on LED (fan on)
      Serial.println("Manual mode: Fan is ON, LED is ON");
    } else {
      digitalWrite(RELAY_PIN, LOW);   // Turn off the fan
      digitalWrite(LED_PIN, LOW);     // Turn off LED (fan off)
      Serial.println("Manual mode: Fan is OFF, LED is OFF");
    }
  } 
  // Automatic mode: Temperature-controlled fan
  else {
    float temperature = dht.readTemperature() - 7;  // Read temperature in °C
    float humidity = dht.readHumidity();        // Read humidity in %

    // Check if the readings are valid
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

      if (temperature > TEMP_THRESHOLD) {
        digitalWrite(RELAY_PIN, HIGH);  // Turn on the fan if temperature exceeds threshold
        digitalWrite(LED_PIN, HIGH);    // Turn on LED (fan on)
        Serial.println("Auto mode: Fan ON (Temperature exceeded threshold), LED ON");
      } else {
        digitalWrite(RELAY_PIN, LOW);   // Turn off the fan if temperature is below threshold
        digitalWrite(LED_PIN, LOW);     // Turn off LED (fan off)
        Serial.println("Auto mode: Fan OFF (Temperature below threshold), LED OFF");
      }
    }
  }

  delay(250);  // Short delay to reduce noise in readings
}

// Function to handle button presses with debouncing (detects only pressing)
void checkButtons() {
  // Read the current state of the buttons
  bool currentButtonState_1 = digitalRead(BUTTON_PIN_1);
  bool currentButtonState_2 = digitalRead(BUTTON_PIN_2);

  // Check Button 1 (manual fan control)
  if (currentButtonState_1 != lastButtonState_1 && (millis() - lastDebounceTime_1) > debounceDelay) {
    lastDebounceTime_1 = millis();  // Reset debounce timer
    if (currentButtonState_1 == HIGH) {
      fanState = !fanState;  // Toggle fan state (on/off)
      Serial.println("Button 1 pressed: Fan state toggled");
    }
  }
  lastButtonState_1 = currentButtonState_1;

  // Check Button 2 (mode toggle)
  if (currentButtonState_2 != lastButtonState_2 && (millis() - lastDebounceTime_2) > debounceDelay) {
    lastDebounceTime_2 = millis();  // Reset debounce timer
    if (currentButtonState_2 == HIGH) {
      autoMode = !autoMode;  // Toggle between Auto and Manual mode
      if (autoMode) {
        Serial.println("Button 2 pressed: Switched to Auto mode");
      } else {
        Serial.println("Button 2 pressed: Switched to Manual mode");
      }
    }
  }
  lastButtonState_2 = currentButtonState_2;
}
