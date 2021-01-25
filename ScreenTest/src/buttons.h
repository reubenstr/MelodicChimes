
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <map>
#include <vector>

class Buttons
{
public:


    struct Button
    {
        String text;
        int buttonId;
       int x, y, w, h;

        uint32_t textColor = TFT_BLACK;
        uint32_t fillColor = TFT_GREEN;
        uint32_t activeColor = TFT_WHITE;
        uint32_t borderColor = TFT_DARKGREEN;

        Button(int buttonId, String text, int x, int y, int w, int h, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
        {
            this->text = text;
            this->buttonId = buttonId;
              this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
            this->textColor = textColor;
            this->fillColor = fillColor;
            this->activeColor = activeColor;
            this->borderColor = borderColor;
        }
    };
  
    Buttons(TFT_eSPI *tft);

    void Add(int key, Button button);

    void DisplayButtons(int key);

private:
    TFT_eSPI *tft;

    std::map<int, std::vector<Button>> buttons;
};