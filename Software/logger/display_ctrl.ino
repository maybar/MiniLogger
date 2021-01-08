
const char *menu_strings[MENU_ITEMS] = { "START", "FILE", "CFG", "4", "5", "6", "7" };
const char *label_strings[4] = { "STOP", "Num.Files:", "|", "B:100%"};

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

/*void draw_menu_0(void) {
  uint8_t i, h;
  u8g_uint_t w, d;
  u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();
  h = u8g.getFontAscent()-u8g.getFontDescent();
  w = u8g.getWidth();
  for( i = 0; i < MENU_ITEMS; i++ ) {        // draw all menu items
    d = (w-u8g.getStrWidth(menu_strings[i]))/2;
    u8g.setDefaultForegroundColor();
    if ( i == menu_current ) {               // current selected menu item
      u8g.drawBox(0, i*h+1, w, h);     // draw cursor bar
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i*h, menu_strings[i]);
  }
}*/
void initial_draw(void) {
  oled.setFont(lcd5x7);
  // 8 lines
  // Line height in rows.
  height_font = oled.fontRows();
  
  oled.clear();
  
  //MENU
  for (int x=0; x != MENU_ITEMS; ++x)
  {
    oled.setRow(x + 1);
    oled.setCol(90);
    oled.println(label_strings[2]);
  }
  for (int x=0; x != MENU_ITEMS; ++x)
  {
    oled.setRow(x + 1);
    oled.setCol(98);
    if (x == 0)
    {
      oled.setInvertMode(1);
    }
    else
    {
      oled.setInvertMode(0);
    }
    oled.println(menu_strings[x]);
  }

  //List of files
  // Open root directory
  /*if (!dir.open("/")){
    error("dir.open failed");
  }
  // Open next file in root.
  // Warning, openNext starts at the current position of dir so a
  // rewind may be necessary in your application.
  num_files = 0;
  while (file.openNext(&dir, O_RDONLY)) {
    file.printFileSize(&Serial);
    Serial.write(' ');
    file.printModifyDateTime(&Serial);
    Serial.write(' ');
    //file.printName(&Serial);
    if (file.isDir()) {
      // Indicate a directory.
      Serial.write('/');
    }
    else
    {
      if (num_files < 6)
      {
        oled.setRow(num_files + 1); //
        char f_name[50];
        file.getName(&f_name[0], 50);
        oled.println(&f_name[0]);
      }
      num_files++;
    }
    Serial.println();
    file.close();
  }
  if (dir.getError()) {
    Serial.println("openNext failed");
  } else {
    Serial.println("Done!");
    Serial.println(num_files);
  }*/
  // Bat
  oled.setRow(0); 
  oled.setCol(90);  //px
  oled.println(label_strings[3]);
  
  // Num files
  oled.setRow(7); //line 7
  oled.println(label_strings[1]);
  oled.setCol(65);  //px
  oled.print(num_files);
}

void clearField(int row, int x, int w)
{
  int r = row*height_font;
  oled.clear(x, x + w, r, r+ height_font -1);
}

void draw(void) {
  // print the time
  //int w = oled.strWidth(s_current_time);
  //Serial.println(w);
  //clearField(0, 0, w);
  oled.clearField(0, 0, 14);  // 14 caracteres fijos
  oled.print(s_current_time);
  //
  //oled.setCursor(100,0);
  //oled.print("|");

  
  if (i_state == 0)
  {
    for (int x=0; x != MENU_ITEMS; ++x)
    {
      oled.setRow(x + 1);
      oled.setCol(98);
      if (x == menu_current)
      {
        oled.setInvertMode(1);
      }
      else
      {
        oled.setInvertMode(0);
      }
      oled.print(menu_strings[x]);
    }
  }else if (i_state == 1)
  {
    
  }
  else
  {
  }
  oled.setInvertMode(0);
  
  // Keyboard
  //oled.clearField(25, 5, 4);  // 
  //oled.println(i_key);
  // Bat
  oled.clearField(101, 0, 3);  // 
  oled.print(bat);
  
}
