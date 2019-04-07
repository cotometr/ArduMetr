
#include <lipo.h>
#include <mhz19b.h>
#include <SoftwareSerial.h>;
#include "DHT.h"

#define DHTPIN 2
DHT dht(DHTPIN, DHT22);
Lipo lipo(14, 21, A0, 33);

SoftwareSerial SerialBt(3, 4); // 3 - TX 4 - RX
Mhz19b co2(Serial);

enum Cmd
{
  CMD_GET,
  CMD_CALIBRATE_ON,
  CMD_CALIBRATE_OFF,
  CMD_CALIBRATE_ZERO,
  CMD_NUM
};

String commands[CMD_NUM];


void setup() {
  Serial.begin(9600);
  dht.begin();
  SerialBt.begin(9600);

  commands[CMD_GET] = "Get\r\n";
  commands[CMD_CALIBRATE_ON] = "Calibrate1\r\n";
  commands[CMD_CALIBRATE_OFF] = "Calibrate0\r\n";
  commands[CMD_CALIBRATE_ZERO] = "CalibrateZero\r\n";
}

String received_str()
{
  String received_str;

  while (SerialBt.available( )) {

    received_str += (char)SerialBt.read();
    delay(20);
  }

  return received_str;
}

bool str_cmp(String in_l, String in_r)
{
  if (in_l.length() != in_r.length()) {
    return false;
  }

  for (int i = 0; i < in_l.length(); i++) {
    if (in_l[i] != in_r[i])
      return false;
  }

  return true;
}


void loop()
{
  delay(50);
  String str = received_str();
  if (str.length() == 0)
    return;

  if (str_cmp(str, commands[CMD_GET])){
    int battery_level = lipo.get_level();

    int ppm_uart = co2.get_co2_uart();

    SerialBt.print(co2.get_log_debug());
    SerialBt.print(lipo.get_log_debug());

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
      SerialBt.print("\n");
    }
  }
  else if (str_cmp(str, commands[CMD_CALIBRATE_ON])){
    co2.set_auto_calibrate(true);
    SerialBt.println(co2.get_log_debug());
  }
  else if (str_cmp(str, commands[CMD_CALIBRATE_OFF])){
    co2.set_auto_calibrate(false);
    SerialBt.println(co2.get_log_debug());
  }
  else if (str_cmp(str, commands[CMD_CALIBRATE_ZERO])){
    co2.set_zero_point_calibration();
    SerialBt.println(co2.get_log_debug());
  }
  else {
    SerialBt.println("\n-----------Help-----------");
    SerialBt.println("Received: |" + str +"|");

    String cmds;
    for (int i = 0; i < CMD_NUM; i++)
      cmds += commands[i];

    SerialBt.println("Please use one of the following commands:");
    SerialBt.print(cmds);
    SerialBt.println("--------------------------");
  }
}

