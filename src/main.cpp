#include "main.h"

#include "data.h"
#include "hooks.h"
#include "javaScriptEngine.h"

#include "aml-psdk/game_sa/Events.h"

#include "logUtils.h"
#include "mod/logger.h"

#include "javaScriptOpcode.h"
#include "renderSystem.h"
#include "testingOpcodes.h"
#include "drawUtils.h"

#include "input.h"

#include "interfaces.h"
#include <functional>
#include <vector>

MYMODCFG(com.daniloszk.javascriptplugins, JavascriptPlugins, 1.0.0, DaniloSZK);

JavaScriptEngine* g_JavaScriptEngine = nullptr;

std::vector<std::function<void()>> scriptEventQueue;

void _OnLoggerMessage(eLogPrio prio, const char* msg)
{
    if (!msg) return;

    if (menuSZK) { menuSZK->AddLogMessage("JavascriptPlugins: " + std::string(msg)); }
}

ON_MOD_PRELOAD()
{
    logger->SetTag("JavascriptPlugins-PSDK");
}

ON_MOD_LOAD()
{
    logger->Info("Mod loading...");
    logger->SetMessageCB(_OnLoggerMessage);

    menuSZK = (IMenuSZK*)GetInterface("menuSZK_v2");

    if (menuSZK == nullptr) { logger->Error("MenuSZK interface was not found!"); }

    aml->CreateDirRecursive(GetScriptsFolder().c_str());

    pGTASA = aml->GetLib("libGTASA.so");
    hGTASA = aml->GetLibHandle("libGTASA.so");

    DoHooks();

    DrawUtils::Initialize();
    Input::Initialize();
    RenderSystem::Initialize();

    g_JavaScriptEngine = new JavaScriptEngine();

    if (!g_JavaScriptEngine->Initialize())
    {
        logger->Error("Failed to initialize JavaScript engine");
        delete g_JavaScriptEngine;
        g_JavaScriptEngine = nullptr;

        return;
    }

    logger->Info("JavaScript engine initialized");
    logger->Info("Mod loaded");

    Input::Initialize();
    RenderSystem::Initialize();

    logger->Info("Loading scripts from folder...");

    g_JavaScriptEngine->LoadScripts(GetScriptsFolder());

    g_JavaScriptEngine->CallFunction("onRegisterOpcodes");

    //

    JavaScriptOpcode::DefineRedirect(0xF000,
        [](JSContext* context, int argc, JSValueConst* argv)
        {
            if (argc != 2) { return JS_ThrowTypeError(context, "CREATE_SCRIPT_ENTRY expects 2 arguments, got %d", argc); }

            const char* scriptName = JS_ToCString(context, argv[0]);

            if (!scriptName) return JS_EXCEPTION;

            const char* data = JS_ToCString(context, argv[1]);

            if (!data)
            {
                JS_FreeCString(context, scriptName);
                return JS_EXCEPTION;
            }

            std::string scriptData = data;

            if (menuSZK)
            {
                menuSZK->AddCellphoneScript(scriptName,
                    "",
                    [data = std::move(scriptData)]()
                    {
                        scriptEventQueue.push_back(
                            [data]()
                            {
                                if (g_JavaScriptEngine) g_JavaScriptEngine->OnScriptCalled(data);
                            });
                    });
            }

            JS_FreeCString(context, scriptName);
            JS_FreeCString(context, data);

            return JS_NewBool(context, true);
        });

    JavaScriptOpcode::DefineRedirect(0x0DE0,
        [](JSContext* context, int argc, JSValueConst* argv)
        {
            //logger->Info("someone called be, 0x0DE0");

            int point;
            int minTime;

            if (JS_ToInt32(context, &point, argv[0]) < 0) return JS_EXCEPTION;

            if (JS_ToInt32(context, &minTime, argv[1]) < 0) return JS_EXCEPTION;

            const bool pressed = Input::IsTouchPointPressed(point, static_cast<uint32_t>(minTime));

            //if (pressed) logger->Info("Will return true");

            return JS_NewBool(context, pressed);
        });

    //

    g_JavaScriptEngine->CallFunction("onRegisterAliases");

    g_JavaScriptEngine->RegisterOpcodeContexts();

    g_JavaScriptEngine->CallFunction("onLoaded");

    Events::timerUpdateEvent += []() { g_JavaScriptEngine->CallFunction("onTimerUpdate"); };

    Events::processScriptsEvent += []()
    {
        BEGIN_OPERATION(op_ProcessScripts);

        RenderSystem::BeginScriptsUpdate();

        auto events = std::move(scriptEventQueue);
        scriptEventQueue.clear();

        for (auto& event : events) { event(); }

        g_JavaScriptEngine->CallFunction("onGameProcess");

        RenderSystem::EndScriptsUpdate();

        END_OPERATION(op_ProcessScripts);
    };

    Events::drawHudEvent += []() {};

    //

    if (menuSZK)
    {
        menuSZK->AddCellphoneScript("Test opcodes", "", []() { scriptEventQueue.push_back([]() { TestOnce(); }); });
    }

    if (menuSZK)
    {
        menuSZK->onKeyboardResult->Add(
            [](std::string result)
            {
                auto scripts = g_JavaScriptEngine->GetScripts();

                for (auto& script : scripts)
                {
                    JSValue argv[1];
                    argv[0] = JS_NewString(script.context, result.c_str());

                    logger->Info("Keyboard result: '%s'", result.c_str());

                    g_JavaScriptEngine->CallScriptFunction(script, "onKeyboardEnterPressed", 1, argv);

                    JS_FreeValue(script.context, argv[0]);
                }
            });
    }
}

void Render()
{
    BEGIN_OPERATION(op_OnDraw);
    g_JavaScriptEngine->CallFunction("onDraw");
    END_OPERATION(op_OnDraw);
}