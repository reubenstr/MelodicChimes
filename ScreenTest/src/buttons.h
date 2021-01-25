
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <map>
#include <vector>

#ifndef BUTTONS_H
#define BUTTONS_H

enum class Justification
{
    Left,
    Center,
    Right
};

struct GFXItem
{
    String text;
    int id;
    bool isPressable = true;
    int x, y;
    int w = 0;
    int h = 0;

    uint32_t textColor;
    uint32_t fillColor;
    uint32_t activeColor;
    uint32_t borderColor;

    bool isPressed = false;
    int textSize = 1;

    Justification justification = Justification::Center;

    int borderThickness = 0;
    signed int minimumCharacters = -1;
    int padding = -1;
    int cornerSize = 0;

    GFXItem();

    // Candidate for buttons.
    GFXItem(int id, String text, int x, int y, int w, int h, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
    {
        this->text = text;
        this->id = id;
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->textColor = textColor;
        this->fillColor = fillColor;
        this->activeColor = activeColor;
        this->borderColor = borderColor;
              
        cornerSize = 5;
        textSize = 3;
        borderThickness = 1;
    }

    // Candidate for labels.
    GFXItem(int id, String text, int textSize, int x, int y, int w, int h, uint32_t textColor, uint32_t fillColor, Justification justification)
    {
        this->id = id;
        this->text = text;
        this->textSize = textSize;
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->textColor = textColor;
        this->fillColor = fillColor;
        this->justification = justification;

      
    }
};

class Buttons
{
public:
    Buttons(TFT_eSPI *tft);

    void Add(GFXItem button);

    void AddElementToGroup(int groupId, int eId);

    void DisplayButton(int eId);

    bool IsButtonInGroupPressed(int key, int *id);

    void DisplayGroup(int groupId);

    GFXItem &GetButtonById(int eId);

    // Button operator [](int i) const    {return vButtons[i];}

    GFXItem &operator[](int i)
    {
        for (auto &b : buttons)
        {
            if (b.id == i)
            {
                return buttons[i];
            }
        }
        return buttons[i];
    }

private:
    std::vector<GFXItem> buttons;

    TFT_eSPI *tft;
    std::map<int, std::vector<int>> groups;
    GFXItem *lastPressedButton;

    void DisplayElement(GFXItem b);
};

#endif