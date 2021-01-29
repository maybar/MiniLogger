#ifndef LoggerProc_h
#define LoggerProc_h

#include "SdFat.h"
#include "SSD1306AsciiWire.h"

class LoggerProc
{
	public:
	LoggerProc(SSD1306AsciiWire & oled);
  void Config(void);
	void StateMachine();
	//Open(char *s_filename, char *s_header);
	//Write(char *s_data);
	//Close();
  int getState();
  int CountFiles();
  String get_sd_info();
  bool get_logging();
  void set_key(byte key);
  String get_label();
  String get_filename();
  bool write(String s_data);
  int get_length();
  int get_num_data();
	
	private:
  void start_logging();
  void stop_logging();
  
		int m_i_logger_sm;
	  SdFat32 m_c_sd; // file system object
    File32 m_c_dir;
    File32 m_c_file;
    SSD1306AsciiWire m_c_display;
    String m_s_card_size;
    byte m_i_key;
    bool m_b_logging;
    String m_s_display;
    String m_s_filename;
    File m_logfile;
    int m_i_num_data;
    int m_i_bytes;
};

#endif
