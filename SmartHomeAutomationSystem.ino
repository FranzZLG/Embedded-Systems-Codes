#include <DHT.h>
#include <IRremote.h>

#define TEMP_SENSOR_PIN 4
#define RELAY_PIN 5
#define BUTTON_PIN_1 9
#define BUTTON_PIN_2 11
#define LED_PIN 6
#define IR_RECEIVER_PIN 13

IRrecv irrecv(IR_RECEIVER_PIN);
decode_results results;

#define TEMP_THRESHOLD 25
#define DHTTYPE DHT11

bool autoMode = true;
bool fanState = false;

unsigned long lastDebounceTime_1 = 0;
unsigned long lastDebounceTime_2 = 0;
unsigned long debounceDelay = 20;
bool lastButtonState_1 = LOW;
bool lastButtonState_2 = LOW;

DHT dht(TEMP_SENSOR_PIN, DHTTYPE);

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT);
  pinMode(BUTTON_PIN_2, INPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  irrecv.enableIRIn();

  Serial.begin(9600);
  dht.begin();
  Serial.println("System Initialized");
}

void loop() {
  checkButtons();

  if (irrecv.decode(&results)) {
    long int decCode = results.value;
    Serial.print("IR Code: ");
    Serial.println(decCode);
    irrecv.resume();

    if (decCode == 0xFF30CF) {
      fanState = !fanState;
      Serial.println("IR Remote: Fan state toggled");
    }
    if (decCode == 0xFF18E7) {
      autoMode = !autoMode;
      if (autoMode) {
        Serial.println("IR Remote: Switched to Auto mode");
      } else {
        Serial.println("IR Remote: Switched to Manual mode");
      }
    }
  }

  if (!autoMode) {
    Serial.println("Manual mode active");
    if (fanState) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Manual mode: Fan is ON, LED is ON");
    } else {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      Serial.println("Manual mode: Fan is OFF, LED is OFF");
    }
  } else {
    float temperature = dht.readTemperature() - 7;
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

      if (temperature > TEMP_THRESHOLD) {
        digitalWrite(RELAY_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Auto mode: Fan ON (Temperature exceeded threshold), LED ON");
      } else {
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        Serial.println("Auto mode: Fan OFF (Temperature below threshold), LED OFF");
      }
    }
  }

  delay(250);
}

void checkButtons() {
  bool currentButtonState_1 = digitalRead(BUTTON_PIN_1);
  bool currentButtonState_2 = digitalRead(BUTTON_PIN_2);

  if (currentButtonState_1 != lastButtonState_1 && (millis() - lastDebounceTime_1) > debounceDelay) {
    lastDebounceTime_1 = millis();
    if (currentButtonState_1 == HIGH) {
      fanState = !fanState;
      Serial.println("Button 1 pressed: Fan state toggled");
    }
  }
  lastButtonState_1 = currentButtonState_1;

  if (currentButtonState_2 != lastButtonState_2 && (millis() - lastDebounceTime_2) > debounceDelay) {
    lastDebounceTime_2 = millis();
    if (currentButtonState_2 == HIGH) {
      autoMode = !autoMode;
      if (autoMode) {
        Serial.println("Button 2 pressed: Switched to Auto mode");
      } else {
        Serial.println("Button 2 pressed: Switched to Manual mode");
      }
    }
  }
  lastButtonState_2 = currentButtonState_2;
}
