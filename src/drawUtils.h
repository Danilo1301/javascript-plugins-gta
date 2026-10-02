#pragma once

#include <string>

#include "aml-psdk/gta_base/Rect.h"
#include "textDrawStyle.h"
#include "aml-psdk/gta_base/RGBA.h"
#include "aml-psdk/gta_base/Vector.h"

class DrawUtils
{
public:
    static constexpr float BASE_WIDTH = 640.0f;
    static constexpr float BASE_HEIGHT = 448.0f;

public:
    static void Initialize();

    static float MapWidthToScreen(float value);
    static float MapHeightToScreen(float value);

    static void DrawRect(CRect rect, CRGBA color);

    static void DrawText(const std::string& text, CVector2D position, const TextDrawStyle& style);
};