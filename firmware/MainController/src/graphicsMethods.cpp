#include <TFT_eSPI.h>
#include "gfxItems.h"
#include "graphicsMethods.h"
#include "main.h"
#include "Free_Fonts.h"

extern TFT_eSPI tft;
extern GFXItems gfxItems;



void InitScreenElements()
{
    const int buttonMainW = 110;
    const int buttonMainH = 55;

    // Create and add buttons.
    const GFXfont *gfxFont = FF22;
    gfxItems.Add(GFXItem(int(GFXItemId::Home), "HOME", 30, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Configuration), "CONFIG.", 180, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Calibration), "CALIB.", 330, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE, gfxFont));

    gfxItems.Add(GFXItem(int(GFXItemId::Play), "PLAY", 30, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Pause), "PAUSE", 180, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Stop), "STOP", 330, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Previous), "PREV.", 30, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Next), "NEXT", 330, 150, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));

    gfxItems.Add(GFXItem(int(GFXItemId::Hourly), "Yes", 20, 90, buttonMainW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::StartHour), "900", 180, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::EndHour), "1300", 340, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::TimeZone), "EST -4 GMT", 20, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Startup), "Yes", 180, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
    gfxItems.Add(GFXItem(int(GFXItemId::Song), "Random", 20, 230, 270, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));

    // Add buttons to groups.
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::Home));
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::Configuration));
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::Calibration));

    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::Play));
    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::Pause));
    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::Stop));
    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::Previous));
    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::Next));

    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Hourly));
    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::StartHour));
    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::EndHour));
    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::TimeZone));
    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Startup));
    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Song));

    // Create and add labels.
    gfxItems.Add(GFXItem(int(GFXItemId::PlayState), "State", 2, 20, 294, 70, 20, TFT_BLACK, TFT_BLACK, Justification::Left));
    gfxItems.Add(GFXItem(int(GFXItemId::Clock), "Clock", 2, 390, 294, 60, 20, TFT_WHITE, TFT_BLACK, Justification::Right));
    gfxItems.Add(GFXItem(int(GFXItemId::SD), "SD", 2, 150, 294, 50, 20, TFT_BLACK, TFT_YELLOW, Justification::Center));
    gfxItems.Add(GFXItem(int(GFXItemId::Wifi), "WIFI", 2, 220, 294, 50, 20, TFT_BLACK, TFT_YELLOW, Justification::Center));
    gfxItems.Add(GFXItem(int(GFXItemId::Time), "TIME", 2, 300, 294, 50, 20, TFT_BLACK, TFT_YELLOW, Justification::Center));
    gfxItems.Add(GFXItem(int(GFXItemId::SongTitle), "Song Title 2", 3, 25, 72, 420, 35, TFT_GREEN, TFT_BLACK, Justification::Center));
    gfxItems.Add(GFXItem(int(GFXItemId::SongLength), "Length", 2, 175, 145, 120, 25, TFT_GREEN, TFT_BLACK, Justification::Center));
    gfxItems.Add(GFXItem(int(GFXItemId::SongNumber), "Count", 2, 175, 165, 120, 25, TFT_GREEN, TFT_BLACK, Justification::Center));
    gfxItems.Add(GFXItem(int(GFXItemId::Default), "Hourly", 2, 20, 70, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
    gfxItems.Add(GFXItem(int(GFXItemId::Default), "Start Hour", 2, 180, 70, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
    gfxItems.Add(GFXItem(int(GFXItemId::Default), "End Hour", 2, 340, 70, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
    gfxItems.Add(GFXItem(int(GFXItemId::Default), "Time Zone", 2, 20, 140, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
    gfxItems.Add(GFXItem(int(GFXItemId::Default), "On Startup", 2, 180, 140, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));
    gfxItems.Add(GFXItem(int(GFXItemId::Default), "Song", 2, 20, 210, 0, 0, TFT_WHITE, TFT_BLACK, Justification::Left));

    // Add labels to groups.
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::PlayState));
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::Clock));
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::SD));
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::Wifi));
    gfxItems.AddElementToGroup(int(PageId::All), int(GFXItemId::Time));

    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::SongTitle));
    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::SongLength));
    gfxItems.AddElementToGroup(int(PageId::Home), int(GFXItemId::SongNumber));

    gfxItems.AddElementToGroup(int(PageId::Configuration), int(GFXItemId::Default));
}

void DisplayFatalError(int error)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(GLCD);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(3);

  tft.drawString("Error: ", 50, 50);
  if (error == 1)
  {
    tft.drawString("SD card not detected!", 50, 100);
  }

  // TODO: tell chimes to halt activity.
  while (1)
    ;
}

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

    gfxItems.DisplayGroup(int(PageId::All));
}

void DisplayHomePage()
{
    // Song border.
    tft.drawRect(20, 70, 440, 40, TFT_LIGHTGREY);

    // Progress bar.
    tft.drawRect(20, 120, 440, 15, TFT_LIGHTGREY);

    gfxItems.DisplayGroup(int(PageId::Home));
}

void DisplayConfigurationPage()
{
    gfxItems.DisplayGroup(int(PageId::Configuration));
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

