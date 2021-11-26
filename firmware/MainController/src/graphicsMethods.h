

enum class ErrorCodes
{
  sdCardInitFailed,
  midiNotFound
};


void DisplayClearPartial();
void DisplayMain();
void InitScreenElements();
void DisplayError(ErrorCodes error);
void ScreenInit();