// #-----------------------
// #BOARD: MINI LOGGER
// #-----------------------
#include "SdFat.h"
#include "sdios.h"
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

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

#define MENU_ITEMS 7

// RTC
RTC_DS1307 RTC;
char s_current_time[25];
unsigned long prevTime = 0;
int prevMin = 0;

// LCD OLED
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

// =====================================
// SD card
const int8_t DISABLE_CS_PIN = 10;
#define SD_FAT_TYPE 1

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
#endif  // HAS_SDIO_CLASS


SSD1306AsciiWire oled;

// 
int i_state = 0;
bool b_redraw = true;
bool b_key_pressed = false;
int height_font;
int width_font;
int num_files = 0;
byte pinV = A1; // Para leer el voltaje
byte pinKey = A0; // Para leer el teclado
int a0_level = 0;
int last_a0_level = 0;
byte i_key = 0;
byte i_last_key = 0;
int bat = 0;  //Porcentaje
byte menu_current = 0;
byte debounce_counter = 255;  //OFF

// file system object
SdFat32 sd;
File32 dir;
File32 file;

File logfile;

// Store error strings in flash to save RAM.
//#define error(s) sd.errorHalt(&Serial, F(s))

int CountFiles() {
  int k = 0;
  while (file.openNext(&dir, O_RDONLY)) {
    //file.printName(&Serial);
    if (file.isDir() == false) {
      k++;
    }
    
    //Serial.println();
    file.close();
  }
  //Serial.println(k);
  return k;
} 

void setup() {
  Serial.begin(115200); 
  delay(200);

  // I2C
  Wire.begin(); 
  
  //LCD
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Arial_bold_14);
  oled.clear();
  oled.setCursor(0,0);
  oled.print(F(" Mini Data Logger "));
  oled.setCursor(0,3);
  oled.print(F("   -= Arduino =-"));
  delay(2000);

  
  // RTC Setup 
  if (!RTC.begin()) 
      Serial.println(F("No hay RTC."));   
  else
      Serial.println(F("RTC correcto. Iniciando captura de datos"));      
  if (! RTC.isrunning()) 
  {
    Serial.println(F("RTC is NOT running!"));
  }
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  
  // SD card setup
  //pinMode(chipSelect, OUTPUT);        // SD card pin select
  if (!sd.begin(SD_CONFIG)) {
    oled.setCursor(0,3);
    oled.print(F("     SD Error!    "));
    sd.initErrorHalt(&Serial);
  }
  // Open root directory
  if (!dir.open("/")){
    //error("dir.open failed");
    oled.setCursor(0,3);
    oled.print(F("dir.open failed!"));
    sd.initErrorHalt(&Serial);
  }
    
  num_files = CountFiles();
  
 /* // Creamos el fichero de registro
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
      error("No se pudo crear el fichero de registro");
  else
  {
    Serial.print("Registrando en: ");   
    Serial.println(filename);
  
    logfile.print("Fecha/Hora") ; 
    logfile.print(", "); 
    logfile.print("Temp") ; 
    logfile.print(", ");
    logfile.println("Humedad") ; 
  }*/
  initial_draw();
  width_font = oled.fontWidth();
  Serial.print("width_font:");Serial.print(width_font); Serial.print("\n");
}

// Returns the button number based on the analog value
byte buttonFromValue(int adValue) {
  if (adValue > 1000) {
    return 1;
  }

  else if (adValue > 800) {
    return 2;
  }

  else if (adValue > 600) {
    return 3;
  }

  else{
    return 0;
  }
}

void readVoltaje()
{
  // Lee el voltaje
  int medida = analogRead(pinV);
  float voltaje = medida * 5.0 / 1023.0;
  bat = 0;
  if (voltaje > 4.2)
  {
    bat = 100;
  }
  else if (voltaje > 3.2)
  {
    bat = 100 * (voltaje - 3.2);
  }
  else
  {
    bat = 0;
  }
  // Serial.print("Medida: ");Serial.print(medida);Serial.print('\n');
  // Serial.print("Voltaje: ");Serial.println(voltaje);Serial.print('\n');
}

// This function is called each 10ms
void getKey()
{
  a0_level = analogRead(pinKey);
  i_key = buttonFromValue(a0_level);
  if (i_key != i_last_key)
  {
    //Serial.print("Changed: ");Serial.println(i_key);
    if (i_last_key == 0)
    {
      debounce_counter = 9;   //Debounce for pressed
    }
    else
    {
      debounce_counter = 15;  //Debounce for release
    }
    i_last_key = i_key;
  }
  else
  {
    if (debounce_counter != 255)
    {
      debounce_counter--;
      //Serial.print(i_key);
    }
    if (debounce_counter == 0)
    {
      b_redraw = true;
      b_key_pressed = true;
      //Serial.print("Key: ");Serial.print(a0_level);Serial.print("-");Serial.println(i_key);
      debounce_counter = 255;
    }
  }
}
void loop() {
  //digitalWrite(LED_BUILTIN, HIGH);
  DateTime now = RTC.now();    
  int i_sec = now.second();
  int i_min = now.minute();

  // Each minute refresh voltage
  if (prevMin != i_min)
  {
    prevMin = i_min;
    readVoltaje();
    b_redraw = true;
  }
  
  
  // Logfile
  /*logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print("1");
  logfile.print(", ");
  logfile.println("2");
  logfile.flush();              // Para forzar la escritura en la SD
*/
  
  //Serial.println();
  
  //digitalWrite(LED_BUILTIN, LOW);
 

  // Refresca por defecto cada segundo
  unsigned long currentMillis = millis();  
  if (currentMillis - prevTime >= 1000) {
    prevTime = currentMillis;
  }
 
  // Process the keyboard
  if ((currentMillis % 10) == 0)
  {
    getKey();
  }
  
  // Maquina de estado
  switch (i_state)
  {
    case 0:   // IDLE
    {
      if (b_key_pressed)
      {
        if (i_key == 3) // DOWN
        {
          menu_current ++;
          if (menu_current == MENU_ITEMS)
          {
            menu_current = 0;
          }
        }
        else if (i_key == 1) // UP
        {
          if (menu_current == 0)
          {
            menu_current = MENU_ITEMS;
          }
          menu_current --;
        }
        b_key_pressed = false;
      }
      break;
    }
    case 1: //
    {
      break;
    }
  }
 
  // Redraw
  if (b_redraw == true)
  {
    //DateTime now = RTC.now();
    //printDateTime(now);
    String s = time2string(now);
    s.toCharArray(s_current_time, s.length() + 1);
    // picture loop
    draw();
    b_redraw = false;
  }
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
    Serial.print(':');    Serial.print(now.second(), DEC);
    Serial.println();
}


void error(char *str)
{
  Serial.print(F("error: "));
  Serial.println(str);
  
  while(1);
}
