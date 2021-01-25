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

  /*
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
*/

  //labels.SetMinimumCharacters(int(PageId::All), int(LabelId::PlayState), 8);
  labels.SetPadding(int(PageId::All), int(LabelId::PlayState), 3);
  labels.SetPadding(int(PageId::All), int(LabelId::SD), 3);
  labels.SetPadding(int(PageId::All), int(LabelId::Wifi), 3);
  labels.SetPadding(int(PageId::All), int(LabelId::Time), 3);

  labels.SetMinimumCharacters(int(PageId::Home), int(LabelId::SongTitle), 22);
  labels.SetMinimumCharacters(int(PageId::Home), int(LabelId::SongLength), 11);
  labels.SetMinimumCharacters(int(PageId::Home), int(LabelId::SongNumber), 11);

  // Create and add buttons.
  buttons.Add(GFXItem(int(GFXItemId::Home), "HOME", 30, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Configuration), "CONFIG.", 180, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Calibration), "CALIB.", 330, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE));

  buttons.Add(GFXItem(int(GFXItemId::Play), "PLAY", 30, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Pause), "PAUSE", 180, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Stop), "STOP", 330, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Previous), "PREV.", 30, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Next), "NEXT", 330, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));

  buttons.Add(GFXItem(int(GFXItemId::Hourly), "Yes", 20, 90, buttonMainW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::StartHour), "900", 180, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::EndHour), "1300", 340, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::TimeZone), "EST -4 GMT", 20, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Startup), "Yes", 180, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));
  buttons.Add(GFXItem(int(GFXItemId::Song), "Random", 20, 230, 270, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE));

  // Add buttons to groups.
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::Home));
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::Configuration));
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::Calibration));

  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::Play));
  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::Pause));
  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::Stop));
  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::Previous));
  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::Next));

  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Hourly));
  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::StartHour));
  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::EndHour));
  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::TimeZone));
  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Startup));
  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Song));

  // Add labels.
  buttons.Add(GFXItem(int(GFXItemId::PlayState), "State", 2, 20, 294, 70, 20, TFT_BLACK, TFT_BLACK, Justification::Left));
  buttons.Add(GFXItem(int(GFXItemId::Clock), "Clock", 2, 390, 294, 60, 20, TFT_WHITE, TFT_BLACK, Justification::Right));
  buttons.Add(GFXItem(int(GFXItemId::SD), "SD", 2, 150, 294, 50, 20, TFT_BLACK, TFT_YELLOW, Justification::Center));
  buttons.Add(GFXItem(int(GFXItemId::Wifi), "WIFI", 2, 220, 294, 50, 20, TFT_BLACK, TFT_YELLOW, Justification::Center));
  buttons.Add(GFXItem(int(GFXItemId::Time), "TIME", 2, 300, 294, 50, 20, TFT_BLACK, TFT_YELLOW, Justification::Center));
  buttons.Add(GFXItem(int(GFXItemId::SongTitle), "Song Title 2", 3, 25, 72, 420, 35,TFT_GREEN, TFT_BLACK, Justification::Center));
  buttons.Add(GFXItem(int(GFXItemId::SongLength), "Length", 2, 175, 145, 120, 30,TFT_GREEN, TFT_BLACK, Justification::Center));
  buttons.Add(GFXItem(int(GFXItemId::SongNumber), "Count", 2, 175, 165, 120, 30,TFT_GREEN, TFT_BLACK, Justification::Center));
  buttons.Add(GFXItem(int(GFXItemId::Default), "Hourly", 2, 20, 70, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
  buttons.Add(GFXItem(int(GFXItemId::Default), "Start Hour", 2, 180, 70, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
  buttons.Add(GFXItem(int(GFXItemId::Default), "End Hour", 2, 340, 70, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
  buttons.Add(GFXItem(int(GFXItemId::Default), "Time Zone", 2, 20, 140, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
  buttons.Add(GFXItem(int(GFXItemId::Default), "On Startup", 2, 180, 140, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
  buttons.Add(GFXItem(int(GFXItemId::Default), "Song", 2, 20, 210, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));

  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::PlayState));
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::Clock));
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::SD));
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::Wifi));
  buttons.AddElementToGroup(int(PageId::All), int(GFXItemId::Time));

  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::SongTitle));
  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::SongLength));
  buttons.AddElementToGroup(int(PageId::Home), int(GFXItemId::SongNumber));

  buttons.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Default));
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
  buttons.DisplayGroup(int(PageId::All));
}

void DisplayHomePage()
{
  // Song border.
  tft.drawRect(20, 70, 440, 40, TFT_LIGHTGREY);

  // Progress bar.
  tft.drawRect(20, 120, 440, 15, TFT_LIGHTGREY);

  buttons.DisplayGroup(int(PageId::Home));
}

void DisplayConfigurationPage()
{
  buttons.DisplayGroup(int(PageId::Configuration));
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

void ProcessPressedButton(int id)
{

  if ((GFXItemId)id == GFXItemId::Home)
  {
    pageId = PageId::Home;
  }
  else if ((GFXItemId)id == GFXItemId::Calibration)
  {
    pageId = PageId::Calibration;
  }
  else if ((GFXItemId)id == GFXItemId::Configuration)
  {
    pageId = PageId::Configuration;
  }

  if ((GFXItemId)id == GFXItemId::Previous)
  {
    playState = PlayState::Stop;
    if (selectedFileId-- == 0)
    {
      selectedFileId = midiFiles.size() - 1;
    }

    buttons.GetButtonById(int(GFXItemId::SongTitle)).text = midiFiles[selectedFileId];
    buttons.GetButtonById(int(GFXItemId::SongNumber)).text = String(String(selectedFileId + 1) + " of " + midiFiles.size());
    buttons.DisplayButton(int(GFXItemId::SongTitle));
    buttons.DisplayButton(int(GFXItemId::SongNumber));
  }
  else if ((GFXItemId)id == GFXItemId::Next)
  {

    playState = PlayState::Stop;
    if (selectedFileId++ == midiFiles.size() - 1)
    {
      selectedFileId = 0;
    }
    buttons.GetButtonById(int(GFXItemId::SongTitle)).text = midiFiles[selectedFileId];
    buttons.GetButtonById(int(GFXItemId::SongNumber)).text = String(String(selectedFileId + 1) + " of " + midiFiles.size());
    buttons.DisplayButton(int(GFXItemId::SongTitle));
    buttons.DisplayButton(int(GFXItemId::SongNumber));
  }
  else if ((GFXItemId)id == GFXItemId::Play)
  {
    playState = PlayState::Play;
  }
  else if ((GFXItemId)id == GFXItemId::Pause)
  {
    playState = PlayState::Pause;
  }
  else if ((GFXItemId)id == GFXItemId::Stop)
  {
    playState = PlayState::Stop;
  }
}

void CheckTouchScreen()
{

  signed id = -1;

  if (millis() - touchDebounceMillis > 250)
  {
    takeTouchReadings = true;
  }

  buttons.IsButtonInGroupPressed(int(PageId::All), &id);

  if (pageId == PageId::Home)
  {
    buttons.IsButtonInGroupPressed(int(PageId::Home), &id);
  }

  if (takeTouchReadings)
  {
    if (id != -1)
    {

      ProcessPressedButton(id);
      takeTouchReadings = false;
      touchDebounceMillis = millis();
    }
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