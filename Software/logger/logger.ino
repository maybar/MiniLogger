
#include <Wire.h>
#include "RTClib.h"

// #-----------------------
// #PLACA: MINI LOGGER
// # i2c para el reloj
// # A4: SDA   
// # A5: SCL
// #-----------------------

RTC_DS1307 RTC;
#define countof(a) (sizeof(a) / sizeof(a[0]))

void setup() {
  Serial.begin(115200); 
  delay(200);

  // RTC DS3231 Setup
  RTC.begin();    
  if (! RTC.isrunning()) 
  {
    Serial.println("RTC is NOT running!");
  }
  RTC.adjust(DateTime(__DATE__, __TIME__));
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  DateTime now = RTC.now();   
  printDateTime(now); 
  
  Serial.println();
  digitalWrite(LED_BUILTIN, LOW);
  delay(3000);

}

void printDateTime(const DateTime& now)
{
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

}
