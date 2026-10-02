#include "javaScriptBindingsRender.h"

#include "javaScriptBindings.h"
#include "drawUtils.h"
#include "mod/logger.h"

JSValue DrawRect(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    if (argc != 8) return JS_ThrowTypeError(context, "DrawRect expects 8 arguments");

    double left;
    double top;
    double right;
    double bottom;

    int r;
    int g;
    int b;
    int a;

    if (JS_ToFloat64(context, &left, argv[0]) < 0) return JS_EXCEPTION;

    if (JS_ToFloat64(context, &top, argv[1]) < 0) return JS_EXCEPTION;

    if (JS_ToFloat64(context, &right, argv[2]) < 0) return JS_EXCEPTION;

    if (JS_ToFloat64(context, &bottom, argv[3]) < 0) return JS_EXCEPTION;

    if (JS_ToInt32(context, &r, argv[4]) < 0) return JS_EXCEPTION;

    if (JS_ToInt32(context, &g, argv[5]) < 0) return JS_EXCEPTION;

    if (JS_ToInt32(context, &b, argv[6]) < 0) return JS_EXCEPTION;

    if (JS_ToInt32(context, &a, argv[7]) < 0) return JS_EXCEPTION;

    auto* data = static_cast<JavaScriptContextData*>(JS_GetContextOpaque(context));

    DrawUtils::DrawRect(
        CRect(static_cast<float>(left), static_cast<float>(bottom), static_cast<float>(right), static_cast<float>(top)), CRGBA(r, g, b, a));

    return JS_UNDEFINED;
}

JSValue SetTextSize(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    if (argc != 2) return JS_ThrowTypeError(context, "SetTextSize expects 2 arguments");

    double sizeX;
    double sizeY;

    if (JS_ToFloat64(context, &sizeX, argv[0]) < 0) return JS_EXCEPTION;
    if (JS_ToFloat64(context, &sizeY, argv[1]) < 0) return JS_EXCEPTION;

    auto* data = static_cast<JavaScriptContextData*>(JS_GetContextOpaque(context));

    data->textDrawStyle.sizeX = static_cast<float>(sizeX);
    data->textDrawStyle.sizeY = static_cast<float>(sizeY);

    return JS_UNDEFINED;
}

JSValue SetTextColor(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    if (argc != 4) return JS_ThrowTypeError(context, "SetTextColor expects 4 arguments");

    int r;
    int g;
    int b;
    int a;

    if (JS_ToInt32(context, &r, argv[0]) < 0) return JS_EXCEPTION;
    if (JS_ToInt32(context, &g, argv[1]) < 0) return JS_EXCEPTION;
    if (JS_ToInt32(context, &b, argv[2]) < 0) return JS_EXCEPTION;
    if (JS_ToInt32(context, &a, argv[3]) < 0) return JS_EXCEPTION;

    auto* data = static_cast<JavaScriptContextData*>(JS_GetContextOpaque(context));

    data->textDrawStyle.color = CRGBA(r, g, b, a);

    return JS_UNDEFINED;
}

JSValue SetTextCentered(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    if (argc != 1) return JS_ThrowTypeError(context, "SetTextCentered expects 1 argument");

    int value = JS_ToBool(context, argv[0]);

    if (value < 0) return JS_EXCEPTION;

    auto* data = static_cast<JavaScriptContextData*>(JS_GetContextOpaque(context));

    data->textDrawStyle.centered = value != 0;

    return JS_UNDEFINED;
}

JSValue DrawText(JSContext* context, JSValueConst, int argc, JSValueConst* argv)
{
    if (argc != 3) return JS_ThrowTypeError(context, "DrawText expects 3 arguments");

    double x;
    double y;

    if (JS_ToFloat64(context, &x, argv[0]) < 0) return JS_EXCEPTION;
    if (JS_ToFloat64(context, &y, argv[1]) < 0) return JS_EXCEPTION;

    const char* text = JS_ToCString(context, argv[2]);

    if (!text) return JS_EXCEPTION;

    auto* data = static_cast<JavaScriptContextData*>(JS_GetContextOpaque(context));

    DrawUtils::DrawText(text, CVector2D(static_cast<float>(x), static_cast<float>(y)), data->textDrawStyle);

    JS_FreeCString(context, text);

    return JS_UNDEFINED;
}

void RegisterRenderBindings(JSContext* context)
{
    JSValue globalObject = JS_GetGlobalObject(context);

    JS_SetPropertyStr(context, globalObject, "DrawRect", JS_NewCFunction(context, DrawRect, "DrawRect", 8));

    JS_SetPropertyStr(context, globalObject, "SetTextSize", JS_NewCFunction(context, SetTextSize, "SetTextSize", 2));

    JS_SetPropertyStr(context, globalObject, "SetTextColor", JS_NewCFunction(context, SetTextColor, "SetTextColor", 4));

    JS_SetPropertyStr(context, globalObject, "SetTextCentered", JS_NewCFunction(context, SetTextCentered, "SetTextCentered", 1));

    JS_SetPropertyStr(context, globalObject, "DrawText", JS_NewCFunction(context, DrawText, "DrawText", 3));

    JS_FreeValue(context, globalObject);
}