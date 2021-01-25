// Crude proof of concept.

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "msTimer.h"
#include <SdFat.h>
#include <list>
#include <vector>
#include <map>
#include "main.h"

#include "labels.h"  // local library
#include "buttons.h" // local libray

// TFT Screen.
// TFT configuration contained in User_Setup.h in the local library.
TFT_eSPI tft = TFT_eSPI();
bool takeTouchReadings = true;
unsigned long touchDebounceMillis = millis();

// SD Card.
const uint8_t SD_SELECT = 8;
SdFat32 SD;
File32 dir;
File32 file;

// General system.
PageId pageId = PageId::Home;
PlayState playState = PlayState::Stop;

// MIDI system.
std::vector<String> midiFiles;
int selectedFileId = 0;

// Configuration.
struct Configuration
{
  bool hourly = true;
  int startHour = 900;
  int endHour = 2200;
  signed int timeZone = -4;
  bool startup = true;
  String songName = "Default Song";
} configuration;

const int buttonMainW = 110;
const int buttonMainH = 55;

/*
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
                                 Button(ButtonId::Song, "Random", Boundry(20, 230, 270, 35), TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE)};
*/
/*
Label labelsMain[] = {Label(LabelId::MainState, "MainState", 2, 20, 294, TFT_WHITE, Justification::Left),
                      Label(LabelId::Time, "Time", 2, 260, 460, TFT_WHITE, Justification::Right)};

Label labelsHome[] = {Label(LabelId::SongTitle, "Song Title", 3, 240, 80, TFT_GREEN, Justification::Center),
                      Label(LabelId::SongLength, "Length", 2, 240, 160, TFT_GREEN, Justification::Center),
                      Label(LabelId::SongNumber, "Count", 2, 240, 185, TFT_GREEN, Justification::Center)};

Label labelsConfiguration[] = {Label(LabelId::Default, "Hourly", 2, 20, 70, TFT_WHITE, Justification::Left),
                               Label(LabelId::Default, "Start Hour", 2, 180, 70, TFT_WHITE, Justification::Left),
                               Label(LabelId::Default, "End Hour", 2, 340, 70, TFT_WHITE, Justification::Left),
                               Label(LabelId::Default, "Time Zone", 2, 20, 140, TFT_WHITE, Justification::Left),
                               Label(LabelId::Default, "On Startup", 2, 180, 140, TFT_WHITE, Justification::Left),
                               Label(LabelId::Default, "Song", 2, 20, 210, TFT_WHITE, Justification::Left)};
                               */

StatusItem _statusItems[] = {StatusItem("SD", 0),
                             StatusItem("WIFI", 0),
                             StatusItem("TIME", 0)};

//std::map<std::pair<PageId, LabelId>, Label> labels = {{std::make_pair(PageId::HomePage, LabelId::MainState), Label(LabelId::MainState, "MainState", 2, 20, 294, TFT_WHITE, Justification::Left)}};

Labels labels(&tft);
Buttons buttons(&tft);

void InitScreenElements()
{

  labels.Add(int(PageId::All), Labels::Label(int(LabelId::MainState), "MainState", 2, 20, 294, TFT_WHITE, Labels::Justification::Left));
  labels.Add(int(PageId::All), Labels::Label(int(LabelId::Time), "Time", 2, 260, 460, TFT_WHITE, Labels::Justification::Right));
  labels.Add(int(PageId::Home), Labels::Label(int(LabelId::SongTitle), "Song Title 2", 3, 240, 80, TFT_GREEN, Labels::Justification::Center));
  labels.Add(int(PageId::Home), Labels::Label(int(LabelId::SongLength), "Length", 2, 240, 160, TFT_GREEN, Labels::Justification::Center));
  labels.Add(int(PageId::Home), Labels::Label(int(LabelId::SongNumber), "Count", 2, 240, 185, TFT_GREEN, Labels::Justification::Center));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Hourly", 2, 20, 70, TFT_WHITE, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Start Hour", 2, 180, 70, TFT_WHITE, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "End Hour", 2, 340, 70, TFT_WHITE, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Time Zone", 2, 20, 140, TFT_WHITE, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "On Startup", 2, 180, 140, TFT_WHITE, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Song", 2, 20, 210, TFT_WHITE, Labels::Justification::Left));

  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Play), "PLAY", 30, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Pause), "PAUSE", 180, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Stop), "STOP", 330, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Previous), "PREV.", 30, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Next), "NEXT",330, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));

  //labels.DisplayLabels(int(PageId::All));
  // labels.DisplayLabels(int(PageId::Home));
  //labels.DisplayLabels(int(PageId::Configuration));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

/*
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
*/



void DisplayClearPartial()
{
  tft.fillRect(5, 60, 470, 225, TFT_BLACK);
}

void DisplayMain()
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
  tft.setFreeFont(FF21);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  int sX = 160;
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



  tft.setTextFont(GLCD); // TODO: add fonts to the label class.
  /*
  for (Label l : labelsMain)
  {
    DisplayLabel(l);
  }
  */
}

void DisplayHomePage()
{
  // Song border.
  tft.drawRect(20, 70, 440, 40, TFT_LIGHTGREY);

  // Progress bar.
  tft.drawRect(20, 120, 440, 15, TFT_LIGHTGREY);



  /*
  tft.setTextFont(GLCD); // TODO: add fonts to the label class.
  for (Label l : labelsHome)
  {
    DisplayLabel(l);
  }
  */
}

void DisplayConfigurationPage()
{


  /*
  tft.setFreeFont(FF21); // TODO: add fonts to the label class.
  for (auto const &l : labelsConfiguration)
  {
    DisplayLabel(l);
  }
  */
}

void UpdateScreen()
{
  static PageId previousPageId = pageId;

  if (previousPageId != pageId)
  {
    previousPageId = pageId;

    labels.DisplayLabels(int(pageId));
 buttons.DisplayButtons(int(pageId));




    if (pageId == PageId::Home)
    {
      DisplayClearPartial();
      DisplayHomePage();
    }
    else if (pageId == PageId::Configuration)
    {
      DisplayClearPartial();
      DisplayConfigurationPage();
    }
    else if (pageId == PageId::Calibration)
    {
      DisplayClearPartial();
    }
  }
}

void CheckTouchScreen()
{
  /*
  uint16_t x, y;

  if (millis() - touchDebounceMillis > 250)
  {
    takeTouchReadings = true;
  }

  if (takeTouchReadings)
  {
    if (tft.getTouch(&x, &y, 20))
    {
     
      for (auto &b : buttonsMain)
      {
        if (IsButtonPressed(b, x, y))
        {
          if (b.buttonId == ButtonId::Home)
          {
            pageId = PageId::Home;
          }
          else if (b.buttonId == ButtonId::Calibration)
          {
            pageId = PageId::Calibration;
          }
          else if (b.buttonId == ButtonId::Configuration)
          {
            pageId = PageId::Configuration;
          }
        }
      }

      if (pageId == PageId::Home)
      {
        for (auto &b : buttonsHome)
        {
          if (IsButtonPressed(b, x, y))
          {
            if (b.buttonId == ButtonId::Previous)
            {
            
              playState = PlayState::Stop;
              if (selectedFileId-- == 0)
              {
                selectedFileId = midiFiles.size() - 1;
              }

              labelsHome[int(LabelId::SongTitle)].text = midiFiles[selectedFileId];
              DisplayLabel(labelsHome[int(LabelId::SongTitle)]);
             

              // Serial.println(labelsHome[int(LabelId::SongTitle)].text);
            }
            else if (b.buttonId == ButtonId::Next)
            {
              
              playState = PlayState::Stop;
              if (selectedFileId++ > midiFiles.size() - 1)
              {
                selectedFileId = 0;
              }
            

              // DisplayLabel(labelsHome[int(LabelId::SongTitle)]);
            }
            else if (b.buttonId == ButtonId::Play)
            {
              playState = PlayState::Play;
            }
            else if (b.buttonId == ButtonId::Pause)
            {
              playState = PlayState::Pause;
            }
            else if (b.buttonId == ButtonId::Stop)
            {
              playState = PlayState::Stop;
            }
          }
        }
      }

      if (pageId == PageId::Configuration)
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

      if (pageId == PageId::Calibration)
      {
      }
    }
  }
  */
}

void ScreenInit()
{
  //uint16_t calibrationData[5];
  //uint8_t calDataOK = 0;

  tft.init();
  tft.setRotation(3);

  /*

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
  */
}

// Initialize SD Card
void SDInit()
{
  if (!SD.begin(SD_SELECT, SPI_FULL_SPEED))
  {
    Serial.println("SD Card: init failed!");

    while (true)
      ;
  }

  Serial.println("SD Card: init successful.");

  Serial.println("SD Card: finding .mid files.");
  // Open root directory
  if (!dir.open("/"))
  {
    Serial.println("SD Card: dir.open() failed!");
  }

  while (file.openNext(&dir, O_RDONLY))
  {
    char buf[25];
    file.getName(buf, 25);
    String fileName(buf);
    String extension = fileName.substring(fileName.length() - 3);
    if (extension.equals("mid") || extension.equals("MID"))
    {
      midiFiles.push_back(fileName);
    }
    file.close();
  }
  if (dir.getError())
  {
    Serial.println("SD Card: openNext() failed!");
  }
  else
  {
    Serial.printf("SD Card: %u .mid files found.\n", midiFiles.size());
  }

  for (auto &s : midiFiles)
  {
    Serial.println(s);
  }
}

void setup(void)
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("starting");

  SDInit();

  ScreenInit();

  DisplayMain();

  InitScreenElements();

  DisplayHomePage();


   labels.DisplayLabels(int(pageId));
 buttons.DisplayButtons(int(pageId));


  Serial.println("Screen printed.");

}

void loop()
{
  CheckTouchScreen();

  UpdateScreen();
}