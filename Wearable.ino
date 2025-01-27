#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

const int heartSensorPin = A0;
OneWire ds(10);

void setup() {
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  pinMode(heartSensorPin, INPUT);

  Serial.println("Heart Rate and Temperature Monitoring System");
}

void loop() {
  int heartSignal = analogRead(heartSensorPin);

  byte i;
  byte present = 0;
  byte type_s;
  byte data[9];
  byte addr[8];
  float celsius, fahrenheit;

  if (!ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

  Serial.print("ROM =");
  for (i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  Serial.println();

  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  delay(1000);

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for (i = 0; i < 9; i++) {
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3;
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;
    else if (cfg == 0x20) raw = raw & ~3;
    else if (cfg == 0x40) raw = raw & ~1;
  }
  celsius = (float)raw / 16.0;

  display.clearDisplay();
  display.setCursor(0, 16);
  display.println("Smart Sensor");

  display.setCursor(0, 32);
  display.print("Heart Rate:");
  display.println(heartSignal);

  display.setCursor(0, 48);
  display.print("Temperature:");
  display.print(celsius);
  display.println(" C");

  display.display();

  Serial.print("Heartbeat Signal:");
  Serial.println(heartSignal);

  Serial.print("Temperature:");
  Serial.print(celsius);
  Serial.println("°C");

  delay(1000);
}
