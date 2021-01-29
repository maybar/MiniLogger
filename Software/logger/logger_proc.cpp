#include "logger_proc.h"

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

LoggerProc::LoggerProc(SSD1306AsciiWire & oled)
{
  m_i_logger_sm = 0; //Idle
  m_c_display = oled;
  m_i_key = 0; //No key
  m_b_logging = false;
  m_s_display = F("LOG:OFF");
  m_i_num_data = 0;
  m_i_bytes = 0;
}

String LoggerProc::get_sd_info()
{
  return m_s_card_size;
}
void LoggerProc::Config(void)
{
  if (!m_c_sd.begin(SD_CONFIG)) {
    m_c_display.setCursor(0, 3);
    m_c_display.print(F("     SD Error!    "));
    m_c_sd.initErrorHalt(&Serial);
  }
  // Open root directory
  if (!m_c_dir.open("/")) {
    //error("dir.open failed");
    m_c_display.setCursor(0, 3);
    m_c_display.print(F("dir.open failed!"));
    m_c_sd.initErrorHalt(&Serial);
  }
  float const f_card_size = m_c_sd.card()->cardSize() * 0.000512;
  char tbs[50];
  float const used = f_card_size - m_c_sd.vol()->freeClusterCount() * m_c_sd.vol()->blocksPerCluster() * 0.000512;
  unsigned int const i_perc = (unsigned int)((unsigned int)used / (unsigned int)f_card_size);
  sprintf(tbs, "S:%d.%02dMB(%d%%)", (unsigned int)used, (unsigned int)(used * 100.0) % 100, i_perc);
  m_s_card_size = String(tbs);
}

int LoggerProc::CountFiles() {
  int k = 0;
  while (m_c_file.openNext(&m_c_dir, O_RDONLY)) {
    //file.printName(&Serial);
    if (m_c_file.isDir() == false) {
      k++;
    }
    
    //Serial.println();
    m_c_file.close();
  }
  //Serial.println(k);
  return k;
} 
void LoggerProc::set_key(byte key)
{
  m_i_key = key;
}
bool LoggerProc::get_logging()
{
  return m_b_logging;
}
String LoggerProc::get_label()
{
  return m_s_display;
}

void LoggerProc::start_logging()
{
  // Creamos el fichero de registro
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) 
  {  
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! m_c_sd.exists(filename))      // Si no existe el fichero, lo creamos
        { 
          m_logfile = m_c_sd.open(filename, FILE_WRITE); 
          break;  // leave the loop!
        }
  }
  if (! m_logfile) 
      Serial.print(F("No se pudo crear el fichero de registro"));
  else {
    m_b_logging = true;
    m_s_filename = String(filename);
    m_i_num_data = 0;
    m_i_bytes = 0;
  }
}

void LoggerProc::stop_logging()
{
  m_logfile.close();
  m_b_logging = false;
}

String LoggerProc::get_filename()
{
  return m_s_filename;
}
void LoggerProc::StateMachine()
{
  switch (m_i_logger_sm)
  {
    case 0: //IDLE
      if (m_i_key == 2)
      {
        m_i_logger_sm = 1;
        m_s_display = F("ON?");
      }
      break;
    case 1: // CONFIRM
      if (m_b_logging)
      {
        if (m_i_key == 1)
        {
          m_i_logger_sm = 0;
          this->stop_logging();
          m_s_display = F("LOG OFF");
        }
        if (m_i_key == 3)
        {
          m_i_logger_sm = 2;
          this->start_logging();
          m_s_display = F("LOGGING");
        }
      }
      else
      {
        if (m_i_key == 1)
        {
          m_i_logger_sm = 2;
          this->start_logging();
          m_s_display = F("LOGGING");
        }
        if (m_i_key == 3)
        {
          m_i_logger_sm = 0;
          this->stop_logging();
          m_s_display = F("LOG OFF");
        }
      }
      
      break;
    case 2: // LOGGING
      if (m_i_key == 2)
      {
        m_i_logger_sm = 1;
        m_s_display = F("OFF?");
      }
      break;
    default:
      break;
  }
}

int LoggerProc::getState()
{
  return m_i_logger_sm;
}
int LoggerProc::get_length()
{
  return m_i_bytes;
}
int LoggerProc::get_num_data()
{
  return m_i_num_data;
}
bool LoggerProc::write(String s_data)
{
  m_logfile.println(s_data); 
  m_i_num_data++;
  m_i_bytes = m_i_bytes + s_data.length() + 1;
  m_logfile.flush();  // Para forzar la escritura en la SD
}
