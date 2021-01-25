#include "buttons.h"
#include <vector>
#include "Free_Fonts.h"
#include <TFT_eSPI.h>

Buttons::Buttons(TFT_eSPI *tft)
{
  this->tft = tft;
}

void Buttons::Add(GFXItem button)
{
  buttons.push_back(button);
}

void Buttons::AddElementToGroup(int groupId, int eId)
{
  if (groups.find(groupId) != groups.end())
  {
    groups[groupId].push_back(eId);
  }
  else
  {
    std::vector<int> newVector{eId};
    groups.insert(std::make_pair(groupId, newVector));
  }
}

GFXItem &Buttons::GetButtonById(int eId)
{
  for (auto &b : buttons)
  {
    if (b.id == eId)
    {
      return b;
    }
  }
  return buttons[0];
}

void Buttons::DisplayElement(GFXItem b)
{

  uint32_t backgroundColor = b.fillColor;
  if (b.isPressed)
    backgroundColor = b.activeColor;

  tft->setTextFont(GLCD);
  tft->setTextSize(b.textSize);
  tft->setTextColor(b.textColor);

  String text(b.text);

  if (b.cornerSize > 0)
  {
    tft->fillRoundRect(b.x, b.y, b.w, b.h, b.cornerSize, b.borderColor);
  }

  tft->fillRoundRect(b.x + b.borderThickness, b.y + b.borderThickness, b.w - b.borderThickness * 2, b.h - b.borderThickness * 2, b.cornerSize, backgroundColor);

  int textX;
  if (b.justification == Justification::Left)
  {
    textX = b.x;
  }
  else if (b.justification == Justification::Center)
  {
    textX = b.x + b.w / 2 - tft->textWidth(b.text) / 2;
  }
  else if (b.justification == Justification::Right)
  {
    textX = b.x + b.w - tft->textWidth(text);
  }

  int textY;
  textY = b.y + b.h / 2 - tft->fontHeight() / 2;

  tft->drawString(text, textX, textY);

  /*
  int borderThickness = b.borderThickness;
  tft->fillRoundRect(b.x, b.y, b.w, b.h, 6, b.borderColor);
  tft->fillRoundRect(b.x + borderThickness, b.y + borderThickness, b.w - borderThickness * 2, b.h - borderThickness * 2, 5, backgroundColor);

  int textWidth = tft->textWidth(b.text);
  int textHeight = tft->fontHeight();
  tft->drawString(b.text, b.x + (b.w - textWidth) / 2 - 1, b.y + (b.h - textHeight) / 2 + borderThickness * 2);
*/

  /*
  int x;
  String text(b.text);

  // Pad text with spaces on the left and right to center the text which clears the background.
  if (b.minimumCharacters != -1)
  {
    signed int spaces = (b.minimumCharacters - (signed int)b.text.length()) / 2;
    char buf[50];
    if (spaces > 0)
    {
      sprintf(buf, "%*s%s%*s", spaces, "", b.text.c_str(), spaces - 1, "");
      text = String(buf);
    }
  }

  if (b.justification == Justification::Left)
  {
    x = b.x;
  }
  else if (b.justification == Justification::Center)
  {
    x = b.x - (tft->textWidth(text) / 2);
  }
  else if (b.justification == Justification::Right)
  {
    x = b.x - tft->textWidth(text);
  }

  if (b.padding > 0)
  {
    tft->setTextColor(b.textColor);
    tft->fillRect(x - b.padding, b.y - b.padding, tft->textWidth(text) + b.padding * 2, tft->fontHeight() + b.padding * 2, b.fillColor);
  }
  else
  {
    tft->setTextColor(b.textColor, backgroundColor);
  }

  tft->drawString(text, x, b.y);
  */
}

void Buttons::DisplayButton(int eId)
{
  for (auto const &b : buttons)
  {
    if (b.id == eId)
    {
      DisplayElement(b);
    }
  }
}

void Buttons::DisplayGroup(int groupId)
{
  if (groups.find(groupId) != groups.end())
  {
    for (auto const &eId : groups[groupId])
    {
      for (auto &b : buttons)
      {
        if (b.id == eId)
        {
          DisplayElement(b);
        }
      }
    }
  }
}

bool Buttons::IsButtonInGroupPressed(int groupId, int *id)
{

  uint16_t x, y;
  bool buttonPressedFlag = false;

  if (tft->getTouch(&x, &y, 20))
  {
    if (groups.find(groupId) != groups.end())
    {
      for (auto const &eId : groups[groupId])
      {
        GFXItem &b = GetButtonById(eId);
        if (b.isPressable)
        {
          bool isButtonPressed = false;

          // Check if touch is contained in the boundry of the button.
          if (x >= b.x && x <= (b.x + b.w))
          {
            if (y >= b.y && y <= (b.y + b.h))
            {
              isButtonPressed = true;
            }
          }

          if (isButtonPressed)
          {
            buttonPressedFlag = true;
            *id = b.id;
            // Prevent button flashing when being held down.
            if (!b.isPressed)
            {
              b.isPressed = true;
              DisplayElement(b);
            }
          }
        }
      }
    }
  }
  else
  {
    // No touch detected, depress all buttons.
    if (groups.find(groupId) != groups.end())
    {
      for (auto const &eId : groups[groupId])
      {

        GFXItem &b = GetButtonById(eId);
        if (b.isPressed)
        {
          b.isPressed = false;
          DisplayElement(b);
        }
      }
    }
  }

  return buttonPressedFlag;
}
