
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
        uint32_t textColor;
        uint32_t fillColor;
        int size;
        int x;
        int y;
        Justification justification;

        signed int minimumCharacters = -1;
        int padding = 0;

        Label(int id, String text, int size, int x, int y, uint32_t textColor, uint32_t fillColor, Justification justification)
        {
            this->id = id;
            this->text = text;
            this->size = size;
            this->x = x;
            this->y = y;
            this->textColor = textColor;
            this->fillColor = fillColor;
            this->justification = justification;
        }
    };

    Labels();
    Labels(TFT_eSPI *tft);

    void Add(int key, Label Labels);
    void SetMinimumCharacters(int key, int id, int amt);
    void SetPadding(int key, int id, int amt);
    void DisplayLabels(int key);
    void UpdateLabelText(int key, int id, String text, bool displayLabel = true);
    void UpdateLabelFillColor(int key, int id, uint32_t color, bool displayLabel = true);


private:
    void DisplayLabel(Label l);

    TFT_eSPI *tft;

    std::map<int, std::vector<Label>> labels;
};