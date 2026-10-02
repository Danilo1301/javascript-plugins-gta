#pragma once

#include "quickjs/quickjs.h"
#include "textDrawStyle.h"

struct JavaScriptContextData
{
    uint32_t scriptId;
    TextDrawStyle textDrawStyle;
};

class JavaScriptBindings
{
public:
    static void Register(JSContext* context, uint32_t scriptId);

private:
    static JSValue Print(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);
    static JSValue Info(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);
    static JSValue ShowBottomMessage(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);
    static JSValue OpenKeyboard(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);
};