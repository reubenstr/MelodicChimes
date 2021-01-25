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
  Clock,
  SD,
  Wifi,
  Time,
  SongTitle,
  SongLength,
  SongNumber
};

enum class PageId
{
  All,
  Home,
  Calibration,
  Configuration
};

enum class PlayState
{
  Default,
  Play,
  Pause,
  Stop
};

static const char *playStateText[4] = {"", "Play ", "Pause", "Stop "};
const uint32_t playStateFillColor[4] = {TFT_BLACK, TFT_GREEN, TFT_YELLOW, TFT_RED};

#endif