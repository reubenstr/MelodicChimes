// Crude proof of concept.

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "msTimer.h"

TFT_eSPI tft = TFT_eSPI(240, 320);

#define CALIBRATION_FILE "/calibrationData"

String names[] = {"Fozy", "Piper", "Zoey", "Pickles", "Fish"};

struct Boundry
{
  int x, y, w, h;

  Boundry()
  {
  }

  Boundry(int x, int y, int w, int h)
  {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }
};

struct Button
{
  String name;
  Boundry boundry;

  uint32_t textColor = TFT_BLACK;
  uint32_t fillColor = TFT_GREEN;
  uint32_t activeColor = TFT_WHITE;
  uint32_t borderColor = TFT_DARKGREEN;

  Button(String name, Boundry boundry, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
  {
    this->name = name;
    this->boundry = boundry;
    this->textColor = textColor;
    this->fillColor = fillColor;
    this->activeColor = activeColor;
    this->borderColor = borderColor;
  }
};

struct Label
{
  String name;
  Boundry boundry;

  uint32_t textColor = TFT_BLACK;
  uint32_t fillColor = TFT_GREEN;
  uint32_t activeColor = TFT_WHITE;
  uint32_t borderColor = TFT_DARKGREEN;

  Label(String name, Boundry boundry, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
  {
    this->name = name;
    this->boundry = boundry;
    this->textColor = textColor;
    this->fillColor = fillColor;
    this->activeColor = activeColor;
    this->borderColor = borderColor;
  }
};

Button buttons[5] = {Button("PLAY", Boundry(65, 220, 100, 50), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE),
                     Button("PAUSE", Boundry(190, 220, 100, 50), TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE),
                     Button("STOP", Boundry(315, 220, 100, 50), TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE),
                     Button("PREV", Boundry(15, 150, 75, 40), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE),
                     Button("NEXT", Boundry(385, 150, 75, 40), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE)};

Label labels[3] = {Label("SD", Boundry(140, 290, 55, 18), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE),
                   Label("WIFI", Boundry(240, 290, 55, 18), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE),
                   Label("API", Boundry(340, 290, 55, 18), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE)};

void DisplayLayout()
{
  int w = tft.width() - 1;
  int h = tft.height() - 1;
  int t = 3;

  tft.fillScreen(TFT_BLACK);

  // Perimeter.
  tft.fillRect(0, 0, w, t, TFT_BLUE);
  tft.fillRect(w - t, 0, w, h, TFT_BLUE);
  tft.fillRect(0, h - t, tft.width(), t, TFT_BLUE);
  tft.fillRect(0, 0, 0 + t, h, TFT_BLUE);

  // Cross lines.
  tft.fillRect(0, 40, tft.width(), t, TFT_BLUE);
  tft.fillRect(0, 285, tft.width(), t, TFT_BLUE);

  //
  tft.setFreeFont(FF23);
  tft.setTextColor(TFT_BLUE);
  tft.drawString("Melodic Chimes", 100, 10);

  tft.setTextColor(TFT_GREEN);
  tft.setTextFont(GLCD);
  tft.setTextSize(2);
  tft.drawString("Song:", 70, 80, GFXFF);

  tft.setTextColor(TFT_GREEN);
  tft.setTextFont(GLCD);
  tft.setTextSize(3);
  tft.drawString("Fly Away Time By", 100, 130, GFXFF);

  tft.setTextSize(2);
  tft.drawString("2:33", 50, 110, GFXFF);

  tft.drawRect(110, 180, 160, 20, TFT_CYAN);

  for (Button b : buttons)
  {
    int borderThickness = 3;
    tft.fillRoundRect(b.boundry.x, b.boundry.y, b.boundry.w, b.boundry.h, 10, b.borderColor);
    tft.fillRoundRect(b.boundry.x + borderThickness, b.boundry.y + borderThickness, b.boundry.w - borderThickness * 2, b.boundry.h - borderThickness * 2, 5, b.fillColor);

    tft.setTextColor(b.textColor);
    tft.setTextSize(3);
    int textWidth = tft.textWidth(b.name);
    int textHeight = tft.fontHeight();
    tft.drawString(b.name, b.boundry.x + (b.boundry.w - textWidth) / 2, b.boundry.y + (b.boundry.h - textHeight) / 2 + borderThickness, GFXFF);
  }

  tft.drawTriangle(10, 130, 40, 160, 10, 190, TFT_CYAN);

  for (Label l : labels)
  {
    tft.fillRect(l.boundry.x, l.boundry.y, l.boundry.w, l.boundry.h, l.fillColor);
    tft.setTextColor(l.textColor);
    tft.setTextSize(2);
    int textWidth = tft.textWidth(l.name);
    int textHeight = tft.fontHeight();
    tft.drawString(l.name, l.boundry.x + (l.boundry.w - textWidth) / 2, l.boundry.y + (l.boundry.h - textHeight) / 2, GFXFF);
  }

  tft.setTextColor(TFT_BLUE);
  tft.setTextSize(2);
  tft.drawString("STATUS:", 20, 295, GFXFF);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.drawString("15:01:01", 380, 295, GFXFF);
}

void setup(void)
{
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  Serial.begin(115200);
  Serial.println("starting");

  tft.init();
  tft.setRotation(3);
  tft.fillScreen((0xFFFF));

  tft.setCursor(20, 0, 2);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(1);
  tft.println("calibration run");

  // check file system
  if (!SPIFFS.begin())
  {
    Serial.println("formating file system");

    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists
  if (SPIFFS.exists(CALIBRATION_FILE))
  {
    File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f)
    {
      if (f.readBytes((char *)calibrationData, 14) == 14)
        calDataOK = 1;

      f.close();
    }
  }

  //calDataOK = false;

  if (calDataOK)
  {
    // calibration data valid
    Serial.println("calDataOK");
    tft.setTouch(calibrationData);
  }
  else
  {
    // data not valid. recalibrate
    Serial.println("calData NOT OK");
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
  }

  DisplayLayout();
}

void loop()
{
  uint16_t x, y;
  static uint16_t color;

  static bool takeTouchReadings = true;
  static msTimer touchDebouncTimer(100);

  if (touchDebouncTimer.elapsed())
  {
    takeTouchReadings = true;
  }
  /*
  if (takeTouchReadings)
  {
    if (tft.getTouch(&x, &y))
    {


      for (int i = 0; i < 5; i++)
      {
        if (pets[i].boundry.x1 <= x && pets[i].boundry.x2 >= x)
        {
          if (pets[i].boundry.y1 <= y && pets[i].boundry.y2 >= y)
          {
            pets[i].isFed = !pets[i].isFed;
           
            takeTouchReadings = false;
            touchDebouncTimer.resetDelay();
          }
        }
      }
    }
  }
  */
}