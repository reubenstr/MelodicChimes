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

  // Calibration
  Chime_1_down,
  Chime_1_up,
  Chime_2_down,
  Chime_2_up,
   Chime_3_down,
  Chime_3_up,
  Chime_4_down,
  Chime_4_up
  
};

enum class PageId
{
  All,
  Home,
  Configuration,
    Calibration
};

enum class MidiState
{
  Idle, 
  Load,
  Play,
  Stop,
  Wait
};

enum class PlayState
{
  Default,
  Play,
  Pause, 
  Stop
};

static const char *playStateText[4] = {"", "PLAY ", "PAUSE", "STOP "};
const uint32_t playStateFillColor[4] = {TFT_BLACK, TFT_GREEN, TFT_YELLOW, TFT_RED};

enum class Commands
{
  Restring,
  Tune
};

enum class Direction
{ 
  Down,
  Up
};



#endif