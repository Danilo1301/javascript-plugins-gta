#include "drawUtils.h"

#include "aml-psdk/game_sa/Events.h"
#include "aml-psdk/game_sa/engine/OS.h"
#include "aml-psdk/game_sa/engine/Sprite2d.h"
#include "aml-psdk/game_sa/engine/Font.h"

#include "hooks.h"
#include "mod/logger.h"
#include "textDrawStyle.h"

namespace
{
int g_screenWidth = 0;
int g_screenHeight = 0;

char g_textBuffer[0xFF];
GxtChar* g_gxtText = new GxtChar[0xFF];
} // namespace

void DrawUtils::Initialize()
{
    g_screenWidth = OS_ScreenGetWidth();
    g_screenHeight = OS_ScreenGetHeight();

    Events::timerUpdateEvent += []()
    {
        const int screenWidth = OS_ScreenGetWidth();
        const int screenHeight = OS_ScreenGetHeight();

        if (screenWidth == g_screenWidth && screenHeight == g_screenHeight) return;

        g_screenWidth = screenWidth;
        g_screenHeight = screenHeight;

        logger->Info("DrawUtils: screen resolution changed to %dx%d", g_screenWidth, g_screenHeight);
    };
}

float DrawUtils::MapWidthToScreen(float value)
{
    return (value / BASE_WIDTH) * g_screenWidth;
}

float DrawUtils::MapHeightToScreen(float value)
{
    return (value / BASE_HEIGHT) * g_screenHeight;
}

void DrawUtils::DrawRect(CRect rect, CRGBA color)
{
    rect.left = MapWidthToScreen(rect.left);
    rect.right = MapWidthToScreen(rect.right);
    rect.top = MapHeightToScreen(rect.top);
    rect.bottom = MapHeightToScreen(rect.bottom);

    CSprite2d_DrawRect(rect, color);
}

void DrawUtils::DrawText(const std::string& text, CVector2D position, const TextDrawStyle& style)
{
    const float x = MapWidthToScreen(position.x);
    const float y = MapHeightToScreen(position.y);

    const float scaleX = style.sizeX;
    const float scaleY = style.sizeY;

    CRGBA color = style.color;

    FontSetOrientation(style.centered ? eFontAlignment::ALIGN_CENTER : eFontAlignment::ALIGN_LEFT);
    FontSetColor(&color);
    FontSetBackground(false, false);
    FontSetWrapx(3000.0f);
    FontSetScale(scaleX, scaleY);
    FontSetStyle(eFontStyle::FONT_SUBTITLES);
    FontSetProportional(true);
    FontSetDropShadowPosition(0);

    sprintf(g_textBuffer, "%s", text.c_str());
    AsciiToGxtChar(g_textBuffer, g_gxtText);

    CRect rect;
    CFont::GetTextRect(&rect, x, y, g_gxtText);

    const float textHeight = rect.bottom - rect.top;
    const float centeredY = y + textHeight / 2.0f;

    CFont::PrintString(x, centeredY, g_gxtText);

    RenderFontBuffer();
}