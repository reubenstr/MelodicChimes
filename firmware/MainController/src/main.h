#ifndef MAIN_H
#define MAIN_H

#include <vector>

enum class GFXItemId
{
  Default,

  Home,
  Calibration,
  Configuration,

  Previous,
  Next,
  Play,
  Pause,
  Stop,
  Hourly,
  StartHour,
  EndHour,
  TimeZone,
  Song,
  Startup,

  // Labels.
  PlayState,
  Beat,
  Clock,
  SD,
  Wifi,
  Time,

  SongTitle,
  SongLength,
  SongNumber,

  // Calibration.
  Chime_1_down,
  Chime_1_up,
  Chime_2_down,
  Chime_2_up,
  Chime_3_down,
  Chime_3_up,

  // Development.
  Chime1mute,
  Chime2mute,
  Chime3mute,
  Chime1pick,
  Chime2pick,
  Chime3pick,

  // Volume
  Chime1VolumePlus,
  Chime2VolumePlus,
  Chime3VolumePlus,
  Chime1VolumeMinus,
  Chime2VolumeMinus,
  Chime3VolumeMinus,
};

enum class PageId
{
  All,
  Home,
  Configuration,
  Restring,
  Volume
};

enum class PlayState
{
  Default,
  Idle,
  PreLoad,
  Load,
  Play,
  Pause,
  Stop,
  Random
};

// Order of text and colors need to match order of PlayState.
static const char *playStateText[] = {"", "IDLE ", "PRE  ", "LOAD ", "PLAY ", "PAUSE", "STOP "};
const uint32_t playStateFillColor[] = {TFT_BLACK, TFT_SKYBLUE, TFT_MAGENTA, TFT_BLUE, TFT_GREEN, TFT_YELLOW, TFT_RED};

const char delimiter = ':';

enum class Commands
{
  Enable,
  Tighten,
  Loosen,
  VolumePlus,
  VolumeMinus,
  SetTargetNote,
  Pick,
  StatusEnabled,
  StatusDisabled,
  Error
};

struct WifiCredentials
{
  String ssid;
  String password;
};

struct Time
{
  struct tm currentTimeInfo;
  time_t currentEpoch;
  const char *ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = -5 * 60 * 60;
  const int daylightOffset_sec = 3600;
  String timeZone;
};

struct Parameters
{
  std::vector<WifiCredentials> wifiCredentials;
};

struct Status
{
  bool beat;
  bool wifi;
  bool sd;
  bool time;
  bool chime1Enabled;
  bool chime2Enabled;
  bool chime3Enabled;

  bool operator!=(Status const &s)
  {
    return (beat != s.beat ||
            wifi != s.wifi ||
            sd != s.sd ||
            time != s.time ||
            chime1Enabled != s.chime1Enabled ||
            chime2Enabled != s.chime2Enabled ||
            chime3Enabled != s.chime3Enabled);
  }
};

struct System
{
  Time time;

  const unsigned long delayMsBetweenWifiScan = 3000;
  const int delayMsBetweenFetchTime = 250;
  const int delayMsNameplateLEDFade = 10;
  const int delayMsNameplateLEDRevolve = 15;
  const char delimiter = ':';

  const int nameplateLEDBrightnessFade = 255;
  const int nameplateLEDBrightnessRevolve = 127;

  const unsigned long delayMsAfterPlayToEnableChimes = 500;
};

#endif