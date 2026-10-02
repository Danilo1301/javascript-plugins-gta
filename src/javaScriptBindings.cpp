#include "javaScriptBindings.h"

#include "interfaces.h"
#include "javaScriptBindingsRender.h"
#include "mod/logger.h"
#include <string>

void JavaScriptBindings::Register(JSContext* context, uint32_t scriptId)
{
    auto* data = new JavaScriptContextData;
    data->scriptId = scriptId;

    JS_SetContextOpaque(context, data);

    JSValue globalObject = JS_GetGlobalObject(context);

    JS_SetPropertyStr(context, globalObject, "print", JS_NewCFunction(context, Print, "print", 1));

    JS_SetPropertyStr(context, globalObject, "info", JS_NewCFunction(context, Info, "info", 1));

    JS_SetPropertyStr(context, globalObject, "ShowBottomMessage", JS_NewCFunction(context, ShowBottomMessage, "info", 1));

    JS_SetPropertyStr(context, globalObject, "OpenKeyboard", JS_NewCFunction(context, OpenKeyboard, "info", 1));

    RegisterRenderBindings(context);

    JS_FreeValue(context, globalObject);
}

JSValue JavaScriptBindings::Print(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (argc > 0)
    {
        const char* text = JS_ToCString(context, argv[0]);

        if (text)
        {
            logger->Info("%s", text);
            JS_FreeCString(context, text);
        }
    }

    return JS_UNDEFINED;
}

JSValue JavaScriptBindings::Info(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (argc > 0)
    {
        const char* text = JS_ToCString(context, argv[0]);

        if (text)
        {
            if (menuSZK)
            {
                logger->Info("%s", text);
                menuSZK->LogOnScreen(std::string(text), ScreenLogType::Special, 10000);
            }
            JS_FreeCString(context, text);
        }
    }

    return JS_UNDEFINED;
}

JSValue JavaScriptBindings::ShowBottomMessage(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (argc < 2) { return JS_UNDEFINED; }

    const char* text = JS_ToCString(context, argv[0]);

    if (!text) { return JS_UNDEFINED; }

    int32_t duration;

    if (JS_ToInt32(context, &duration, argv[1]) < 0)
    {
        JS_FreeCString(context, text);
        return JS_UNDEFINED;
    }

    if (menuSZK) { menuSZK->ShowBottomMessage(std::string(text), duration); }

    JS_FreeCString(context, text);

    return JS_UNDEFINED;
}

JSValue JavaScriptBindings::OpenKeyboard(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (argc < 1) { return JS_UNDEFINED; }

    const char* text = JS_ToCString(context, argv[0]);

    if (!text) { return JS_UNDEFINED; }

    int32_t flags = 0;

    if (argc >= 2)
    {
        if (JS_ToInt32(context, &flags, argv[1]) < 0)
        {
            JS_FreeCString(context, text);
            return JS_UNDEFINED;
        }
    }

    if (menuSZK) { menuSZK->ToggleKeyboard(text, static_cast<KeyboardFlags>(flags)); }

    JS_FreeCString(context, text);

    return JS_UNDEFINED;
}