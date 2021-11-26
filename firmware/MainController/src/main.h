#ifndef MAIN_H
#define MAIN_H



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
  Chime_4_down,
  Chime_4_up,

  // Development.
  Chime1mute,
  Chime2mute,
  Chime3mute,
  Chime4mute,
  Chime1pick,
  Chime2pick,
  Chime3pick,
  Chime4pick,

};

enum class PageId
{
  All,
  Home,
  Configuration,
  Calibration,
  Development
};

enum class PlayState
{
  Default,
  Idle,
  Load,
  Play,
  Pause,
  Stop
};

// Order of text and colors need to match order of PlayState.
static const char *playStateText[6] = {"", "IDLE", "LOAD", "PLAY ", "PAUSE", "STOP "};
const uint32_t playStateFillColor[6] = {TFT_BLACK, TFT_SKYBLUE, TFT_BLUE, TFT_GREEN, TFT_YELLOW, TFT_RED};

enum class Commands
{
    RestringTighten,
    RestringLoosen,
    SetTargetNote,
    PretuneNote,
    Mute,
    Pick
};



#endif