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

/*
struct ButtonType
{
  Text,
  LeftArrow,
  RightArrow
};
*/
enum ButtonId
{
  Default,
  Prev,
  Menu,
  Next,
  Play,
  Pause,
  Start,
  Hourly,
  StartHour,
  EndHour

};

struct Button
{
  String name;
  ButtonId buttonId;
  Boundry boundry;

  uint32_t textColor = TFT_BLACK;
  uint32_t fillColor = TFT_GREEN;
  uint32_t activeColor = TFT_WHITE;
  uint32_t borderColor = TFT_DARKGREEN;

  Button(ButtonId buttonId, String name, Boundry boundry, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
  {
    this->name = name;
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
  String name;
  ButtonId buttonId;
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

Button buttons[6] = {Button(ButtonId::Default, "PLAY", Boundry(40, 220, 100, 50), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY),
                     Button(ButtonId::Default, "PAUSE", Boundry(190, 220, 100, 50), TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_LIGHTGREY),
                     Button(ButtonId::Default, "STOP", Boundry(340, 220, 100, 50), TFT_BLACK, TFT_RED, TFT_WHITE, TFT_LIGHTGREY),
                     Button(ButtonId::Default, "PREV", Boundry(40, 60, 100, 50), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_LIGHTGREY),
                     Button(ButtonId::Default, "MENU", Boundry(190, 60, 100, 50), TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_LIGHTGREY),
                     Button(ButtonId::Default, "NEXT", Boundry(340, 60, 100, 50), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_LIGHTGREY)};

Label labels[3] = {Label("SD", Boundry(120, 293, 50, 18), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY),
                   Label("WIFI", Boundry(190, 293, 50, 18), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY),
                   Label("API", Boundry(260, 293, 50, 18), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY)};

/*
Button buttonsChime[6] = {Button("Hourly", Boundry(40, 220, 100, 50), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY),
                     Button("PAUSE", Boundry(190, 220, 100, 50), TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_LIGHTGREY),
                     Button("STOP", Boundry(340, 220, 100, 50), TFT_BLACK, TFT_RED, TFT_WHITE, TFT_LIGHTGREY),
                     Button("PREV", Boundry(40, 60, 100, 50), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_LIGHTGREY),
                     Button("MENU", Boundry(190, 60, 100, 50), TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_LIGHTGREY),
                     Button("NEXT", Boundry(340, 60, 100, 50), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_LIGHTGREY)};
                     */

std::list<Button> _buttonsChime;

/*
enum MenuId
{
  Chime,
  Play,
  Calibration,
  ChimeHourly
};

struct MenuItem
{
  String name;
  MenuId id;
  std::list<MenuItem> menuItems;

  MenuItem(MenuId id, String name)
  {
    this->id = id;
    this->name = name;
  }
};

std::list<MenuItem> _menuItems;

void AddMenuItem(MenuId id, MenuItem mi)
{
  for (auto &m : _menuItems)
  {
    if (m.id == id)
    {
      m.menuItems.push_back(mi);
    }
  }
}

void DisplayMenu()
{

  _menuItems.push_back(MenuItem(Chime, "Chime"));
  _menuItems.push_back(MenuItem(Play, "Play"));
  _menuItems.push_back(MenuItem(Calibration, "Calibration"));

  AddMenuItem(Chime, MenuItem(ChimeHourly, "Chime Hourly"));

  int w = tft.width() - 1;
  int h = tft.height() - 1;
  int t = 3;

  tft.fillScreen(TFT_BLACK);

  // Perimeter.
  tft.fillRect(0, 0, w, t, TFT_BLUE);
  tft.fillRect(w - t, 0, w, h, TFT_BLUE);
  tft.fillRect(0, h - t, tft.width(), t, TFT_BLUE);
  tft.fillRect(0, 0, 0 + t, h, TFT_BLUE);

  int y = 50;

  for (auto const &m : _menuItems)
  {
    Serial.println(m.name);

    tft.setFreeFont(FF22);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(1);

    tft.fillRect(150, y, 200, 40, TFT_GREEN);

    //int textWidth = tft.textWidth(m.name);
    //int textHeight = tft.fontHeight();
    tft.drawString(m.name, 150 + 20, y);

    if (m.menuItems.size() != 0)
    {
      tft.fillRect(150 + 20, y + 20, 10, 10, TFT_BLACK);
    }

    y += 50;
  }
}
*/

enum PageId
{
  Home,
  Chime,
  Calibration
};

void DisplayPage(PageId pageId)
{

  int w = tft.width() - 1;
  int h = tft.height() - 1;
  tft.fillScreen(TFT_BLACK);

  int buttonHeight = 30;

  tft.setFreeFont(FF22);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);

  std::list<String> labels = {"Hourly :", "Start Hour :", "End Hour :", "Time Zone :", "Song :", "Play On Startup :"};

  int labelY = 60;
  for (auto const &l : labels)
  {
    tft.drawString(l, 30, labelY);
    labelY += 40;
  }

  _buttonsChime.push_back(Button(ButtonId::Hourly, "Yes", Boundry(170, 75, 300, buttonHeight), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY));
  _buttonsChime.push_back(Button(ButtonId::StartHour, "900", Boundry(170, 75, 300, buttonHeight), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY));
  _buttonsChime.push_back(Button(ButtonId::EndHour, "2200", Boundry(170, 75, 300, buttonHeight), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY));
  _buttonsChime.push_back(Button(ButtonId::EndHour, "EST (GMT - 4)", Boundry(170, 155, 300, buttonHeight), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY));
  _buttonsChime.push_back(Button(ButtonId::EndHour, "Random", Boundry(170, 235, 300, buttonHeight), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY));
  _buttonsChime.push_back(Button(ButtonId::EndHour, "Yes", Boundry(170, 235, 300, buttonHeight), TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_LIGHTGREY));

  int buttonY = 60;
  for (auto const &b : _buttonsChime)
  {
    tft.setFreeFont(FF22);
    tft.setTextColor(b.textColor);
    tft.setTextSize(1);
    int borderThickness = 3;
    tft.fillRoundRect(b.boundry.x, buttonY, b.boundry.w, b.boundry.h, 10, b.borderColor);
    tft.fillRoundRect(b.boundry.x + borderThickness, buttonY + borderThickness, b.boundry.w - borderThickness * 2, b.boundry.h - borderThickness * 2, 5, b.fillColor);

    int textWidth = tft.textWidth(b.name);
    int textHeight = tft.fontHeight();
    tft.drawString(b.name, b.boundry.x + (b.boundry.w - textWidth) / 2 - 1, buttonY + (b.boundry.h - textHeight) / 2 + borderThickness * 2);

    buttonY += 40;
  }
}

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
  tft.fillRect(0, 43, tft.width(), t, TFT_BLUE);
  tft.fillRect(0, 285, tft.width(), t, TFT_BLUE);

  //
  tft.setFreeFont(FF23);
  tft.setTextColor(TFT_BLUE);
  int textWidth = tft.textWidth("MELODIC CHIMES");
  tft.drawString("MELODIC CHIMES", (tft.width() - textWidth) / 2, 10);

  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_BLUE);
  tft.setTextSize(1);
  tft.drawString("STATUS:", 20, 295);

  /*
  tft.setTextColor(TFT_GREEN);
  tft.setTextFont(GLCD);
  tft.setTextSize(2);
  tft.drawString("Song:", 70, 80, GFXFF);
  */

  // Song title.
  String songTitle = "Castle on the River";
  tft.setTextColor(TFT_GREEN);
  tft.setTextFont(GLCD);
  tft.setTextSize(3);
  textWidth = tft.textWidth(songTitle);
  tft.drawString(songTitle, (tft.width() - textWidth) / 2, 155, GFXFF);

  // Song border.
  tft.drawRect(40, 145, 400, 40, TFT_LIGHTGREY);

  // Progress bar.
  tft.drawRect(40, 195, 400, 15, TFT_LIGHTGREY);

  // Song time.
  tft.setTextSize(2);
  tft.drawString("2:33", 45, 127, GFXFF);

  // Song time.
  tft.setTextSize(2);
  tft.drawString("1 of 5", 365, 127, GFXFF);

  // Control buttons
  for (Button b : buttons)
  {
    tft.setFreeFont(FF22);
    tft.setTextColor(b.textColor);
    tft.setTextSize(1);
    int borderThickness = 3;
    tft.fillRoundRect(b.boundry.x, b.boundry.y, b.boundry.w, b.boundry.h, 10, b.borderColor);
    tft.fillRoundRect(b.boundry.x + borderThickness, b.boundry.y + borderThickness, b.boundry.w - borderThickness * 2, b.boundry.h - borderThickness * 2, 5, b.fillColor);

    int textWidth = tft.textWidth(b.name);
    int textHeight = tft.fontHeight();
    tft.drawString(b.name, b.boundry.x + (b.boundry.w - textWidth) / 2 - 1, b.boundry.y + (b.boundry.h - textHeight) / 2 + borderThickness * 2);
  }

  //tft.drawTriangle(10, 130, 40, 160, 10, 190, TFT_CYAN);

  // Status labels.
  for (Label l : labels)
  {
    tft.setFreeFont(FF21);
    tft.setTextColor(l.textColor);
    tft.setTextSize(1);
    tft.fillRect(l.boundry.x, l.boundry.y, l.boundry.w, l.boundry.h, l.fillColor);
    int textWidth = tft.textWidth(l.name);
    int textHeight = tft.fontHeight();
    tft.drawString(l.name, l.boundry.x + (l.boundry.w - textWidth) / 2, l.boundry.y + 2); // Manual verticle spacing fix.
  }

  // Time.
  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.drawString("15:01:01", 360, 295);
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
  // DisplayMenu();

  //DisplayPage(PageId::Chime);
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