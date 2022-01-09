#include <Arduino.h>
#include <gfxItem.h>
#include <TFT_eSPI.h>
#include <map>
#include <vector>

#ifndef GFXITEMS_H
#define GFXITEMS_H

class GFXItems
{
public:
    GFXItems(TFT_eSPI *tft);
    void Add(GFXItem gfxItem);

    void DisplayGfxItem(int id);
    void DisplayGroup(int groupId);
    bool IsItemInGroupPressed(int key, int *id);
    GFXItem &GetGfxItemById(int id);

private:
    TFT_eSPI *tft;
    std::vector<GFXItem> gfxItems;    

    void DisplayElement(GFXItem b);

    GFXItem &operator[](int i)
    {
        for (auto &b : gfxItems)
        {
            if (b.id == i)
            {
                return gfxItems[i];
            }
        }
        return gfxItems[i];
    }
};

#endif