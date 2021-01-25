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

Labels labels(&tft);
Buttons buttons(&tft);

void InitScreenElements()
{
  labels.Add(int(PageId::All), Labels::Label(int(LabelId::PlayState), "", 2, 20, 294, TFT_BLACK, TFT_BLACK, Labels::Justification::Left));
  labels.Add(int(PageId::All), Labels::Label(int(LabelId::Clock), "Clock", 2, 470, 294, TFT_WHITE, TFT_BLACK, Labels::Justification::Right));
  labels.Add(int(PageId::All), Labels::Label(int(LabelId::SD), "SD", 2, 170, 294, TFT_BLACK, TFT_YELLOW, Labels::Justification::Center));
  labels.Add(int(PageId::All), Labels::Label(int(LabelId::Wifi), "WIFI", 2, 235, 294, TFT_BLACK, TFT_YELLOW, Labels::Justification::Center));
  labels.Add(int(PageId::All), Labels::Label(int(LabelId::Time), "TIME", 2, 310, 294, TFT_BLACK, TFT_YELLOW, Labels::Justification::Center));
  labels.Add(int(PageId::Home), Labels::Label(int(LabelId::SongTitle), "Song Title 2", 3, 240, 80, TFT_GREEN, TFT_BLACK, Labels::Justification::Center));
  labels.Add(int(PageId::Home), Labels::Label(int(LabelId::SongLength), "Length", 2, 230, 160, TFT_GREEN, TFT_BLACK, Labels::Justification::Center));
  labels.Add(int(PageId::Home), Labels::Label(int(LabelId::SongNumber), "Count", 2, 230, 185, TFT_GREEN, TFT_BLACK, Labels::Justification::Center));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Hourly", 2, 20, 70, TFT_WHITE, TFT_BLACK, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Start Hour", 2, 180, 70, TFT_WHITE, TFT_BLACK, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "End Hour", 2, 340, 70, TFT_WHITE, TFT_BLACK, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Time Zone", 2, 20, 140, TFT_WHITE, TFT_BLACK, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "On Startup", 2, 180, 140, TFT_WHITE, TFT_BLACK, Labels::Justification::Left));
  labels.Add(int(PageId::Configuration), Labels::Label(int(LabelId::Default), "Song", 2, 20, 210, TFT_WHITE, TFT_BLACK, Labels::Justification::Left));

  //labels.SetMinimumCharacters(int(PageId::All), int(LabelId::PlayState), 8);
  labels.SetPadding(int(PageId::All), int(LabelId::PlayState), 3);
  labels.SetPadding(int(PageId::All), int(LabelId::SD), 3);
  labels.SetPadding(int(PageId::All), int(LabelId::Wifi), 3);
  labels.SetPadding(int(PageId::All), int(LabelId::Time), 3);



  labels.SetMinimumCharacters(int(PageId::Home), int(LabelId::SongTitle), 22);
  labels.SetMinimumCharacters(int(PageId::Home), int(LabelId::SongLength), 11);
  labels.SetMinimumCharacters(int(PageId::Home), int(LabelId::SongNumber), 11);

  buttons.Add(int(PageId::All), Buttons::Button(int(ButtonId::Home), "HOME", 30, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::All), Buttons::Button(int(ButtonId::Configuration), "CONFIG.", 180, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::All), Buttons::Button(int(ButtonId::Calibration), "CALIB.", 330, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE));

  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Play), "PLAY", 30, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Pause), "PAUSE", 180, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Stop), "STOP", 330, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Previous), "PREV.", 30, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Home), Buttons::Button(int(ButtonId::Next), "NEXT", 330, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));

  buttons.Add(int(PageId::Configuration), Buttons::Button(int(ButtonId::Hourly), "Yes", 20, 90, buttonMainW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Configuration), Buttons::Button(int(ButtonId::StartHour), "900", 180, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Configuration), Buttons::Button(int(ButtonId::EndHour), "1300", 340, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Configuration), Buttons::Button(int(ButtonId::TimeZone), "EST -4 GMT", 20, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Configuration), Buttons::Button(int(ButtonId::Startup), "Yes", 180, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(int(PageId::Configuration), Buttons::Button(int(ButtonId::Song), "Random", 20, 230, 270, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

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

  tft.setTextFont(GLCD); // TODO: add fonts to the label class.
  buttons.DisplayButtons(int(PageId::All));
  labels.DisplayLabels(int(PageId::All));
}

void DisplayHomePage()
{
  // Song border.
  tft.drawRect(20, 70, 440, 40, TFT_LIGHTGREY);

  // Progress bar.
  tft.drawRect(20, 120, 440, 15, TFT_LIGHTGREY);

  labels.DisplayLabels(int(PageId::Home));
  buttons.DisplayButtons(int(PageId::Home));
}

void DisplayConfigurationPage()
{
  labels.DisplayLabels(int(PageId::Configuration));
  buttons.DisplayButtons(int(PageId::Configuration));
}

void UpdateScreen()
{

  static PlayState previousPlayState = PlayState::Default;
  if (previousPlayState != playState)
  {
    previousPlayState = playState;

    labels.UpdateLabelFillColor(int(PageId::All), int(LabelId::PlayState), playStateFillColor[int(playState)], false);
    labels.UpdateLabelText(int(PageId::All), int(LabelId::PlayState), String(playStateText[int(playState)]));
  }

  static PageId previousPageId = pageId;
  if (previousPageId != pageId)
  {
    previousPageId = pageId;

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

  signed id = -1;

  if (millis() - touchDebounceMillis > 250)
  {
    takeTouchReadings = true;
  }

  if (takeTouchReadings)
  {

    if (buttons.IsButtonPressed(int(PageId::All), &id))
    {

      if ((ButtonId)id == ButtonId::Home)
      {
        pageId = PageId::Home;
      }
      else if ((ButtonId)id == ButtonId::Calibration)
      {
        pageId = PageId::Calibration;
      }
      else if ((ButtonId)id == ButtonId::Configuration)
      {
        pageId = PageId::Configuration;
      }
    }

    if (pageId == PageId::Home)
    {
      if (buttons.IsButtonPressed(int(PageId::Home), &id))
      {
        if ((ButtonId)id == ButtonId::Previous)
        {
          playState = PlayState::Stop;
          if (selectedFileId-- == 0)
          {
            selectedFileId = midiFiles.size() - 1;
          }
          labels.UpdateLabelText(int(PageId::Home), int(LabelId::SongTitle), midiFiles[selectedFileId]);
          labels.UpdateLabelText(int(PageId::Home), int(LabelId::SongNumber), String(String(selectedFileId + 1) + " of " + midiFiles.size()));
        }
        else if ((ButtonId)id == ButtonId::Next)
        {

          playState = PlayState::Stop;
          if (selectedFileId++ == midiFiles.size() - 1)
          {
            selectedFileId = 0;
          }
          labels.UpdateLabelText(int(PageId::Home), int(LabelId::SongTitle), midiFiles[selectedFileId]);
          labels.UpdateLabelText(int(PageId::Home), int(LabelId::SongNumber), String(String(selectedFileId + 1) + " of " + midiFiles.size()));
        }
        else if ((ButtonId)id == ButtonId::Play)
        {
          playState = PlayState::Play;
        }
        else if ((ButtonId)id == ButtonId::Pause)
        {
          playState = PlayState::Pause;
        }
        else if ((ButtonId)id == ButtonId::Stop)
        {
          playState = PlayState::Stop;
        }
      }
    }
  }

  if (id != -1)
  {
    takeTouchReadings = false;
    touchDebounceMillis = millis();
  }
}

void ScreenInit()
{
  tft.init();
  tft.setRotation(3);

  uint16_t calibrationData[5] = {293, 3597, 267, 3597, 1};
  // Below line calls calibration routine.
  /*
  tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 15);
  for(int i = 0; i < 5; i ++)
  {
    Serial.println(calibrationData[i]);
  }
  */
  tft.setTouch(calibrationData);
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
  InitScreenElements();

  DisplayMain();
  DisplayHomePage();
}

void loop()
{
  CheckTouchScreen();

  UpdateScreen();
}