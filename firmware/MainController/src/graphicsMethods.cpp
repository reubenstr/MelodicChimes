#include <TFT_eSPI.h>
#include "gfxItems.h"
#include "graphicsMethods.h"
#include "main.h"
#include "Free_Fonts.h"
#include <splash.h>

extern TFT_eSPI tft;
extern GFXItems gfxItems;

void DisplayElementsInit()
{
  const int buttonMainW = 110;
  const int buttonMainH = 55;

  // Create and add buttons.
  const GFXfont *gfxFont = FF22;

  // All.
  gfxItems.Add(GFXItem(int(GFXItemId::Home), int(PageId::All), "HOME", 240, 160, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE, gfxFont));
 

  gfxItems.Add(GFXItem(int(GFXItemId::Configuration), int(PageId::All), "CONFIG.", 180, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Calibration), int(PageId::All), "CALIB.", 330, 10, buttonMainW, 35, TFT_BLACK, TFT_MAGENTA, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Play), int(PageId::Home), "PLAY", 40, 210, buttonMainW, buttonMainH, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  // gfxItems.Add(GFXItem(int(GFXItemId::Pause), int(PageId::Home), "PAUSE", 180, 220, buttonMainW, buttonMainH, TFT_BLACK, TFT_YELLOW, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Stop), int(PageId::Home), "STOP", 320, 210, buttonMainW, buttonMainH, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Previous), int(PageId::Home), "PREV.", 40, 130, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Next), int(PageId::Home), "NEXT", 320, 130, buttonMainW, buttonMainH, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
 
 /*
  // Configuration.
  gfxItems.Add(GFXItem(int(GFXItemId::Hourly), int(PageId::Configuration), "Yes", 20, 90, buttonMainW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::StartHour), int(PageId::Configuration), "900", 180, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::EndHour), int(PageId::Configuration), "1300", 340, 90, buttonMainW, 35, TFT_BLACK, TFT_RED, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::TimeZone), int(PageId::Configuration), "EST -4 GMT", 20, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Startup), int(PageId::Configuration), "Yes", 180, 160, buttonMainW, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Song), int(PageId::Configuration), "Random", 20, 230, 270, 35, TFT_BLACK, TFT_BLUE, TFT_WHITE, TFT_WHITE, gfxFont));

  // Calibration.
  const int buttonCalW = 85;
  gfxItems.Add(GFXItem(int(GFXItemId::Chime_3_up), int(PageId::Restring), "UP", 75, 150, buttonCalW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime_2_up), int(PageId::Restring), "UP", 195, 150, buttonCalW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime_1_up), int(PageId::Restring), "UP", 315, 150, buttonCalW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime_3_down), int(PageId::Restring), "DOWN", 75, 210, buttonCalW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime_2_down), int(PageId::Restring), "DOWN", 195, 210, buttonCalW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime_1_down), int(PageId::Restring), "DOWN", 315, 210, buttonCalW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));

  const int buttonVolW = 85;
  gfxItems.Add(GFXItem(int(GFXItemId::Chime3VolumePlus), int(PageId::Volume), "+", 75, 125, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime2VolumePlus), int(PageId::Volume), "+", 195, 125, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime1VolumePlus), int(PageId::Volume), "+", 315, 125, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));

  gfxItems.Add(GFXItem(int(GFXItemId::Chime3pick), int(PageId::Volume), "Pick", 75, 180, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime2pick), int(PageId::Volume), "Pick", 195, 180, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime1pick), int(PageId::Volume), "Pick", 315, 180, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));

  gfxItems.Add(GFXItem(int(GFXItemId::Chime3VolumeMinus), int(PageId::Volume), "-", 75, 235, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime2VolumeMinus), int(PageId::Volume), "-", 195, 235, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Chime1VolumeMinus), int(PageId::Volume), "-", 315, 235, buttonVolW, 35, TFT_BLACK, TFT_GREEN, TFT_WHITE, TFT_WHITE, gfxFont));

  // Create and add labels.
  gfxItems.Add(GFXItem(int(GFXItemId::SongTitle), int(PageId::Home), "Song Title 2", 3, 25, 72, 420, 35, TFT_GREEN, TFT_BLACK, TC_DATUM));
  // gfxItems.Add(GFXItem(int(GFXItemId::SongLength), int(PageId::Home), "Length", 2, 175, 160, 120, 25, TFT_GREEN, TFT_BLACK, TC_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::SongNumber), int(PageId::Home), "Count", 2, 175, 140, 120, 25, TFT_GREEN, TFT_BLACK, TC_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Configuration), "Hourly", 2, 20, 70, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Configuration), "Start Hour", 2, 180, 70, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Configuration), "End Hour", 2, 340, 70, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Configuration), "Time Zone", 2, 20, 140, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Configuration), "On Startup", 2, 180, 140, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Configuration), "Song", 2, 20, 210, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM));

  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Restring), "Restring", 1, 240, 90, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM, gfxFont));
  gfxItems.Add(GFXItem(int(GFXItemId::Default), int(PageId::Volume), "Volume", 1, 240, 90, 0, 0, TFT_WHITE, TFT_BLACK, TL_DATUM, gfxFont));
  */
}

void DisplayError(ErrorCodes error)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(GLCD);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(3);

  tft.drawString("Error: ", 50, 50);
  if (error == ErrorCodes::sdCardInitFailed)
  {
    tft.drawString("SD card not detected!", 50, 100);
  }
  else if (error == ErrorCodes::midiFilesNotFound)
  {
    tft.drawString("MIDI file(s) not found!", 50, 100);
  }
   else if (error == ErrorCodes::midiFilesNotFound)
  {
    tft.drawString("Parameter file not found or corrupted!", 50, 100);
  }
  else if (error == ErrorCodes::chimeFailed)
  {
    tft.drawString("Frequency not detected!", 50, 100);
    tft.drawString("Check if string(s) are in range.", 50, 150);
  }

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

void DisplayInit()
{
  tft.init();
  delay(150); // TFT driver needs time to process command.
  tft.setRotation(1);
  delay(150); // TFT driver needs time to process command. 

  //tft.pushImage(0, 0, splashWidth, splashHeight, splash);
  //delay(2000);
}

void DisplayIndicator(String string, int x, int y, uint16_t color)
{ 
  tft.setTextFont(1);
  tft.setTextSize(2);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_BLACK, color);
  int padding = tft.textWidth(string);
  tft.setTextPadding(padding + 6);
  tft.drawFastHLine(x - ((padding + 6) / 2), y - 1, padding + 6, color);
  tft.drawFastHLine(x - ((padding + 6) / 2), y - 2, padding + 6, color);
  tft.drawString(string.c_str(), x, y);
}
