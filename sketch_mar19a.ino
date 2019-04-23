#include <SPI.h>
#include "e-ink.h"
#include "e-ink-display.h"
#include "imagedata.h"
#include <epdif.h>
#include <e-ink.h>
#include <fonts.h>
#include <imagedata.h>

#include <lipo.h>
#include <mhz19b.h>
#include <SoftwareSerial.h>
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

#define COLORED     0
#define UNCOLORED   1

Epd epd;
uint8_t picture[600];
Paint paint(picture, 16, 160);

void eink_init()
{
  SPI.begin();

  if (epd.Init() != 0) {
    return;
  }

  epd.ClearFrame();
}

bool eink_print(const String& str)
{
  int busy = digitalRead(7);

  if (busy == 1) {
    paint.SetRotate(ROTATE_90);
    paint.Clear(COLORED);
    paint.DrawStringAt(0, 0, str.c_str(), &Font12, UNCOLORED);
    epd.SetPartialWindowRed(paint.GetImage(), 40, 28, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
    return true;
  }
  return false;
}

void setup() {
  
  Serial.begin(9600);
  dht.begin();
  SerialBt.begin(9600);
  delay(10000);
  SerialBt.println("Start! \n");
  SerialBt.println( "DC pin = " + String(DC_PIN) + ", CS = " + String(CS_PIN) + ", RS = " + String(RST_PIN) + ", Busy = " + String(BUSY_PIN));
  if (epd.Init() != 0) {
    Serial.println("Error!");
    return;
  }

  epd.ClearFrame();
  
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
    int battery_level, ppm_uart;
    for (int i = 0; i < 3; i++) {
      battery_level = lipo.get_level();
      ppm_uart = co2.get_co2_uart();

      delay(100);
    }


    SerialBt.print(co2.get_log_debug());
    SerialBt.print(lipo.get_log_debug());

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(t) || isnan(h)) {
      SerialBt.println("Failed to read from DHT");
    }
    else {
      SerialBt.print("CO2: ");
      SerialBt.print(ppm_uart);
      SerialBt.print("Humidity: ");
      SerialBt.print(h);
      SerialBt.print(" %, Temperature: ");
      SerialBt.print(t);
      SerialBt.print(" Voltage: ");
      SerialBt.print(battery_level);
      bool is_printed = eink_print(String("CO2:" + String(ppm_uart) + " t:" + String(t) + " h:" + String(h)));
      //SerialBt.print("\nPrinting = " + String(is_printed));
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
    SerialBt.println("\nHelp");
    SerialBt.println("Received: |" + str +"|");

    String cmds;
    for (int i = 0; i < CMD_NUM; i++)
      cmds += commands[i];

    SerialBt.println("Please use one of the following commands:");
    SerialBt.print(cmds);
    SerialBt.println("-");
  }
}
