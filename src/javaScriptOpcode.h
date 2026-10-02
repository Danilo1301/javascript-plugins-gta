#pragma once

#include "quickjs/quickjs.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

enum class JavaScriptOpcodeType
{
    Void,
    Int,
    Float,
    Double,
    Bool,
    String,
    IntPointer,
    FloatPointer,
    BoolPointer
};

struct JavaScriptOpcodeDefinition
{
    uint16_t opcode;
    std::string name;
    std::vector<JavaScriptOpcodeType> arguments;
    JavaScriptOpcodeType returnType;
};

struct JavaScriptOpcodeAlias
{
    uint16_t opcode;
    std::string name;
};

class JavaScriptOpcode
{
public:
    static void Initialize(JSContext* context);
    static void RegisterContext(JSContext* context);

    static bool Define(
        uint16_t opcode, const std::string& name, const std::vector<JavaScriptOpcodeType>& arguments, JavaScriptOpcodeType returnType);

    static bool DefineRedirect(uint16_t opcode, std::function<JSValue(JSContext*, int, JSValueConst*)> handler);

private:
    static JavaScriptOpcodeType ParseType(const char* type);

    static JSValue DefineOpcode(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);

    static JSValue ExecuteOpcode(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv, int magic);

    static JSValue MakeAlias(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv);

    static bool CreateAlias(uint16_t opcode, const std::string& name);

    static const JavaScriptOpcodeDefinition* FindDefinition(uint16_t opcode);

    static int GetArgumentCount(const JavaScriptOpcodeDefinition& definition);

private:
    static std::vector<JavaScriptOpcodeDefinition> _definitions;
    static std::vector<JavaScriptOpcodeAlias> _aliases;

    static std::unordered_map<uint16_t, std::function<JSValue(JSContext*, int, JSValueConst*)>> _redirects;
};