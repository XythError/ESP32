// ===== gui.cpp ============================
#include "gui.h"

static TFT_eSPI tft = TFT_eSPI();
static XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
static bool touchIRQ = false;

static uint16_t bgCol  = TFT_BLACK;
static uint16_t fgCol  = TFT_WHITE;
static uint16_t warnCol= TFT_RED;

static void drawHeader(){
  tft.fillRect(0,0,480,24,TFT_NAVY);
  tft.setTextColor(TFT_WHITE,TFT_NAVY);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("HiveScale – Dashboard",240,12,2);
}

static void drawHiveTile(uint8_t idx,const HiveData &d){
  const int col = idx%3;  // 3 columns
  const int row = idx/3;  // 0..2 rows (fits 9)
  const int x = col*160;
  const int y = 24 + row*99; // header 24px, each tile 99px
  tft.fillRect(x,y,160,99,bgCol);
  tft.drawRect(x,y,160,99,TFT_DARKGREY);

  // Title bar
  tft.setTextColor(TFT_CYAN,bgCol);
  tft.setTextDatum(TL_DATUM);
  char buf[10];
  sprintf(buf,"Hive %u",d.pl.id);
  tft.drawString(buf,x+2,y+2,2);

  if(d.lastUpdate==0){
      tft.setTextColor(warnCol,bgCol);
      tft.drawString("no data",x+50,y+40,2);
      return;
  }

  tft.setTextColor(fgCol,bgCol);
  sprintf(buf,"%0.1f kg",d.pl.weight_g/1000.0f);
  tft.drawString(buf,x+10,y+30,4);

  sprintf(buf,"%0.1f°C",d.pl.temp_c/100.0f);
  tft.drawString(buf,x+10,y+60,2);

  // Battery icon
  uint16_t battPct = constrain(map(d.pl.batt_mv,3300,4200,0,100),0,100);
  tft.fillRoundRect(x+120,y+65,30,15,3, TFT_DARKGREEN);
  tft.fillRoundRect(x+121,y+66, (battPct*28)/100,13,2, (battPct<20?warnCol:TFT_GREEN));
}

bool GUI::begin(){
  tft.begin();
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(bgCol);
  ts.begin();
  ts.setRotation(TFT_ROTATION);
  drawHeader();
  return true;
}

void GUI::update(const HiveData *dataArr){
  for(uint8_t i=0;i<HIVE_COUNT;i++) drawHiveTile(i,dataArr[i]);
}

void GUI::loopTouch(HiveData *dataArr){
  if(!ts.touched()) return;
  TS_Point p = ts.getPoint();
  // Map p.x/p.y to display coordinates depending on rotation
  uint16_t x = p.x;
  uint16_t y = p.y;
  if(TFT_ROTATION==1){
    uint16_t tmp = x;
    x = y;
    y = 480 - tmp;
  }
  // Determine tile touched
  if(y<24) return; // header
  uint8_t row = (y-24)/99;
  uint8_t col = x/160;
  uint8_t idx = row*3 + col;
  if(idx>=HIVE_COUNT) return;
  // Show detail popup
  tft.fillRoundRect(60,60,360,200,8,TFT_DARKGREY);
  tft.drawRoundRect(60,60,360,200,8,TFT_WHITE);
  tft.setTextColor(TFT_YELLOW,TFT_DARKGREY);
  tft.setTextDatum(TC_DATUM);
  char buf[64];
  sprintf(buf,"Details Hive %u",dataArr[idx].pl.id);
  tft.drawString(buf,240,80,2);
  tft.setTextDatum(TL_DATUM);
  const HivePayload &p=dataArr[idx].pl;
  sprintf(buf,"Gewicht : %0.2f kg",p.weight_g/1000.0f); tft.drawString(buf,80,110,2);
  sprintf(buf,"Temp     : %0.2f °C",p.temp_c/100.0f);   tft.drawString(buf,80,130,2);
  sprintf(buf,"Feuchte  : %0.2f %%",p.hum_pct/100.0f);   tft.drawString(buf,80,150,2);
  sprintf(buf,"Luftdr.  : %0.1f hPa",p.press_hpa/10.0f); tft.drawString(buf,80,170,2);
  sprintf(buf,"Batterie : %u mV",p.batt_mv);            tft.drawString(buf,80,190,2);
  delay(1500);
  drawHeader();
  update(dataArr);
}
// ===== Ende gui.cpp =======================