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

  Bugs:
    bootup fails when SD card does not have a .mid file. -> provide error message

  Future Features:
    MIDI playback progress bar.


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
#include <Adafruit_NeoPixel.h>
#include <NeoPixelMethods.h>

#define PIN_UART1_TX 33
#define PIN_UART1_RX 25
#define PIN_UART2_TX 26
#define PIN_UART2_RX 27
#define PIN_NEOPIXEL 32

// TFT Screen.
// TFT configuration contained in User_Setup.h in the local library.
TFT_eSPI tft = TFT_eSPI();
bool takeTouchReadings = true;
unsigned long touchDebounceMillis = millis();

// SD Card.
const uint8_t SD_SELECT = 15;
SdFat32 SD;
SdFile dir;
SdFile file;

// General system.
PageId pageId = PageId::Home;
PlayState playState = PlayState::Idle;

// MIDI system.
std::vector<String> midiFiles;
unsigned int selectedFileId = 0;
MD_MIDIFile SMF;
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

Adafruit_NeoPixel strip(9, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

//////////////////////////////////////////////////////////////////////////////////////////////////////

void SendCommand(Commands command, int chime)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u:%u\n", int(command), chime);
  Serial.printf("Sending command: %s", buffer);
  Serial1.print(buffer);
  Serial2.print(buffer);
}

void SendCommandString(String commandString)
{
  Serial.printf("Sending command: %s", commandString);
  Serial1.print(commandString);
  Serial2.print(commandString);
}

String CreateCommandString(Commands command, int chime)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u:%u\n", int(command), chime);
  return String(buffer);
}

String CreateTuneCommandString(Commands command, int chime, int nodeId, bool vibrato = false)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u:%u:%u:%u\n", int(command), chime, nodeId, int(vibrato));
  return String(buffer);
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

  static PageId previousPageId = PageId::Volume;
  if (previousPageId != pageId)
  {
    previousPageId = pageId;

    DisplayClearPartial();
    gfxItems.DisplayGroup(int(pageId));

    if (pageId == PageId::Home)
    {
      // Song border.
      tft.drawRect(20, 70, 440, 40, TFT_LIGHTGREY);
      // Progress bar.
      // tft.drawRect(20, 120, 440, 15, TFT_LIGHTGREY);
    }
  }
}

void ProcessPressedButton(int id)
{
  // All pages.
  if ((GFXItemId)id == GFXItemId::Home)
  {
    pageId = PageId::Home;
  }
  else if ((GFXItemId)id == GFXItemId::Configuration)
  {
    pageId = PageId::Configuration;
  }
  else if ((GFXItemId)id == GFXItemId::Calibration)
  {
    static bool pageToggle = false;
    pageToggle = !pageToggle;
    pageId = pageToggle ? PageId::Restring : PageId::Volume;
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
    //playState = PlayState::Random;
    playState = PlayState::Load;
  }
  else if ((GFXItemId)id == GFXItemId::Pause)
  {
    playState = PlayState::Pause;
  }
  else if ((GFXItemId)id == GFXItemId::Stop)
  {
    playState = PlayState::Stop;
  }

  // Restring page.
  if ((GFXItemId)id == GFXItemId::Chime_1_up)
  {
    SendCommand(Commands::RestringTighten, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_1_down)
  {
    SendCommand(Commands::RestringLoosen, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_2_up)
  {
    SendCommand(Commands::RestringTighten, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_2_down)
  {
    SendCommand(Commands::RestringLoosen, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_3_up)
  {
    SendCommand(Commands::RestringTighten, 2);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_3_down)
  {
    SendCommand(Commands::RestringLoosen, 2);
  }

  // Volume page.
  if ((GFXItemId)id == GFXItemId::Chime1VolumePlus)
  {
    SendCommand(Commands::VolumePlus, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime2VolumePlus)
  {
    SendCommand(Commands::VolumePlus, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime3VolumePlus)
  {
    SendCommand(Commands::VolumePlus, 2);
  }
  else if ((GFXItemId)id == GFXItemId::Chime1pick)
  {
    SendCommand(Commands::Pick, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime2pick)
  {
    SendCommand(Commands::Pick, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime3pick)
  {
    SendCommand(Commands::Pick, 2);
  }
  if ((GFXItemId)id == GFXItemId::Chime1VolumeMinus)
  {
    SendCommand(Commands::VolumeMinus, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime2VolumeMinus)
  {
    SendCommand(Commands::VolumeMinus, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime3VolumeMinus)
  {
    SendCommand(Commands::VolumeMinus, 2);
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

  gfxItems.IsItemInGroupPressed(int(pageId), &id);

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
  if (!SD.begin(SD_SELECT, SPI_HALF_SPEED))
  {
    Serial.println("SD Card: init failed!");
    DisplayError(ErrorCodes::sdCardInitFailed);
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

  if (midiFiles.size() == 0)
  {
    DisplayError(ErrorCodes::sdCardInitFailed);
  }

  for (auto &s : midiFiles)
  {
    Serial.println(s);
  }
}

void TriggerLEDs(int chimeId, int noteId)
{
  uint32_t color = Wheel(map(noteId, 50, 69, 0, 255));
  for (int i = 0; i < 3; i++)
  {
    strip.setPixelColor(i + chimeId * 3, color);
  }
}

void FadeLEDs()
{
  static unsigned long start = millis();
  if (millis() - start > 10)
  {
    start = millis();
    for (int i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Fade(strip.getPixelColor(i), 4));
    }
  }
  strip.show();
}

void NameplateLEDs()
{
  static bool flag = true;

  if (playState == PlayState::Idle && flag)
  {
    flag = false;
    strip.setPixelColor(0, 255, 0, 0);
    strip.setPixelColor(1, 255, 0, 0);
    strip.setPixelColor(2, 255, 0, 0);
    strip.setPixelColor(3, 0, 255, 0);
    strip.setPixelColor(4, 0, 255, 0);
    strip.setPixelColor(5, 0, 255, 0);
    strip.setPixelColor(6, 0, 0, 255);
    strip.setPixelColor(7, 0, 0, 255);
    strip.setPixelColor(8, 0, 0, 255);
    strip.show();
  }
  else if (playState == PlayState::Play)
  {
    flag = true;
    FadeLEDs();
  }
}

void midiCallback(midi_event *pev)
{
  const char *onOfText[] = {"OFF", "ON "};
  bool noteState = false;

  if (pev->size < 3)
  {
    return;
  }

  int track = pev->track;
  int channel = pev->channel;
  int size = pev->size;
  int command = pev->data[0];
  int noteId = pev->data[1];
  int velocity = pev->data[2];

  if (command == 128) // off
  {
    noteState = false;
  }
  else if (command == 144) // on
  {
    noteState = true;
  }

  Serial.printf("(%8u) | Track: %u | Channel: %u | Command: %s | NoteID: %3u | Velocity: %3u | Data: [", millis(), track, channel, onOfText[noteState], noteId, velocity);

  for (uint8_t i = 0; i < size; i++)
  {
    //Serial.print(pev->data[i] , HEX);
    Serial.printf("%3u", pev->data[i]);
    if (i < size - 1)
    {
      Serial.print(",");
    }
  }
  Serial.println("]");

  // Play note.
  if (noteState == true && velocity > 0)
  {
    SendCommandString(CreateTuneCommandString(Commands::SetTargetNote, channel, noteId));
    SendCommand(Commands::Pick, channel);
    TriggerLEDs(channel, noteId);
  }

  // End note.
  if (noteState == false || velocity == 0)
  {
    // Do nothing.
  }
}

// Unlikley to occur or even needed, therefore only display message for development purposes.
void sysexCallback(sysex_event *pev)
{
  Serial.printf("*** Sysex event | Track %u | Data: ", pev->track);

  for (uint8_t i = 0; i < pev->size; i++)
  {
    Serial.printf(" %u", pev->data[i]);
  }
  Serial.println();
}

// TODO: verify this is a real-time metronome.
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
      {
        beatCounter = 1;
      }
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
}

void ProcessMIDI()
{
  if (playState == PlayState::Idle)
  {
    // Do nothing.
  }
  else if (playState == PlayState::Load)
  {
    Serial.printf("Midi: loading file: %s (%u)\n", midiFiles[selectedFileId].c_str(), selectedFileId);

    int errorCode = SMF.load(midiFiles[selectedFileId].c_str());
    if (errorCode != MD_MIDIFile::E_OK)
    {
      Serial.printf("Midi: Error when loading file: %u\n", errorCode);
      midiWaitMillis = millis();
      playState = PlayState::Idle;
    }
    else
    {
      playState = PlayState::Play;
    }
  }
  else if (playState == PlayState::Play)
  {
    if (SMF.isEOF())
    {
      playState = PlayState::Stop;
    }
    else
    {
      if (SMF.getNextEvent())
      {
        tickMetronome();
      }
    }
  }
  else if (playState == PlayState::Stop)
  {
    SMF.close();
    playState = PlayState::Idle;
  }
  else if (playState == PlayState::Random)
  {
    //TODO
  }
}

void setup(void)
{

  strip.begin();
  strip.show();

  delay(1000);
  Serial.begin(115200);
  Serial.println("Melodic Chimes starting up.");

  Serial1.begin(115200, SERIAL_8N1, PIN_UART1_RX, PIN_UART1_TX);
  Serial2.begin(115200, SERIAL_8N1, PIN_UART2_RX, PIN_UART2_TX);

  ScreenInit();

  SDInit();

  // Initialize MIDIFile
  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setSysexHandler(sysexCallback);

  InitScreenElements();
  UpdateMidiInfo(false);
  DisplayMain();
}

void loop()
{
  CheckTouchScreen();

  UpdateScreen();

  ProcessMIDI();

  NameplateLEDs();
}