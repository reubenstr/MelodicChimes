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
        std::vector<Label> newVector{label};
        labels.insert(std::make_pair(key, newVector));
    }
    else
    {
        // Key found.
        labels[key].push_back(label);
    }
}

void Labels::SetMinimumCharacters(int key, int id, int amt)
{
    if (labels.find(key) != labels.end())
    {
        for (auto &l : labels[key])
        {
            if (l.id == id)
            {
                l.minimumCharacters = amt;
            }
        }
    }
}

void Labels::SetPadding(int key, int id, int amt)
{
    if (labels.find(key) != labels.end())
    {
        for (auto &l : labels[key])
        {
            if (l.id == id)
            {
                l.padding = amt;
            }
        }
    }
}

void Labels::DisplayLabel(Label b)
{
    tft->setTextFont(GLCD);
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

    tft->setTextSize(b.size);

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
        tft->setTextColor(b.textColor, b.fillColor);
    }

    tft->drawString(text, x, b.y);
}

void Labels::DisplayLabels(int key)
{
    // TODO: Check if key is found.
    for (auto const &l : labels[key])
    {
        DisplayLabel(l);
    }
}

void Labels::UpdateLabelText(int key, int id, String text, bool displayLabel = true)
{
    for (auto &l : labels[key])
    {
        if (l.id == id)
        {
            l.text = text;
            if (displayLabel)
            {
                DisplayLabel(l);
            }
        }
    }
}

void Labels::UpdateLabelFillColor(int key, int id, uint32_t color, bool displayLabel = true)
{
    for (auto &l : labels[key])
    {
        if (l.id == id)
        {
            l.fillColor = color;
            if (displayLabel)
            {
                DisplayLabel(l);
            }
        }
    }
}