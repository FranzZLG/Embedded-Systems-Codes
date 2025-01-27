#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define SDA_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 5
#define SERVO_PIN 6
#define GREEN_LED_PIN 3
#define RED_LED_PIN 4
#define OLED_RESET 4

Adafruit_SH1106 display(OLED_RESET);

MFRC522 rfid(SDA_PIN, RST_PIN);
Servo doorServo;

String authorizedTag = "43a0ed2c";
bool doorUnlocked = false;

void setup() {
  Serial.begin(9600);
  Serial.println("System initializing...");

  SPI.begin();
  rfid.PCD_Init();
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  doorServo.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  
  doorServo.write(90);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  Serial.println("System Initialized.");
  Serial.println("Door Locked. Waiting for RFID tag...");
  display.setCursor(2, 20);
  display.println("System Initialized.");
  display.setCursor(2, 40);
  display.println("RFID Lock System");
  display.display();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String tagUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) tagUID += "0";
    tagUID += String(rfid.uid.uidByte[i], HEX);
  }
  tagUID.toLowerCase();
  Serial.print("Scanned RFID Tag UID: ");
  Serial.println(tagUID);

  if (tagUID == authorizedTag) {
    if (doorUnlocked) {
      lockDoor();
    } else {
      unlockDoor();
    }
  } else {
    denyAccess();
  }

  rfid.PICC_HaltA();
}

void unlockDoor() {
  Serial.println("Door Unlocked");
  digitalWrite(GREEN_LED_PIN, HIGH);
  tone(BUZZER_PIN, 900);
  delay(200);
  tone(BUZZER_PIN, 1200);
  delay(200);
  noTone(BUZZER_PIN);

  doorServo.write(0);
  doorUnlocked = true;

  digitalWrite(GREEN_LED_PIN, LOW);
}

void lockDoor() {
  Serial.println("Door Locked");
  digitalWrite(GREEN_LED_PIN, HIGH);
  tone(BUZZER_PIN, 1000);
  delay(200);
  tone(BUZZER_PIN, 500);
  delay(200);
  noTone(BUZZER_PIN);

  doorServo.write(90);
  doorUnlocked = false;

  digitalWrite(GREEN_LED_PIN, LOW);
}

void denyAccess() {
  Serial.println("Access denied.");

  digitalWrite(RED_LED_PIN, HIGH);

  tone(BUZZER_PIN, 500);
  delay(250);
  tone(BUZZER_PIN, 250);
  delay(250);
  tone(BUZZER_PIN, 500);
  delay(250);
  noTone(BUZZER_PIN);

  digitalWrite(RED_LED_PIN, LOW);
}