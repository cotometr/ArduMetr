
#include <lipo.h>
#include <mhz19b.h>
#include <SoftwareSerial.h>;
#include "DHT.h"

#define DHTPIN 2
DHT dht(DHTPIN, DHT22);
Lipo lipo(14, 21, A0, 33);

SoftwareSerial SerialBt(3, 4); // 3 - TX 4 - RX
Mhz19b co2(Serial);

void setup() {
  Serial.begin(9600);
  dht.begin();
  SerialBt.begin(9600);
}

bool is_get_arrived()
{
  String received_str;

  while (SerialBt.available( )) {

    received_str += (char)SerialBt.read();
    delay(20);
  }

  if (received_str.length() != 0)
  {
    if (received_str[0] == 'G')
      return true;
  }

  return false;
}

void loop() 
{
  if (!is_get_arrived()){
    return;
  }

  int battery_level = lipo.get_level();

  int ppm_uart = co2.get_co2_uart();

  SerialBt.println(co2.get_log_debug());
  SerialBt.println(lipo.get_log_debug());

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t) || isnan(h)) {
    SerialBt.println("Failed to read from DHT");
  }
  else {
    SerialBt.print("Humidity: ");
    SerialBt.print(h);
    SerialBt.print(" %, Temperature: ");
    SerialBt.print(t);
    SerialBt.print(" Voltage: ");
    SerialBt.print(battery_level);

    SerialBt.print("C\n");
  }
}
