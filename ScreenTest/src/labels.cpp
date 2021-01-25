#include "labels.h"
#include <vector>
#include "Free_Fonts.h"

Labels::Labels()
{
}

Labels::Labels(TFT_eSPI *tft)
{
    this->tft = tft;
}

void Labels::Add(int key, Label label)
{
    // Check if key exists.
    if (labels.find(key) == labels.end())
    {
        // Key not found.
        std::vector<Label> newVector {label};
        labels.insert(std::make_pair(key, newVector));
    }
    else
    {
        // Key found.
        labels[key].push_back(label);
    }
}

void Labels::DisplayLabels(int key)
{

    // TODO: Check if key is found.

      tft->setTextFont(GLCD);

    for (auto const &l : labels[key])
    {   
        int x;
        tft->setTextSize(l.size);
        tft->setTextColor(l.textColor);

        if (l.justification == Justification::Left)
        {
            x = l.x;
        }
        else if (l.justification == Justification::Center)
        {
            x = l.x - (tft->textWidth(l.text) / 2);
        }
        else if (l.justification == Justification::Right)
        {
            x = l.x - tft->textWidth(l.text);
        }
        tft->drawString(l.text, x, l.y);
    }
}