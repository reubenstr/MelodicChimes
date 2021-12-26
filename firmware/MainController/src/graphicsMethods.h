enum class ErrorCodes
{
  sdCardInitFailed,
  midiNotFound,
  chimeFailed
};


void DisplayClearPartial();
void DisplayMain();
void InitScreenElements();
void DisplayError(ErrorCodes error);
void ScreenInit();