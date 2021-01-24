// Crude proof of concept.

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "msTimer.h"

#include <list>

// TFT configuration contained in User_Setup.h in the local library.
TFT_eSPI tft = TFT_eSPI();

#define CALIBRATION_FILE "/calibrationData"

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

enum ButtonId
{
  Home,
  Calibration,
  Configuration,
  Default,
  Previous,
  Next,
  Play,
  Pause,
  Stop,
  Hourly,
  StartHour,
  EndHour,
  TimeZone,
  Song,
  Startup
};

struct Button
{
  String text;
  ButtonId buttonId;
  Boundry boundry;

  uint32_t textColor = TFT_BLACK;
  uint32_t fillColor = TFT_GREEN;
  uint32_t activeColor = TFT_WHITE;
  uint32_t borderColor = TFT_DARKGREEN;

  Button(ButtonId buttonId, String text, Boundry boundry, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
  {
    this->text = text;
    this->buttonId = buttonId;
    this->boundry = boundry;
    this->textColor = textColor;
    this->fillColor = fillColor;
    this->activeColor = activeColor;
    this->borderColor = borderColor;
  }
};

struct Label
{
  String text;
  uint32_t textColor = TFT_BLACK;
  int x;
  int y;

  Label(String text, int x, int y, uint32_t textColor)
  {
    this->text = text;
    this->x = x;
    this->y = y;
    this->textColor = textColor;
  }
};

const int buttonMainW = 110;
const int buttonMainH = 55;

Button buttonsMain[] = {Button(ButtonId::Home, "HOME", Boundry(30, 10, buttonMainW, 35), TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE),
                        Button(ButtonId::Configuration, "CONFIG.", Boundry(180, 10, buttonMainW, 35), TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE),
                        Button(ButtonId::Calibration, "CALIB.", Boundry(330, 10, buttonMainW, 35), TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE)};

Button buttonsHome[] = {Button(ButtonId::Play, "PLAY", Boundry(30, 220, buttonMainW, buttonMainH), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE),
                        Button(ButtonId::Pause, "PAUSE", Boundry(180, 220, buttonMainW, buttonMainH), TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE),
                        Button(ButtonId::Stop, "STOP", Boundry(330, 220, buttonMainW, buttonMainH), TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE),
                        Button(ButtonId::Previous, "PREV.", Boundry(30, 150, buttonMainW, buttonMainH), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE),
                        Button(ButtonId::Next, "NEXT", Boundry(330, 150, buttonMainW, buttonMainH), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE)};

Button buttonsConfiguration[] = {Button(ButtonId::Hourly, "Yes", Boundry(20, 90, buttonMainW, 35), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE),
                                 Button(ButtonId::StartHour, "900", Boundry(180, 90, buttonMainW, 35), TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE),
                                 Button(ButtonId::EndHour, "1300", Boundry(340, 90, buttonMainW, 35), TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE),
                                 Button(ButtonId::TimeZone, "EST -4 GMT", Boundry(20, 160, buttonMainW, 35), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE),
                                 Button(ButtonId::Startup, "Yes", Boundry(180, 160, buttonMainW, 35), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE),
                                 Button(ButtonId::Song, "Random", Boundry(20, 230, 240, 35), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE)};

Label labelsConfiguration[] = {Label("Hourly", 20, 70, TFT_WHITE),
                               Label("Start Hour", 180, 70, TFT_WHITE),
                               Label("End Hour", 340, 70, TFT_WHITE),
                               Label("Time Zone", 20, 140, TFT_WHITE),
                               Label("On Startup", 180, 140, TFT_WHITE),
                               Label("Song", 20, 210, TFT_WHITE)};

struct StatusItem
{
  String text;
  bool status;

  StatusItem(String text, bool status)
  {
    this->text = text;
    this->status = status;
  }
};

enum PageId
{
  HomePage,
  CalibrationPage,
  ConfigurationPage
};

PageId pageId = PageId::HomePage;

// Touch screen variables.
bool takeTouchReadings = true;
unsigned long touchDebounceMillis = millis();

// Configuration variables.
struct Configuration
{
  bool hourly = true;
  int startHour = 900;
  int endHour = 2200;
  signed int timeZone = -4;
  bool startup = true;
  String songName = "Default Song";
} configuration;



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsButtonPressed(Button b, int x, int y)
{
  if (x >= b.boundry.x && x <= (b.boundry.x + b.boundry.w))
  {
    if (y >= b.boundry.y && y <= (b.boundry.y + b.boundry.h))
    {
      takeTouchReadings = false;
      touchDebounceMillis = millis();
      return true;
    }
  }

  return false;
}

void DisplayButton(Button b)
{
  tft.setFreeFont(FF22);
  tft.setTextColor(b.textColor);
  tft.setTextSize(1);
  int borderThickness = 3;
  tft.fillRoundRect(b.boundry.x, b.boundry.y, b.boundry.w, b.boundry.h, 6, b.borderColor);
  tft.fillRoundRect(b.boundry.x + borderThickness, b.boundry.y + borderThickness, b.boundry.w - borderThickness * 2, b.boundry.h - borderThickness * 2, 5, b.fillColor);

  int textWidth = tft.textWidth(b.text);
  int textHeight = tft.fontHeight();
  tft.drawString(b.text, b.boundry.x + (b.boundry.w - textWidth) / 2 - 1, b.boundry.y + (b.boundry.h - textHeight) / 2 + borderThickness * 2);
}

void DisplayClearPartial()
{
  tft.fillRect(5, 60, 470, 225, TFT_BLACK);
}

void DisplaySetup()
{
  tft.fillScreen(TFT_BLACK);

  // Perimeter.
  int w = tft.width() - 1;
  int h = tft.height() - 1;
  int t = 3;
  tft.fillRect(0, 0, w, t, TFT_BLUE);
  tft.fillRect(w - t, 0, w, h, TFT_BLUE);
  tft.fillRect(0, h - t, tft.width(), t, TFT_BLUE);
  tft.fillRect(0, 0, 0 + t, h, TFT_BLUE);

  // Cross lines.
  tft.fillRect(0, 50, tft.width(), t, TFT_BLUE);
  tft.fillRect(0, 285, tft.width(), t, TFT_BLUE);

  // Status items.
  StatusItem _statusItems[3] = {StatusItem("SD", 0), StatusItem("WIFI", 0), StatusItem("TIME", 0)};
  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  int sX = 20;
  int sY = 292;
  const int sW = 50;
  const int sH = 20;
  for (auto &s : _statusItems)
  {
    tft.fillRect(sX, sY, sW, sH, s.status ? TFT_GREEN : TFT_RED);
    int textWidth = tft.textWidth(s.text);
    tft.drawString(s.text, sX + (sW - textWidth) / 2 - 1, sY + 2); // Manual verticle spacing fix.
    sX += sW + 20;
  }

  // Time.
  tft.setTextFont(GLCD);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("15:01:01", 360, 294);

  for (Button b : buttonsMain)
  {
    DisplayButton(b);
  }
}

void DisplayHomePage()
{
  // Song title.
  String songTitle = "Castle on the River";
  tft.setTextFont(GLCD);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  int textWidth = tft.textWidth(songTitle);
  tft.drawString(songTitle, (tft.width() - textWidth) / 2, 80, GFXFF);

  // Song border.
  tft.drawRect(20, 70, 440, 40, TFT_LIGHTGREY);

  // Progress bar.
  tft.drawRect(20, 120, 440, 15, TFT_LIGHTGREY);

  // Song time.
  tft.setTextSize(2);
  tft.drawString("2:33", 200, 155, GFXFF);
  // Song time.
  tft.setTextSize(2);
  tft.drawString("1 of 5", 200, 175, GFXFF);

  // Control buttons
  for (Button b : buttonsHome)
  {
    DisplayButton(b);
  }
}

void DisplayConfigurationPage()
{
  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  for (auto const &l : labelsConfiguration)
  {
    tft.setTextColor(l.textColor);
    tft.drawString(l.text, l.x, l.y);
  }

  for (auto const &b : buttonsConfiguration)
  {
    DisplayButton(b);
  }
}

void UpdateScreen()
{
  static PageId previousPageId = pageId;

  if (previousPageId != pageId)
  {
    previousPageId = pageId;

    if (pageId == PageId::HomePage)
    {
      DisplayClearPartial();
      DisplayHomePage();
    }
    else if (pageId == PageId::ConfigurationPage)
    {
      DisplayClearPartial();
      DisplayConfigurationPage();
    }
    else if (pageId == PageId::CalibrationPage)
    {
      DisplayClearPartial();
    }
  }
}

void CheckTouchScreen()
{
  uint16_t x, y;

  if (millis() - touchDebounceMillis > 250)
  {
    takeTouchReadings = true;
  }

  if (takeTouchReadings)
  {
    if (tft.getTouch(&x, &y))
    {

      for (auto &b : buttonsMain)
      {
        if (IsButtonPressed(b, x, y))
        {
          if (b.buttonId == ButtonId::Home)
          {
            pageId = PageId::HomePage;
          }
          else if (b.buttonId == ButtonId::Calibration)
          {
            pageId = PageId::CalibrationPage;
          }
          else if (b.buttonId == ButtonId::Configuration)
          {
            pageId = PageId::ConfigurationPage;
          }
        }
      }

      if (pageId == PageId::HomePage)
      {
        for (auto &b : buttonsHome)
        {
          if (IsButtonPressed(b, x, y))
          {
            if (b.buttonId == ButtonId::Previous)
            {
            }
            else if (b.buttonId == ButtonId::Next)
            {
            }
            else if (b.buttonId == ButtonId::Play)
            {
            }
            else if (b.buttonId == ButtonId::Pause)
            {
            }
            else if (b.buttonId == ButtonId::Stop)
            {
            }
          }
        }
      }

      if (pageId == PageId::ConfigurationPage)
      {
        for (auto &b : buttonsConfiguration)
        {
          if (IsButtonPressed(b, x, y))
          {
            if (b.buttonId == ButtonId::Hourly)
            {
            }
            else if (b.buttonId == ButtonId::StartHour)
            {
              configuration.startHour += 100;
              if (configuration.startHour > 2300)
              {
                configuration.startHour = 100;
              }
              String postFix = configuration.startHour < 1300 ? "pm" : "am";
              String hour = String(configuration.startHour > 1200 ? (configuration.startHour - 1200) / 100 : configuration.startHour / 100);

              b.text = hour + " " + postFix;
              DisplayButton(b);
            }
            else if (b.buttonId == ButtonId::EndHour)
            {
            }
            else if (b.buttonId == ButtonId::TimeZone)
            {
            }
            else if (b.buttonId == ButtonId::StartHour)
            {
            }
            else if (b.buttonId == ButtonId::Song)
            {
            }
          }
        }
      }

      if (pageId == PageId::CalibrationPage)
      {
      }
    }
  }
}

void SetupScreen()
{
  uint16_t calibrationData[5];
  uint8_t calDataOK = 0;

  tft.init();
  tft.setRotation(3);

  // check file system
  if (!SPIFFS.begin())
  {
    Serial.println("Formating file system.");

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

  // calDataOK = false;

  if (calDataOK)
  {
    Serial.println("GLCD calibration data OK.");
    tft.setTouch(calibrationData);
  }
  else
  {
    Serial.println("GLCD calibration data NOT OK.");
    tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calibrationData, 14);
      f.close();
    }
  }
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("starting");

  SetupScreen();

  DisplaySetup();
  DisplayHomePage();
}

void loop()
{
  CheckTouchScreen();

  UpdateScreen();
}