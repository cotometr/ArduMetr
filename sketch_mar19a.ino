#include <SoftwareSerial.h>;
#include "DHT.h"

#define DHTPIN 2 // номер пина, к которому подсоединен датчик
#define DHTTYPE DHT22


SoftwareSerial mySerial(A0, A1); // A0 - к TX сенсора, A1 - к RX
DHT dht(DHTPIN, DHT22);

SoftwareSerial SerialBt(2, 3);

byte cmd[9] = {
  0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
unsigned char response[9];

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
  SerialBt.begin(9600);
}

int state = 1;

void loop() 
{
  mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes((char*)response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error new: " + String(crc) + " / "+ String(response[8]));
  } 
  else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    Serial.println(ppm);
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  }
  else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }
  delay(10000);
}



