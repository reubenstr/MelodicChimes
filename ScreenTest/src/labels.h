
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <map>
#include <vector>

class Labels
{
    public:
    enum class Justification
    {
        Left,
        Center,
        Right
    };

    struct Label
    {
        String text;
        int id;
        uint32_t textColor = TFT_BLACK;
        int size;
        int x;
        int y;
        Justification justification;

        Label(int id, String text, int size, int x, int y, uint32_t textColor, Justification justification)
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

    Labels();
    Labels(TFT_eSPI *tft);

    void Add(int key, Label label);

    void DisplayLabels(int key);

private:

    TFT_eSPI *tft;

    std::map<int, std::vector<Label>> labels;
};