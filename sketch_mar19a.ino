#include <mhz19b.h>
#include <SoftwareSerial.h>
SoftwareSerial co2Serial(A0, A1); // A0 - к TX сенсора, A1 - к RX
unsigned long startTime = millis();
Mhz19b co2(co2Serial);

void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);
  pinMode(9, INPUT);
}
 
void loop() {
  Serial.println("------------------------------");
  Serial.print("Time from start: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" s");
  int ppm_uart = co2.getCO2uart();
  Serial.println(co2.get_last_log());
  
  delay(10000);
}

