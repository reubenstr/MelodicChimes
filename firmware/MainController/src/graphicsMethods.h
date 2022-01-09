enum class ErrorCodes
{
  sdCardInitFailed,
  midiFilesNotFound,
  parametersFileFailed,
  chimeFailed
};

void DisplayClearPartial();
void DisplayMain();
void DisplayElementsInit();
void DisplayError(ErrorCodes error);
void DisplayInit();
void DisplaySplash();
void DisplayIndicator(String string, int x, int y, uint16_t color);