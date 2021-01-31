// #-----------------------
// #BOARD: MINI LOGGER
// Placa: Arduino Pro or Pro Mini
// Procesasdor: ATMega328P (5V 16MHz)
// #-----------------------
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "logger_proc.h"


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

#define MENU_ITEMS 6

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

SSD1306AsciiWire oled;

// 
String s_rx_data;
bool b_redraw = true;
int height_font;
int width_font;
byte pinV = A1; // Para leer el voltaje
byte pinKey = A0; // Para leer el teclado
int a0_level = 0;
int last_a0_level = 0;
byte i_last_key = 0;
String s_bat_info = "0%";  //Porcentaje
String s_sd_info = "0/0MB";
byte menu_current = 0;
byte debounce_counter = 255;  //OFF

LoggerProc logger(oled);

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
      Serial.println(F("RTC correcto"));      
  if (! RTC.isrunning()) 
  {
    Serial.println(F("RTC is NOT running!"));
  }
  //RTC.adjust(DateTime(__DATE__, __TIME__));

  //pinMode(chipSelect, OUTPUT);        // SD card pin select
  
  // SD card setup
  logger.Config(DISABLE_CS_PIN);  
 
  initial_draw();
  width_font = oled.fontWidth();
  //Serial.print("width_font:");Serial.print(width_font); Serial.print("\n");
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

// FUNCTION: Read the level of the battery
void readVoltaje()
{
  // Lee el voltaje
  int medida = analogRead(pinV);
  float voltaje = medida * 5.0 / 1023.0;
  int bat = 0;
  if (voltaje > 4.1)
  {
    bat = 100;
  }
  else if (voltaje > 3.2)
  {
    bat = 111.1 * (voltaje - 3.2);  // m=100/0.9
  }
  else
  {
    bat = 0;
  }
  s_bat_info  = "B:"+ String(bat) + "%";
}

// This function is called each 10ms
bool getKey(byte &i_key)
{
  bool b_key_pressed = false;
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
      b_key_pressed = true;
      //Serial.print("Key: ");Serial.print(a0_level);Serial.print("-");Serial.println(i_key);
      debounce_counter = 255;
    }
  }
  return b_key_pressed;
}

// MAIN LOOP
void loop() {
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
 

  // Refresca por defecto cada segundo
  unsigned long currentMillis = millis();  
  if (currentMillis - prevTime >= 1000) {
    prevTime = currentMillis;
  }
 
  // Process the keyboard
  if ((currentMillis % 10) == 0)
  {
    byte i_key;
    bool const b_key_pressed = getKey(i_key);
    // When key is pressed, send the cmd to the logger
    if (b_key_pressed)
    {
      logger.set_key(i_key);
      b_redraw = true;
    }
  }

  // If logger is recording
  if (logger.get_logging())
  {
    // If some data in the reception buffer
    while (Serial.available() > 0) 
    {
      char const ch= Serial.read() ;
      // When ENTER is received, the data is stored
      if (ch == '\n')
      {
        logger.write(s_rx_data);
        Serial.println(s_rx_data);
        s_rx_data = "";
        b_redraw = true;
      }
      else
      {
        // Set the time stamp
        if(s_rx_data.length() == 0)
        {
          s_rx_data = String(now.unixtime()); // seconds since 1/1/1970
          s_rx_data.concat(',');
        }
        s_rx_data.concat(ch);
      }
    }//while
  }// if logging

  // Move the Logger state machine
  logger.StateMachine();
 
  // Redraw
  if (b_redraw)
  {
    String s = time2string(now);
    s.toCharArray(s_current_time, s.length() + 1);
    // picture loop
    draw();
    b_redraw = false;
  }
}

// Show error and stop de application
void error(char *str)
{
  Serial.print(F("error: "));
  Serial.println(str);
  
  while(1);
}
