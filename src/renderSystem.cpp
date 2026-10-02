#include "renderSystem.h"

#include "aml-psdk/gta_base/RGBA.h"
#include "aml-psdk/gta_base/Rect.h"
#include "drawUtils.h"

std::unordered_map<uint32_t, ScriptRenderContext> RenderSystem::_scriptContexts;
std::vector<RenderCommand> RenderSystem::_renderCommands;

void RenderSystem::Initialize()
{
    _scriptContexts.clear();
    _renderCommands.clear();
}

void RenderSystem::BeginScriptsUpdate()
{
    _renderCommands.clear();

    for (auto& [scriptId, context] : _scriptContexts) { context.commands.clear(); }
}

void RenderSystem::EndScriptsUpdate()
{
    for (auto& [scriptId, context] : _scriptContexts)
    {
        for (const RenderCommand& command : context.commands) { _renderCommands.push_back(command); }
    }
}

void RenderSystem::BeginScript(uint32_t scriptId)
{
    GetScriptContext(scriptId);
}

void RenderSystem::EndScript(uint32_t scriptId)
{
}

void RenderSystem::SetTextSize(uint32_t scriptId, float sizeX, float sizeY)
{
    ScriptRenderContext* context = GetScriptContext(scriptId);

    context->textState.sizeX = sizeX;
    context->textState.sizeY = sizeY;
}

void RenderSystem::SetTextColor(uint32_t scriptId, int r, int g, int b, int a)
{
    ScriptRenderContext* context = GetScriptContext(scriptId);

    context->textState.color = CRGBA(r, g, b, a);
}

void RenderSystem::SetTextCentered(uint32_t scriptId, bool centered)
{
    ScriptRenderContext* context = GetScriptContext(scriptId);

    context->textState.centered = centered;
}

void RenderSystem::DrawText(uint32_t scriptId, float x, float y, const std::string& text)
{
    auto* context = GetScriptContext(scriptId);

    if (!context) return;

    RenderCommand command;

    command.type = RenderCommandType::Text;

    command.text.text = text;
    command.text.x = x;
    command.text.y = y;

    command.text.sizeX = context->textState.sizeX;
    command.text.sizeY = context->textState.sizeY;

    command.text.color = context->textState.color;
    command.text.centered = context->textState.centered;

    context->commands.push_back(std::move(command));
}

void RenderSystem::DrawRect(uint32_t scriptId, float x, float y, float width, float height, int r, int g, int b, int a)
{
    auto* context = GetScriptContext(scriptId);

    if (!context) return;

    RenderCommand command;

    command.type = RenderCommandType::Rect;

    command.rect.x = x;
    command.rect.y = y;
    command.rect.width = width;
    command.rect.height = height;
    command.rect.color = CRGBA(r, g, b, a);

    context->commands.push_back(std::move(command));
}

ScriptRenderContext* RenderSystem::GetScriptContext(uint32_t scriptId)
{
    return &_scriptContexts[scriptId];
}

void RenderSystem::Render()
{
    for (const auto& command : _renderCommands)
    {
        switch (command.type)
        {
            case RenderCommandType::Text:
            {
                const auto& text = command.text;

                TextDrawStyle style;

                style.sizeX = text.sizeX;
                style.sizeY = text.sizeY;
                style.color = text.color;
                style.centered = text.centered;

                DrawUtils::DrawText(text.text, CVector2D(text.x, text.y), style);

                break;
            }

            case RenderCommandType::Rect:
            {
                const auto& rect = command.rect;

                DrawUtils::DrawRect(CRect(0, 0, 500, 500), rect.color);

                break;
            }

            case RenderCommandType::Sprite: break;
        }
    }
}