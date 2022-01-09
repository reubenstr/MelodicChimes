/*  
  Project:
    Melodic Chimes
  
  Description: 
    Three auto-tuning strings playing music from custom MIDI files.
	  Fetch network time via WiFi for on the hour chime.
	
  Microcontrollers:
      1x ESP32 (DevkitV1): Main controller parsing MIDI files and fetching time from WiFi.
      [External] 2x Teensy 3.2: Chime controllers detecting string frequency and driving chime actions.
 
  Hardware:
      TFT Touch Screen: User interface.
      SD-Card: contains MIDI files.
      9x Stepper Motors: 3x tuning, 3x volume, 3x picking.    
      
  Compilation Notes:
    MD_MIDIFile.h line 899 fix:
        void setFileFolder(const char* apath) { if (apath != nullptr) _sd->chdir(apath); }
		
  Online Tools:
    MIDI Creation: https://musescore.org/en
	
  Bugs:
    bootup fails when SD card does not have a .mid file. -> provide error message
	
  TODO / Possible Upgrades:
    Receive feedback from chime controllers to check if strings are OK (in range).
    MIDI playback progress bar.
	  Rebuild JSON parameters file if the file is missing or contains bad JSON.
    Move TFT Pins to build flags (like quotebot)
    Spagetti TFT graphics drawing code could have better seperation of concerns.
*/

/*
// TFT LCD Display pins defined in User_Setup.h within the TFT_eSPI library folder.
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

#include <Arduino.h>
#include <map>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Free_Fonts.h>
#include <SdFat.h>
#include <sdios.h>
#include <MD_MIDIFile.h>
#include <Adafruit_NeoPixel.h>
#include <main.h>
#include <gfxItems.h>
#include <graphicsMethods.h>
#include <NeoPixelMethods.h>
#include <tftMethods.h>
#include <utilities.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#define PIN_UART1_TX 33
#define PIN_UART1_RX 25
#define PIN_UART2_TX 26
#define PIN_UART2_RX 27
#define PIN_NEOPIXEL 32

/*
TFT LCD pin config are found in TFT_eSPI library in User_Setup.h
#define ILI9488_DRIVER 
#define TFT_WIDTH  480
#define TFT_HEIGHT 320
#define TFT_CS 5
#define TFT_DC 2
#define TFT_SCLK 18
#define TFT_MOSI 23
#define TFT_MISO 19
#define TFT_RST 4
#define TOUCH_CS 22   
*/

// TFT Screen.
TFT_eSPI tft = TFT_eSPI();
bool takeTouchReadings = true;
GFXItems gfxItems(&tft);

// SD Card.
const uint8_t PIN_SD_CHIP_SELECT = 15;
SdFat SD;
SdFile dir;
SdFile file;
const char *parametersFilePath = "/parameters.json";

// General system.
PageId pageId = PageId::Home;
PlayState playState = PlayState::Idle;

// MIDI system.
std::vector<String> midiFiles;
unsigned int selectedFileId = 0;
MD_MIDIFile SMF;
unsigned long midiWaitMillis = 0;

// Nameplate LEDs
Adafruit_NeoPixel strip(9, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Parameters
Parameters parameters;
Status status;
System sys;

// Wi-Fi
TaskHandle_t taskWifi;

//////////////////////////////////////////////////////////////////////////////////////////////////////

void SendCommand(Commands command, int chime)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u:%u\n", int(command), chime);
  Serial.printf("Sending command: %s", buffer);
  Serial1.print(buffer);
  Serial2.print(buffer);
}

void SendTuneCommand(Commands command, int chime, int nodeId, bool vibrato = false)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u:%u:%u:%u\n", int(command), chime, nodeId, int(vibrato));
  Serial.printf("Sending command: %s", buffer);
  Serial1.print(buffer);
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

void ProcessIndicators(bool forceUpdate = false)
{
  static Status previousStatus;
  static int previousMinute;
  char buf[12];
  const int y = 296;

  static PlayState previousPlayState = PlayState::Default;
  if (previousPlayState != playState)
  {
    previousPlayState = playState;
    DisplayIndicator(String(playStateText[int(playState)]), 50, y, playStateFillColor[int(playState)]);
  }

  if (previousStatus != status || forceUpdate)
  {
    previousStatus = status;
    DisplayIndicator("BEAT", 120, y, status.beat ? TFT_CYAN : TFT_RED);  
    DisplayIndicator("3", 185, y, status.chime3Enabled ? TFT_GREEN : TFT_YELLOW);
    DisplayIndicator("2", 210, y, status.chime2Enabled ? TFT_GREEN : TFT_YELLOW);
    DisplayIndicator("1", 235, y, status.chime1Enabled ? TFT_GREEN : TFT_YELLOW);
    DisplayIndicator("SD", 295, y, status.sd ? TFT_GREEN : TFT_RED); 
     DisplayIndicator("WIFI", 347, y, status.wifi ? TFT_GREEN : TFT_RED);
  }

  if (previousMinute != sys.time.currentTimeInfo.tm_min)
  {
    previousMinute = sys.time.currentTimeInfo.tm_min;
    sprintf(buf, "%02u:%02u", sys.time.currentTimeInfo.tm_hour, sys.time.currentTimeInfo.tm_min);
    DisplayIndicator(String(buf), 420, y, status.time ? TFT_GREEN : TFT_RED);
  }
}

void ProcessDisplay()
{
  static PageId previousPageId = PageId::Volume;
  if (previousPageId != pageId)
  {
    previousPageId = pageId;

    DisplayClearPartial();
    gfxItems.DisplayGroup(int(pageId));

    if (pageId == PageId::Home)
    {
      DisplaySongBorder();
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
    if (playState == PlayState::Idle)
    {
      SendCommand(Commands::Enable, 0);
      SendCommand(Commands::Enable, 1);
      SendCommand(Commands::Enable, 2);
      playState = PlayState::Load;
    }
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
    SendCommand(Commands::Tighten, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_1_down)
  {
    SendCommand(Commands::Loosen, 0);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_2_up)
  {
    SendCommand(Commands::Tighten, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_2_down)
  {
    SendCommand(Commands::Loosen, 1);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_3_up)
  {
    SendCommand(Commands::Tighten, 2);
  }
  else if ((GFXItemId)id == GFXItemId::Chime_3_down)
  {
    SendCommand(Commands::Loosen, 2);
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

void ProcessTouchScreen()
{
  static unsigned long start = millis();
  signed id = -1;

  if (millis() - start > 250)
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
      start = millis();
    }
  }
}

bool SDCardInit()
{
  int count = 0;

  Serial.println("SD: Attempting to mount SD card...");

  while (!SD.begin(PIN_SD_CHIP_SELECT, SPI_HALF_SPEED))
  {
    if (++count > 5)
    {
      Serial.println("SD: Card Mount Failed.");
      return false;
    }
    delay(100);
  }

  Serial.println("SD: SD card mounted.");
  return true;
}

bool FetchMidiFiles()
{
  Serial.println("SD Card: fetching .mid files.");

  if (!dir.open("/"))
  {
    Serial.println("SD Card: dir.open() failed!");
    return false;
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
    return false;
  }
  else
  {
    Serial.printf("SD Card: %u .mid files found.\n", midiFiles.size());
  }

  if (midiFiles.size() == 0)
  {
    return false;
  }

  for (auto &s : midiFiles)
  {
    Serial.println(s);
  }
  return true;
}

bool FetchParametersFromSDCard()
{
  Serial.printf("SD: Attempting to fetch parameters from %s...\n", parametersFilePath);

  if (!file.open(parametersFilePath, O_RDONLY))
  {
    Serial.printf("SD: Failed to open file: %s\n", parametersFilePath);
    file.close();
    return false;
  }

  String str;
  while (file.available())
  {
    str += (char)file.read();
  }

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, str);

  if (error)
  {
    Serial.print(F("JSON: DeserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }

  for (int i = 0; i < doc["wifiCredentials"].size(); i++)
  {
    WifiCredentials wC;
    wC.ssid = doc["wifiCredentials"][i]["ssid"].as<String>();
    wC.password = doc["wifiCredentials"][i]["password"].as<String>();
    parameters.wifiCredentials.push_back(wC);
  }

  sys.time.timeZone = doc["system"]["timeZone"].as<String>();

  file.close();

  Serial.printf("SD: parameters fetched.\n");
  return true;
}

void TriggerLEDs(int chimeId, int noteId)
{
  uint32_t color = Wheel(map(noteId, 50, 69, 85, 255));
  for (int i = 0; i < 3; i++)
  {
    strip.setPixelColor(i + chimeId * 3, color);
  }
}

void ProcessNameplate()
{
  static byte position;
  static unsigned long start = millis();

  if (playState == PlayState::Idle)
  {
    if (millis() - start > sys.delayMsNameplateLEDRevolve)
    {
      start = millis();
      position++;
      for (int i = 0; i < strip.numPixels(); i++)
      {
        byte input = (255 / strip.numPixels()) * i + position;
        strip.setPixelColor(i, Wheel(input));
      }
      strip.setBrightness(sys.nameplateLEDBrightnessRevolve);
      strip.show();
    }
  }
  else if (playState == PlayState::Play)
  {
    if (millis() - start > sys.delayMsNameplateLEDFade)
    {
      start = millis();
      for (int i = 0; i < strip.numPixels(); i++)
      {
        strip.setPixelColor(i, Fade(strip.getPixelColor(i), 4));
      }
      strip.setBrightness(sys.nameplateLEDBrightnessFade);
      strip.show();
    }
  }
}

void midiCallback(midi_event *pev)
{
  const char *onOffText[] = {"OFF", "ON "};
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

  Serial.printf("(%8u) | Track: %u | Channel: %u | Command: %s | NoteID: %3u | Velocity: %3u | Data: [", millis(), track, channel, onOffText[noteState], noteId, velocity);

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

  if (noteState == true && velocity > 0)
  {
    SendTuneCommand(Commands::SetTargetNote, channel, noteId);
    SendCommand(Commands::Pick, channel);
    TriggerLEDs(channel, noteId);
  }
  else if (noteState == false || velocity == 0)
  {
    // Do nothing, chimes will fade in volume naturally.
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
  uint16_t beatTime;
  static int beatCounter = 1;

  beatTime = 60000 / SMF.getTempo(); // msec/beat = ((60sec/min)*(1000 ms/sec))/(beats/min)
  if (!status.beat)
  {
    if ((millis() - lastBeatTime) >= beatTime)
    {
      lastBeatTime = millis();
      if (++beatCounter > (SMF.getTimeSignature() & 0xf))
      {
        beatCounter = 1;
      }
      status.beat = true;
    }
  }
  else
  {
    if ((millis() - lastBeatTime) >= 50)
    {
      status.beat = false;
    }
  }
}

void ProcessMIDI()
{
  static unsigned long startPreload;

  if (playState == PlayState::Idle)
  {
    // Do nothing.

    startPreload = millis();
  }
  else if (playState == PlayState::PreLoad)
  {
    if (millis() - startPreload > sys.delayMsAfterPlayToEnableChimes)
    {
      playState = PlayState::Load;
    }
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
    //TODO: create random notes within a from a musical key.
  }
}

void ProcessTime()
{
  static unsigned long start = millis();

  if (millis() - start > sys.delayMsBetweenFetchTime)
  {
    start = millis();

    if (!getLocalTime(&sys.time.currentTimeInfo))
    {
      Serial.println("TIME: Failed to obtain time");
      status.time = false;
    }

    status.time = true;
    time(&sys.time.currentEpoch); // Fetch current time as epoch
  }
}

void WifiTask(void *pvParameters)
{
  while (1)
  {
    ProcessTime();

    static unsigned long start = millis();
    static int wifiCredentialsIndex = -1;
    bool previousStatus = status.wifi;

    status.wifi = WiFi.status() == WL_CONNECTED;

    if (status.wifi == true)
    {
      start = millis();

      if (previousStatus != status.wifi)
      {
        Serial.printf("WIFI: WiFi connected to %s, device IP: %s\n", parameters.wifiCredentials[wifiCredentialsIndex].ssid.c_str(), WiFi.localIP().toString().c_str());
      }
    }

    if (millis() - start > sys.delayMsBetweenWifiScan)
    {
      if (++wifiCredentialsIndex > parameters.wifiCredentials.size() - 1)
      {
        wifiCredentialsIndex = 0;
      }

      Serial.printf("WIFI: Attemping to connect to %s : %s\n", parameters.wifiCredentials[wifiCredentialsIndex].ssid.c_str(), parameters.wifiCredentials[wifiCredentialsIndex].password.c_str());
      WiFi.begin(parameters.wifiCredentials[wifiCredentialsIndex].ssid.c_str(), parameters.wifiCredentials[wifiCredentialsIndex].password.c_str());
    }
  }
}

void ProcessCommand(String command)
{
  int commandInt = getValue(command, delimiter, 0).toInt();
  int chimeId = getValue(command, delimiter, 1).toInt();

  if (commandInt == int(Commands::StatusDisabled))
  {
    Serial.printf("Command received: chime %u disabled.\n", chimeId);
    if (chimeId == 0)
      status.chime1Enabled = false;
    if (chimeId == 1)
      status.chime2Enabled = false;
    if (chimeId == 2)
      status.chime3Enabled = false;
  }
  else if (commandInt == int(Commands::StatusEnabled))
  {
    Serial.printf("Received command: chime %u enabled.\n", chimeId);
    if (chimeId == 0)
      status.chime1Enabled = true;
    if (chimeId == 1)
      status.chime2Enabled = true;
    if (chimeId == 2)
      status.chime3Enabled = true;
  }
  else if (commandInt == int(Commands::Error))
  {
    Serial.printf("Received command: error on chime %u.\n", chimeId);
    DisplayError(ErrorCodes::chimeFailed);
  }
}

void ProcessUart()
{
  static String uartData1 = "";
  static String uartData2 = "";

  while (Serial1.available() > 0)
  {
    char readChar = Serial1.read();
    uartData1 += (char)readChar;

    if (readChar == '\n')
    {
      ProcessCommand(uartData1);
      uartData1 = "";
    }

    if (uartData1.length() > 64)
    {
      uartData1 = "";
    }
  }

   while (Serial2.available() > 0)
  {
    char readChar = Serial2.read();
    uartData2 += (char)readChar;

    if (readChar == '\n')
    {
      ProcessCommand(uartData2);
      uartData2 = "";
    }

    if (uartData2.length() > 64)
    {
      uartData2 = "";
    }
  }
}

void setup(void)
{
  strip.begin();
  strip.show();

  delay(1000);
  Serial.begin(115200);
  Serial.println("Melodic Chimes starting up...");

  Serial1.begin(115200, SERIAL_8N1, PIN_UART1_RX, PIN_UART1_TX);
  Serial2.begin(115200, SERIAL_8N1, PIN_UART2_RX, PIN_UART2_TX);

  DisplayInit();
  CheckTouchCalibration(&tft, false);
  DisplaySplash();

  if (!SDCardInit())
  {
    DisplayError(ErrorCodes::sdCardInitFailed);
  }
  else
  {
    status.sd = true;
  }

  if (!FetchMidiFiles())
  {
    DisplayError(ErrorCodes::midiFilesNotFound);
  }

  if (!FetchParametersFromSDCard())
  {
    DisplayError(ErrorCodes::parametersFileFailed);
  }

  configTime(sys.time.gmtOffset_sec, sys.time.daylightOffset_sec, sys.time.ntpServer);

  DisplayElementsInit();
  UpdateMidiInfo(false);
  DisplayMain();
  ProcessIndicators(true);

  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setSysexHandler(sysexCallback);

  xTaskCreatePinnedToCore(
      WifiTask,       /* Task function. */
      "WifiTask",     /* name of task. */
      10000,         /* Stack size of task */
      NULL,          /* parameter of the task */
      1,             /* priority of the task */
      &taskWifi,     /* Task handle to keep track of created task */
      1);            /* pin task to core 1 */
}

void loop()
{
  ProcessUart();

  ProcessTouchScreen();

  ProcessDisplay();

  ProcessMIDI();

  ProcessIndicators();

  ProcessNameplate();
}