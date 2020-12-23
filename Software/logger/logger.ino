// #-----------------------
// #BOARD: MINI LOGGER
// #-----------------------
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

// #-----------------------
// # RTC, i2c bus:
// # A4: SDA   -> SDA
// # A5: SCL   -> SCL 
// # SD Card, SPI bus:
// # D13: SCK   -> SCK
// # D12: MISO  -> MISO
// # D11: MOSI  -> MOSI
// # D10: SS    -> CS
// #-----------------------

RTC_DS1307 RTC;
#define countof(a) (sizeof(a) / sizeof(a[0]))

const int chipSelect = 10;  // SD card pin select

File logfile;


void setup() {
  Serial.begin(115200); 
  delay(200);

  // RTC Setup
  Wire.begin();  
  if (!RTC.begin()) 
      logfile.println("No hay RTC.");   
  else
      Serial.println("RTC correcto. Iniciando captura de datos");      
  if (! RTC.isrunning()) 
  {
    Serial.println("RTC is NOT running!");
  }
  RTC.adjust(DateTime(__DATE__, __TIME__));

  // SD card setup
  pinMode(chipSelect, OUTPUT);        // SD card pin select
  if (!SD.begin(chipSelect))
       Serial.println("No hay tarjeta SD.");
  else
       Serial.println("Tarjeta SD inicializada.");

  // Creamos el fichero de registro
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) 
     {  
        filename[6] = i/10 + '0';
        filename[7] = i%10 + '0';
        if (! SD.exists(filename))      // Si no existe el fichero, lo creamos
            { 
              logfile = SD.open(filename, FILE_WRITE); 
              break;  // leave the loop!
            }
     }
  if (! logfile) 
      error("No sepudo crear el fichero de registro");
    
  Serial.print("Registrando en: ");   Serial.println(filename);

  logfile.print("Fecha/Hora") ; 
  logfile.print(", "); 
  logfile.print("Temp") ; 
  logfile.print(", ");
  logfile.println("Humedad") ; 


}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  DateTime now = RTC.now();   
  printDateTime(now); 
  // Logfile
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print("1");
  logfile.print(", ");
  logfile.println("2");
  logfile.flush();              // Para forzar la escritura en la SD

  
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

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  while(1);
}
