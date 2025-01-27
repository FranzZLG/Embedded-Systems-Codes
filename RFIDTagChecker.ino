#include <SPI.h>
#include <MFRC522.h>

#define SDA_PIN 10 
#define RST_PIN 9  

MFRC522 rfid(SDA_PIN, RST_PIN); 

void setup() {
  Serial.begin(9600); 
  SPI.begin();        
  rfid.PCD_Init();    
  Serial.println("Place an RFID tag near the reader...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return; // 
  }

  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Error reading RFID tag.");
    return;
  }

  Serial.print("RFID Tag UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  rfid.PICC_HaltA();
}
