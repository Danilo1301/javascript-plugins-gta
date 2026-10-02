#pragma once

#include "aml-psdk/gta_base/RGBA.h"

struct TextDrawStyle
{
    float sizeX = 1.0f;
    float sizeY = 1.0f;

    CRGBA color = CRGBA(255, 255, 255, 255);

    bool centered = false;
};