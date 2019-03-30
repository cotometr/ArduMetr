#include <mhz19b.h>

#include <SoftwareSerial.h>
SoftwareSerial co2Serial(A0, A1); // A0 - к TX сенсора, A1 - к RX
unsigned long startTime = millis();
Mhz19b co2(co2Serial);

void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);
  delay(1000);
  int ret = co2.set_auto_calibrate(false);
  Serial.println(co2.get_last_log() + String("Ret from auto_calibrate = ") + String(ret));
}
 
void loop() {
  Serial.println("------------------------------");
  Serial.print("Time from start: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" s");
  int ppm_uart = co2.get_co2_uart();
  Serial.println(co2.get_last_log());
  Serial.println("GET from function: " + String(ppm_uart));

  delay(10000);
}

