enum class ButtonId
{
  Home,
  Calibration,
  Configuration,
  Default,
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
  Startup
};

enum class LabelId
{
  Default,
  PlayState,
  Clock,
  SongTitle,
  SongLength,
  SongNumber,
  SD,
  Wifi,
  Time
   
};

/*
enum class Justification
{
  Left,
  Center,
  Right
};
*/

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

const char * playStateText[4] = {"", "Play ", "Pause", "Stop "};
const uint32_t playStateFillColor[4] = {TFT_BLACK, TFT_GREEN, TFT_YELLOW, TFT_RED};

struct StatusItem
{
  String text;
  bool status;

  StatusItem(String text, bool status)
  {
    this->text = text;
    this->status = status;
  }
};





/*
struct Label
{
  String text;
  LabelId id;
  uint32_t textColor = TFT_BLACK;
  int size;
  int x;
  int y;
  Justification justification;

  Label(LabelId id, String text, int size, int x, int y, uint32_t textColor, Justification justification)
  {
    this->id = id;
    this->text = text;
    this->size = size;
    this->x = x;
    this->y = y;
    this->textColor = textColor;
    this->justification = justification;
  }
};
*/