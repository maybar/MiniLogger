
const char *menu_strings[6] = { "START", "YES", "STOP", "4", "5", "6" };
const char *label_strings[3] = { "STOP", "Num.Files:", "|"};

String time2csv() {
  DateTime now = RTC.now();
  String d = String(now.unixtime())// number of seconds elapsed since Jan. 1st 1970.
             + ", " + now.year() + ", " + now.month() + ", " + now.day() + ", " + now.hour() + ", " + now.minute() + ", " + now.second();
  return d;
}

String time2string(DateTime now) {
  String d = String("")
             + now.month() + "/" + now.day() + " " + now.hour() + ":" + now.minute();
  return d;
}

// FUNCTION
void initial_draw(void) {
  oled.setFont(lcd5x7);
  // 8 lines
  // Line height in rows.
  height_font = oled.fontRows();
  
  oled.clear();
}

// 
void clearField(int row, int x, int w)
{
  int r = row*height_font;
  oled.clear(x, x + w, r, r+ height_font -1);
}

void draw(void) {
  //Serial.println("Draw---"),
  // print the time =======================
  //int w = oled.strWidth(s_current_time);
  //Serial.println(w);
  //clearField(0, 0, w);
  oled.clearField(0, 0, 14);  // 14 caracteres fijos
  oled.print(s_current_time);
  //==========================================

  // STATE ====================
  oled.clearField(85, 0, 20);  // 14 caracteres fijos
  oled.print(logger.get_label());

  // FILE NAME ==================
  oled.clearField(0, 1, 100); 
  oled.println(logger.get_filename()); 
  oled.clear(0, 128, 2, 6);
  if (logger.get_logging())
  {
    String cad1 = F("Num: "); 
    cad1.concat(logger.get_num_data());
    oled.println(cad1);
    String cad2 = F("Size: "); 
    float len = logger.get_length();
    String units = F("B");
    if (len > 1000000)
    {
      len = len / 1000000.0;
      units = F("MB");
    }
    else if (len > 1000)
    {
      len = len / 1000.0;
      units = F("KB");
    }
      
    cad2.concat(len);
    cad2.concat(units);
    oled.println(cad2);
  }

  // Bat ============================
  oled.clearField(0, 7, 29);  // COL=0, FIL=7, N=30
  oled.print(s_bat_info);
  // Size SD card =================
  oled.clearField(35, 7, 70);  // COL=40, FIL=7, N=50
  String s_sd_info = logger.get_sd_info();
  oled.print(s_sd_info);
}
