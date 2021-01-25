#include "buttons.h"
#include <vector>
#include "Free_Fonts.h"
#include <TFT_eSPI.h>

Buttons::Buttons(TFT_eSPI *tft)
{
  this->tft = tft;
}

void Buttons::Add(int key, Button button)
{
  // Check if key exists.
  if (buttons.find(key) == buttons.end())
  {
    // Key not found.
    std::vector<Button> newVector{button};
    buttons.insert(std::make_pair(key, newVector));
  }
  else
  {
    // Key found.
    buttons[key].push_back(button);
  }
}

void Buttons::DisplayButtons(int key)
{
  for (auto const &b : buttons[key])
  {
    tft->setFreeFont(FF22);
    tft->setTextColor(b.textColor);
    tft->setTextSize(1);
    int borderThickness = 1;
    tft->fillRoundRect(b.x, b.y, b.w, b.h, 6, b.borderColor);
    tft->fillRoundRect(b.x + borderThickness, b.y + borderThickness, b.w - borderThickness * 2, b.h - borderThickness * 2, 5, b.fillColor);

    int textWidth = tft->textWidth(b.text);
    int textHeight = tft->fontHeight();
    tft->drawString(b.text, b.x + (b.w - textWidth) / 2 - 1, b.y + (b.h - textHeight) / 2 + borderThickness * 2);
  }
}

bool Buttons::IsButtonPressed(int key, int *id)
{
  uint16_t x, y;
  if (tft->getTouch(&x, &y, 20))
  {
    for (auto const &b : buttons[key])
    {
      if (x >= b.x && x <= (b.x + b.w))
      {
        if (y >= b.y && y <= (b.y + b.h))
        {
          *id = b.id;
          return true;
        }
      }
    }
  }

  return false;
}

/*
bool Buttons::IsButtonPressed(Button b, int x, int y)
{
  if (x >= b.boundry.x && x <= (b.boundry.x + b.boundry.w))
  {
    if (y >= b.boundry.y && y <= (b.boundry.y + b.boundry.h))
    {
      takeTouchReadings = false;
      touchDebounceMillis = millis();
      return true;
    }
  }

  return false;
}
*/