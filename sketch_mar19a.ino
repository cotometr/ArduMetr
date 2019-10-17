#include <button.h>

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

const int transistor_pin = 9;
int transistor_state = LOW;
const int button_pin = 5;    // the number of the pushbutton pin
unsigned long debounce_delay = 500;    // the debounce time; increase if the output flickers
Button button(button_pin, debounce_delay);



const int interval = 10000;
int working_time = 7000;

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

  if (epd.Init(lut_full_update) != 0) {
    SerialBt.print("e-Paper init failed");
    return;
  }
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  delay(1000);

  if (epd.Init(lut_partial_update) != 0) {
    SerialBt.print("e-Paper init failed");
    return;
  }
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  delay(1000);
}

bool eink_print(int co2, float temp, float hum, float battery)
{
  int busy = 1;//digitalRead(7);

  if (busy == 1) {
    epd.Reset();
    paint.SetWidth(24);
    paint.SetHeight(200);
    paint.SetRotate(ROTATE_270);
    paint.Clear(COLORED);
    paint.DrawStringAt(60, 0, "Cotometr", &Font24, UNCOLORED);
    epd.ClearFrameMemory(0xFF);
    epd.SetFrameMemory(paint.GetImage(),  paint.Size(), 0, 0, paint.GetWidth(), paint.GetHeight());

    String str = "CO2: " + String(co2);
    paint.Clear(COLORED);
    paint.DrawStringAt(0, 0, str.c_str(), &Font24, UNCOLORED);
    epd.SetFrameMemory(paint.GetImage(), paint.Size(), 48, 0, paint.GetWidth(), paint.GetHeight());

    str = "Temp: " + String(temp);
    paint.Clear(COLORED);
    paint.DrawStringAt(0, 0, str.c_str(), &Font24, UNCOLORED);
    epd.SetFrameMemory(paint.GetImage(),  paint.Size(), 72, 0, paint.GetWidth(), paint.GetHeight());

    str = "Hum: " + String(hum);
    paint.Clear(COLORED);
    paint.DrawStringAt(0, 0, str.c_str(), &Font24, UNCOLORED);
    epd.SetFrameMemory(paint.GetImage(),  paint.Size(), 104, 0, paint.GetWidth(), paint.GetHeight());

    str = "V: " + String(battery);
    paint.Clear(COLORED);
    paint.DrawStringAt(0, 0, str.c_str(), &Font24, UNCOLORED);
    epd.SetFrameMemory(paint.GetImage(), paint.Size(), 136, 0, paint.GetWidth(), paint.GetHeight());

    int time = millis() / 1000;
    str = "Time: " + String(time);
    paint.Clear(COLORED);
    paint.DrawStringAt(0, 0, str.c_str(), &Font24, UNCOLORED);

    epd.SetFrameMemory(paint.GetImage(), paint.Size(), 176, 0, paint.GetWidth(), paint.GetHeight());

    epd.DisplayFrame();
    delay(600);
    epd.Sleep();
    return true;
  } else
  {
    SerialBt.println("Print busy");
  }
  return false;
}

void setup_on_button()
{
  Serial.begin(9600);
  dht.begin();
  SerialBt.begin(9600);
  eink_init();

  SerialBt.println("Start! \n");
}
void button_handler(void*)
{
  transistor_state = !transistor_state;
  digitalWrite(transistor_pin, transistor_state);
  static int inited;
  if (transistor_state == HIGH && !inited)
  {
    inited = 1;
    setup_on_button();
  }
}

void setup() {
  //button.set_handler(button_handler, NULL);

  pinMode(transistor_pin, OUTPUT);
  button_handler(NULL);
  digitalWrite(transistor_pin, transistor_state);


  commands[CMD_GET] = "Get\r\n";
  commands[CMD_CALIBRATE_ON] = "Calibrate1\r\n";
  commands[CMD_CALIBRATE_OFF] = "Calibrate0\r\n";
  commands[CMD_CALIBRATE_ZERO] = "CalibrateZero\r\n";
}

String received_str()
{
  String received_str;

  while (SerialBt.available()) {
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
  //SerialBt.println("Loop: work time = " + String(working_time));
  //button.poll();
  digitalWrite(transistor_pin, transistor_state);
  if (transistor_state == LOW)
    return;

  delay(1);
  working_time += 1;
  //SerialBt.println("Loop: after delat time = " + String(working_time));
  String str = received_str();
  if (working_time >= interval && str.length() == 0)
    str = commands[CMD_GET];

  if (str.length() == 0)
    return;

  working_time = 0;
  if (str_cmp(str, commands[CMD_GET])) {
    int ppm_uart;
    for (int i = 0; i < 3; i++) {
      ppm_uart = co2.get_co2_uart();

      delay(100);
    }


    SerialBt.print(co2.get_log_debug());
    SerialBt.print(lipo.get_log_debug());

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float battery = lipo.get_voltage();
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
      SerialBt.print(battery);
      bool is_printed = eink_print(ppm_uart, t, h, battery);
      //SerialBt.print("\nPrinting = " + String(is_printed));
      SerialBt.print("\n");
    }
  }
  else if (str_cmp(str, commands[CMD_CALIBRATE_ON])) {
    co2.set_auto_calibrate(true);
    SerialBt.println(co2.get_log_debug());
  }
  else if (str_cmp(str, commands[CMD_CALIBRATE_OFF])) {
    co2.set_auto_calibrate(false);
    SerialBt.println(co2.get_log_debug());
  }
  else if (str_cmp(str, commands[CMD_CALIBRATE_ZERO])) {
    co2.set_zero_point_calibration();
    SerialBt.println(co2.get_log_debug());
  }
  else {
    SerialBt.println("\nHelp");
    SerialBt.println("Received: |" + str + "|");

    String cmds;
    for (int i = 0; i < CMD_NUM; i++)
      cmds += commands[i];

    SerialBt.println("Please use one of the following commands:");
    SerialBt.print(cmds);
    SerialBt.println("-");
  }
}
