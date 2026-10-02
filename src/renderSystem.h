#pragma once

#include "aml-psdk/gta_base/RGBA.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

enum class RenderCommandType
{
    Text,
    Rect,
    Sprite
};

struct TextDrawState
{
    float sizeX = 1.0f;
    float sizeY = 1.0f;

    CRGBA color = CRGBA(255, 255, 255, 255);

    bool centered = false;
};

struct TextRenderCommand
{
    std::string text;

    float x;
    float y;

    float sizeX;
    float sizeY;

    CRGBA color;

    bool centered;
};

struct RectRenderCommand
{
    float x;
    float y;

    float width;
    float height;

    CRGBA color;
};

struct RenderCommand
{
    RenderCommandType type;

    TextRenderCommand text;
    RectRenderCommand rect;
};

struct ScriptRenderContext
{
    TextDrawState textState;

    std::vector<RenderCommand> commands;
};

class RenderSystem
{
public:
    static void Initialize();

    static void BeginScriptsUpdate();
    static void EndScriptsUpdate();

    static void BeginScript(uint32_t scriptId);
    static void EndScript(uint32_t scriptId);

    static void SetTextSize(uint32_t scriptId, float sizeX, float sizeY);
    static void SetTextColor(uint32_t scriptId, int r, int g, int b, int a);
    static void SetTextCentered(uint32_t scriptId, bool centered);

    static void DrawText(uint32_t scriptId, float x, float y, const std::string& text);

    static void DrawRect(uint32_t scriptId, float x, float y, float width, float height, int r, int g, int b, int a);

    static void Render();

private:
    static ScriptRenderContext* GetScriptContext(uint32_t scriptId);

private:
    static std::unordered_map<uint32_t, ScriptRenderContext> _scriptContexts;

    static std::vector<RenderCommand> _renderCommands;
};