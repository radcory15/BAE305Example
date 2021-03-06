#include <OneWire.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)
const int button1 = 2; //button 1 on pin 2
const int button2 = 3; //button 2 on pin 3
int buttonState1 = 0;
int buttonState2 = 0;
int setTemp = 72; //Default temperature setting
int relay = 9;
void setup(void) {
  Serial.begin(9600);
  delay(1);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Initialize Display
  pinMode(button1, INPUT);
  pinMode(relay, OUTPUT);
}

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  if ( !ds.search(addr)) {

    ds.reset_search();
    delay(1);
    return;
  }


  for ( i = 0; i < 8; i++) {
    Serial.write(' ');

  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  Serial.println();

  // the first ROM byte indicates which chip
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
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  delay(1);

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(" ");
  }
  Serial.println();
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;

  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Temp");
  display.println(fahrenheit);
  display.println("Set Temp");
  buttonState1 = digitalRead(button1);
  buttonState2 = digitalRead(button2);
  if (buttonState1 == HIGH) {
    setTemp = setTemp + 1;
  }
  else {
  }
  if (buttonState2 == HIGH) {
    setTemp = setTemp - 1;
  }
  else {
  }

  display.println(setTemp);
  display.display();
  if (fahrenheit >= setTemp) {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }

  delay(1);

}
