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
#include "gfxItems.h" // local libray
#include "main.h"
#include "graphicsMethods.h"


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
unsigned int selectedFileId = 0;

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


GFXItems gfxItems(&tft);



//////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateScreen()
{
  static PlayState previousPlayState = PlayState::Default;
  if (previousPlayState != playState)
  {
    previousPlayState = playState;

    gfxItems.GetGfxItemById(int(GFXItemId::PlayState)).text = String(playStateText[int(playState)]);
    gfxItems.GetGfxItemById(int(GFXItemId::PlayState)).fillColor = playStateFillColor[int(playState)];
    gfxItems.DisplayGfxItem(int(GFXItemId::PlayState));
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

    gfxItems.GetGfxItemById(int(GFXItemId::SongTitle)).text = midiFiles[selectedFileId];
    gfxItems.GetGfxItemById(int(GFXItemId::SongNumber)).text = String(String(selectedFileId + 1) + " of " + midiFiles.size());
    gfxItems.DisplayGfxItem(int(GFXItemId::SongTitle));
    gfxItems.DisplayGfxItem(int(GFXItemId::SongNumber));
  }
  else if ((GFXItemId)id == GFXItemId::Next)
  {

    playState = PlayState::Stop;
    if (selectedFileId++ == midiFiles.size() - 1)
    {
      selectedFileId = 0;
    }
    gfxItems.GetGfxItemById(int(GFXItemId::SongTitle)).text = midiFiles[selectedFileId];
    gfxItems.GetGfxItemById(int(GFXItemId::SongNumber)).text = String(String(selectedFileId + 1) + " of " + midiFiles.size());
    gfxItems.DisplayGfxItem(int(GFXItemId::SongTitle));
    gfxItems.DisplayGfxItem(int(GFXItemId::SongNumber));
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

  gfxItems.IsButtonInGroupPressed(int(PageId::All), &id);

  if (pageId == PageId::Home)
  {
    gfxItems.IsButtonInGroupPressed(int(PageId::Home), &id);
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



// Initialize SD Card
void SDInit()
{
  if (!SD.begin(SD_SELECT, SPI_FULL_SPEED))
  {
    Serial.println("SD Card: init failed!");
    DisplayFatalError(1);
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
  Serial.println("Melodic Chimes starting up.");

  ScreenInit();

  SDInit();

  InitScreenElements();

  DisplayMain();
  DisplayHomePage();
}

void loop()
{
  CheckTouchScreen();

  UpdateScreen();
}