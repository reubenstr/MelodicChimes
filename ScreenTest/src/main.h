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
  MainState,
  Time,
  SongTitle,
  SongLength,
  SongNumber
};

enum class Justification
{
  Left,
  Center,
  Right
};

enum PageId
{
  HomePage,
  CalibrationPage,
  ConfigurationPage
};

enum class PlayState
{
  Play,
  Pause,
  Stop
};

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

struct Boundry
{
  int x, y, w, h;

  Boundry()
  {
  }

  Boundry(int x, int y, int w, int h)
  {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }
};

struct Button
{
  String text;
  ButtonId buttonId;
  Boundry boundry;

  uint32_t textColor = TFT_BLACK;
  uint32_t fillColor = TFT_GREEN;
  uint32_t activeColor = TFT_WHITE;
  uint32_t borderColor = TFT_DARKGREEN;

  Button(ButtonId buttonId, String text, Boundry boundry, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
  {
    this->text = text;
    this->buttonId = buttonId;
    this->boundry = boundry;
    this->textColor = textColor;
    this->fillColor = fillColor;
    this->activeColor = activeColor;
    this->borderColor = borderColor;
  }
};

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