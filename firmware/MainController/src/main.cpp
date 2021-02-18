/*  
  Project: Melodic Chimes
  Description: Four auto-tuning strings playing melodic music from custom MIDI files.

  MCUs:
      1x ESP32 (DevkitV1) : Main controller parsing MIDI files and fetching time from WiFi.
      2x Teensy 3.2 : Each determines frequency of two strings and controls two chimes.

  Hardware:
      TFT Touch Screen : User interface.
      SD-Card : contains MIDI files.
      12x Stepper Motors : 4x tuning, 4x muting, 4x picking.
      12x Stepper motor drivers.
      
  Notes:

    TODO


    MD_MIDIFile.h line 899 fix:
        void setFileFolder(const char* apath) { if (apath != nullptr) _sd->chdir(apath); }

  Online Tools:
      MID to JSON: https://tonejs.github.io/Midi/


*/

/*
// tft esp pins
#define ILI9488_DRIVER 
#define TFT_WIDTH  320
#define TFT_HEIGHT 480
#define TFT_CS 5
#define TFT_DC 2
#define TFT_SCLK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_RST 4

#define TOUCH_CS 22     // Chip select pin (T_CS) of touch screen
*/

/*
// tft teensy pins
#define ILI9488_DRIVER 
#define TFT_WIDTH  320
#define TFT_HEIGHT 480
#define TFT_CS 4
#define TFT_DC 6
#define TFT_SCLK 13
#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_RST 5

#define TOUCH_CS 3      // Chip select pin (T_CS) of touch screen
*/

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
#include "msTimer.h"
#include <SdFat.h>
#include "sdios.h"
#include <list>
#include <vector>
#include <map>
#include "gfxItems.h" // local libray
#include "main.h"
#include "graphicsMethods.h"

#include <MD_MIDIFile.h>

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

// TFT Screen.
// TFT configuration contained in User_Setup.h in the local library.
TFT_eSPI tft = TFT_eSPI();
bool takeTouchReadings = true;
unsigned long touchDebounceMillis = millis();

// SD Card.
const uint8_t SD_SELECT = 8;
SdFat SD;
SdFile dir;
SdFile file;

// General system.
PageId pageId = PageId::Home;
PlayState playState = PlayState::Stop;

// MIDI system.
std::vector<String> midiFiles;
unsigned int selectedFileId = 0;
MD_MIDIFile SMF;
MidiState midiState = MidiState::Idle;
unsigned long midiWaitMillis = 0;

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

const char delimiter = ':';

//////////////////////////////////////////////////////////////////////////////////////////////////////

void SendRestringCommand(int chime, Direction direction)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u:%u:%u\n", int(Commands::Restring), chime, int(direction));
  Serial.print(buffer);
  Serial2.print(buffer);
}

void UpdateMidiInfo(bool updateScreenFlag = true)
{
  // TODO: check vector before attemping access.
  gfxItems.GetGfxItemById(int(GFXItemId::SongTitle)).text = midiFiles[selectedFileId];
  gfxItems.GetGfxItemById(int(GFXItemId::SongNumber)).text = String(String(selectedFileId + 1) + " of " + midiFiles.size());
  if (updateScreenFlag)
  {
    gfxItems.DisplayGfxItem(int(GFXItemId::SongTitle));
    gfxItems.DisplayGfxItem(int(GFXItemId::SongNumber));
  }
}

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
      DisplayCalibrationPage();
    }
  }
}

void ProcessPressedButton(int id)
{
  // All page.
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

  // Home page.
  if ((GFXItemId)id == GFXItemId::Previous)
  {
    playState = PlayState::Stop;
    if (selectedFileId-- == 0)
    {
      selectedFileId = midiFiles.size() - 1;
    }

    UpdateMidiInfo();
  }
  else if ((GFXItemId)id == GFXItemId::Next)
  {

    playState = PlayState::Stop;
    if (selectedFileId++ == midiFiles.size() - 1)
    {
      selectedFileId = 0;
    }
    UpdateMidiInfo();
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

  // Calibration page.
  if ((GFXItemId)id == GFXItemId::Chime_1_up)
  {
    SendRestringCommand(1, Direction::Up);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_1_down)
  {
    SendRestringCommand(1, Direction::Down);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_2_up)
  {
    SendRestringCommand(2, Direction::Up);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_2_down)
  {
    SendRestringCommand(2, Direction::Down);
  }
}

void CheckTouchScreen()
{
  signed id = -1;

  if (millis() - touchDebounceMillis > 250)
  {
    takeTouchReadings = true;
  }

  gfxItems.IsItemInGroupPressed(int(PageId::All), &id);

  if (pageId == PageId::Home)
  {
    gfxItems.IsItemInGroupPressed(int(PageId::Home), &id);
  }
  else if (pageId == PageId::Configuration)
  {
    gfxItems.IsItemInGroupPressed(int(PageId::Configuration), &id);
  }
  else if (pageId == PageId::Calibration)
  {
    gfxItems.IsItemInGroupPressed(int(PageId::Calibration), &id);
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

void midiCallback(midi_event *pev)
{

  //Serial.printf("%u | Track: %u | Channel: %u | Data: ", millis(), pev->track, pev->channel + 1);

  for (uint8_t i = 0; i < pev->size; i++)
  {
    //Serial.print(" ");
    //Serial.print(pev->data[i] , HEX);
    //Serial.printf(" %u" ,pev->data[i]);
  }
  //Serial.println("");
  const char *onOfText[] = {"OFF ", "ON"};

  bool noteState = false;
  int noteId = pev->data[1];

  if (pev->data[0] == 128) // off
  {
    noteState = false;
  }

  if (pev->data[0] == 144) // on
  {
    noteState = true;
  }

  Serial.printf("%u | Track: %u | Channel: %u | %u | %s\n", millis(), pev->track, pev->channel + 1, noteId, onOfText[noteState]);
}

void sysexCallback(sysex_event *pev)
{
  Serial.printf("*** Sysex event | Track %u | Data: ", pev->track);

  for (uint8_t i = 0; i < pev->size; i++)
  {
    Serial.print(pev->data[i]);
  }
  Serial.println("");
}

void tickMetronome()
{
  static uint32_t lastBeatTime = 0;
  static boolean inBeat = false;
  uint16_t beatTime;
  static int beatLed;
  static int beatCounter = 1;

  beatTime = 60000 / SMF.getTempo(); // msec/beat = ((60sec/min)*(1000 ms/sec))/(beats/min)
  if (!inBeat)
  {
    if ((millis() - lastBeatTime) >= beatTime)
    {
      lastBeatTime = millis();
      if (++beatCounter > (SMF.getTimeSignature() & 0xf))
        beatCounter = 1;
      gfxItems.GetGfxItemById(int(GFXItemId::Beat)).fillColor = TFT_CYAN;
      gfxItems.DisplayGfxItem(int(GFXItemId::Beat));

      inBeat = true;
    }
  }
  else
  {
    if ((millis() - lastBeatTime) >= 50) // keep the flash on for 50ms only
    {
      gfxItems.GetGfxItemById(int(GFXItemId::Beat)).fillColor = TFT_SKYBLUE;
      gfxItems.DisplayGfxItem(int(GFXItemId::Beat));
      inBeat = false;
    }
  }

  /*

  static unsigned long lastBeatTime = 0;
  static boolean inBeat = false;
  uint16_t beatTime = 60000 / SMF.getTempo(); // msec/beat = ((60sec/min)*(1000 ms/sec))/(beats/min)

  // TODO: make the beat indicator actually flash on the beat.
  // getTempoAdjust

  inBeat = !inBeat;

  if (!inBeat)
  {
    if ((millis() - lastBeatTime) >= beatTime)
    {
      lastBeatTime = millis();
      gfxItems.GetGfxItemById(int(GFXItemId::Beat)).fillColor = TFT_CYAN;
      gfxItems.DisplayGfxItem(int(GFXItemId::Beat));
    }
  }
  else
  {
    if ((millis() - lastBeatTime) >= 100)
    {
      gfxItems.GetGfxItemById(int(GFXItemId::Beat)).fillColor = TFT_SKYBLUE;
      gfxItems.DisplayGfxItem(int(GFXItemId::Beat));
    }
  }
  */
}

void ProcessMIDI()
{
  if (midiState == MidiState::Idle)
  {
    if (playState == PlayState::Play)
    {
      midiState = MidiState::Load;
    }
  }
  else if (midiState == MidiState::Load)
  {
    Serial.printf("Midi: loading file: %s (%u)\n", midiFiles[selectedFileId].c_str(), selectedFileId);

    int errorCode = SMF.load(midiFiles[selectedFileId].c_str());
    if (errorCode != MD_MIDIFile::E_OK)
    {
      Serial.printf("Midi: Error when loading file: %u\n", errorCode);
      midiWaitMillis = millis();
      midiState = MidiState::Wait;
    }
    else
    {
      midiState = MidiState::Play;
    }
  }
  else if (midiState == MidiState::Play)
  {
    if (!SMF.isEOF())
    {
      if (SMF.getNextEvent())
      {
        tickMetronome();
      }
    }
    else
    {
      playState = PlayState::Stop;
      midiState = MidiState::Stop;
    }
  }
  else if (midiState == MidiState::Stop)
  {
    SMF.close();
    midiState = MidiState::Idle;
  }
  else if (midiState == MidiState::Wait)
  {
    if (millis() - midiWaitMillis > 2000)
    {
      midiState = MidiState::Idle;
    }
  }
}

void setup(void)
{
  delay(2000);
  Serial.begin(115200);
  Serial.println("Melodic Chimes starting up.");

  Serial2.begin(115200);

  ScreenInit();

  //SDInit();

  // Initialize MIDIFile
  //SMF.begin(&SD);
  //SMF.setMidiHandler(midiCallback);
  //SMF.setSysexHandler(sysexCallback);

  InitScreenElements();
  //UpdateMidiInfo(false);
  DisplayMain();
  DisplayHomePage();
 
}

void loop()
{
  CheckTouchScreen();

  UpdateScreen();

  //ProcessMIDI();
}