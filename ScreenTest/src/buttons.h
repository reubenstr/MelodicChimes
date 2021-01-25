
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
        int id;
        int x, y, w, h;

        uint32_t textColor;
        uint32_t fillColor;
        uint32_t activeColor;
        uint32_t borderColor;

        bool isPressed = false;

        Button(int id, String text, int x, int y, int w, int h, uint32_t textColor, uint32_t fillColor, uint32_t activeColor, uint32_t borderColor)
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
        }
    };

    Buttons(TFT_eSPI *tft);

    void Add(int key, Button button);

    void DisplayButtons(int key);

    bool IsButtonPressed(int key, int *id);

private:


    TFT_eSPI *tft;
    std::map<int, std::vector<Button>> buttons;
     Button *lastPressedButton;

    void DisplayButton(Button b);
};