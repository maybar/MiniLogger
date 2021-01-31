#include "logger_proc.h"

#define SD_FAT_TYPE 1

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN



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
  return m_s_card_info;
}

int LoggerProc::get_used()
{
  int used = 0;
  while (true) {
    File entry =  m_c_dir.openNextFile();
    if (! entry) {
      // no more files
      //Serial.println("no more files");
      m_c_dir.rewindDirectory();
      break;
    }
    Serial.print(entry.name());
    if (entry.isDirectory() == false) {
      // files have sizes, directories do not
      //Serial.print("\t\t");
      //Serial.println(entry.size(), DEC);
      used += entry.size();
    }
    entry.close();
  }
  return used;
}
void LoggerProc::calculate_sd_info()
{
  char tbs[50];
  float used = get_used();  // bytes
  float const used_mb = (float)used/1048576.0;  //MB
  Serial.println(used);
  unsigned int const i_perc = (unsigned int)((unsigned int)used_mb / (unsigned int)m_f_card_size);
  if (used < 1024.0)
  {
    sprintf(tbs, "S:%dB(%d%%)", (unsigned int)used, i_perc);
  }
  else if (used < 1048576.0)
  {
    used = (float)used / 1024.0;
    sprintf(tbs, "S:%d.%02dKB(%d%%)", (unsigned int)used, (unsigned int)(used * 100.0) % 100, i_perc);
  }
  else
  {
    sprintf(tbs, "S:%d.%02dMB(%d%%)", (unsigned int)used_mb, (unsigned int)(used_mb * 100.0) % 100, i_perc);
  }
  m_s_card_info = String(tbs);
  Serial.println(m_s_card_info);
}
void LoggerProc::Config(int chipSelect)
{
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while (1);
  }
  m_c_dir = SD.open("/");
  
  if (!m_c_sd.init(SPI_HALF_SPEED, chipSelect)) {
    m_c_display.setCursor(0, 3);
    m_c_display.print(F("     SD Error!    "));
    while (1);
  }
  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!m_c_volume.init(m_c_sd)) {
    Serial.println(F("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card"));
    m_c_display.setCursor(0, 3);
    m_c_display.print(F("Volume.init failed!"));
    while (1);
  }
  uint32_t volumesize = m_c_volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= m_c_volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  m_f_card_size = (float)volumesize/1024.0;                           // MB
  Serial.println(m_f_card_size);
  
  this->calculate_sd_info();
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
    if (! SD.exists(filename))      // Si no existe el fichero, lo creamos
        { 
          m_logfile = SD.open(filename, FILE_WRITE); 
          break;  // leave the loop!
        }
  }
  if (! m_logfile) 
  {
      Serial.print(F("No se pudo crear el fichero de registro"));
      m_i_logger_sm = 0;
      m_s_display = F("ERROR");

      m_c_display.setCursor(0, 3);
      m_c_display.print(F("Max 100 files!"));
    
      /*File errorfile = SD.open("error.txt", FILE_WRITE);
      errorfile.println(F("It couldn't be created the record file!")); 
      errorfile.println(F("Limit reached. Max 100 files.")); 
      errorfile.flush();
      errorfile.close();*/
  }
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
  this->calculate_sd_info();
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
