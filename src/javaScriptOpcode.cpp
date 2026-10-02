#include "javaScriptOpcode.h"

#include "mod/logger.h"
#include "OpcodeCaller_fixed.h"

#include <cstring>
#include <utility>
#include <vector>

std::vector<JavaScriptOpcodeDefinition> JavaScriptOpcode::_definitions;
std::vector<JavaScriptOpcodeAlias> JavaScriptOpcode::_aliases;

std::unordered_map<uint16_t, std::function<JSValue(JSContext*, int, JSValueConst*)>> JavaScriptOpcode::_redirects;

void JavaScriptOpcode::Initialize(JSContext* context)
{
    JSValue globalObject = JS_GetGlobalObject(context);

    JS_SetPropertyStr(context, globalObject, "DefineOpcode", JS_NewCFunction(context, DefineOpcode, "DefineOpcode", 4));

    JS_SetPropertyStr(context, globalObject, "MakeAlias", JS_NewCFunction(context, MakeAlias, "MakeAlias", 2));

    JS_FreeValue(context, globalObject);
}

bool JavaScriptOpcode::Define(
    uint16_t opcode, const std::string& name, const std::vector<JavaScriptOpcodeType>& arguments, JavaScriptOpcodeType returnType)
{
    for (auto& current : _definitions)
    {
        if (current.opcode != opcode) continue;

        logger->Info("Replacing opcode 0x%04X: %s -> %s", opcode, current.name.c_str(), name.c_str());

        current.name = name;
        current.arguments = arguments;
        current.returnType = returnType;

        return true;
    }

    JavaScriptOpcodeDefinition definition;

    definition.opcode = opcode;
    definition.name = name;
    definition.arguments = arguments;
    definition.returnType = returnType;

    _definitions.push_back(std::move(definition));

    //logger->Info("Opcode defined globally: 0x%04X -> %s", opcode, name.c_str());

    return true;
}

bool JavaScriptOpcode::DefineRedirect(uint16_t opcode, std::function<JSValue(JSContext*, int, JSValueConst*)> handler)
{
    _redirects[opcode] = std::move(handler);

    return true;
}

void JavaScriptOpcode::RegisterContext(JSContext* context)
{
    JSValue globalObject = JS_GetGlobalObject(context);

    for (const auto& definition : _definitions)
    {
        JSValue function = JS_NewCFunctionMagic(
            context, ExecuteOpcode, definition.name.c_str(), GetArgumentCount(definition), JS_CFUNC_generic_magic, definition.opcode);

        if (JS_IsException(function)) continue;

        JS_SetPropertyStr(context, globalObject, definition.name.c_str(), function);
    }

    for (const auto& alias : _aliases)
    {
        const auto* definition = FindDefinition(alias.opcode);

        if (!definition) continue;

        JSValue function = JS_NewCFunctionMagic(
            context, ExecuteOpcode, alias.name.c_str(), GetArgumentCount(*definition), JS_CFUNC_generic_magic, alias.opcode);

        if (JS_IsException(function)) continue;

        JS_SetPropertyStr(context, globalObject, alias.name.c_str(), function);
    }

    JS_FreeValue(context, globalObject);
}

JavaScriptOpcodeType JavaScriptOpcode::ParseType(const char* type)
{
    if (!type) return JavaScriptOpcodeType::Void;

    if (strcmp(type, "void") == 0) return JavaScriptOpcodeType::Void;

    if (strcmp(type, "int") == 0) return JavaScriptOpcodeType::Int;

    if (strcmp(type, "float") == 0) return JavaScriptOpcodeType::Float;

    if (strcmp(type, "double") == 0) return JavaScriptOpcodeType::Double;

    if (strcmp(type, "bool") == 0) return JavaScriptOpcodeType::Bool;

    if (strcmp(type, "string") == 0) return JavaScriptOpcodeType::String;

    if (strcmp(type, "int*") == 0) return JavaScriptOpcodeType::IntPointer;

    if (strcmp(type, "float*") == 0) return JavaScriptOpcodeType::FloatPointer;

    if (strcmp(type, "bool*") == 0) return JavaScriptOpcodeType::BoolPointer;

    return JavaScriptOpcodeType::Void;
}

JSValue JavaScriptOpcode::DefineOpcode(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (argc < 4) { return JS_ThrowTypeError(context, "DefineOpcode(opcode, name, arguments, returnType)"); }

    int32_t opcode;

    if (JS_ToInt32(context, &opcode, argv[0]) < 0) return JS_EXCEPTION;

    const char* name = JS_ToCString(context, argv[1]);

    if (!name) return JS_EXCEPTION;

    if (!JS_IsArray(argv[2]))
    {
        JS_FreeCString(context, name);

        return JS_ThrowTypeError(context, "DefineOpcode arguments must be an array");
    }

    const char* returnTypeString = JS_ToCString(context, argv[3]);

    if (!returnTypeString)
    {
        JS_FreeCString(context, name);

        return JS_EXCEPTION;
    }

    std::vector<JavaScriptOpcodeType> arguments;

    JSValue lengthValue = JS_GetPropertyStr(context, argv[2], "length");

    uint32_t length;

    if (JS_ToUint32(context, &length, lengthValue) < 0)
    {
        JS_FreeValue(context, lengthValue);
        JS_FreeCString(context, returnTypeString);
        JS_FreeCString(context, name);

        return JS_EXCEPTION;
    }

    JS_FreeValue(context, lengthValue);

    arguments.reserve(length);

    for (uint32_t i = 0; i < length; ++i)
    {
        JSValue typeValue = JS_GetPropertyUint32(context, argv[2], i);

        const char* type = JS_ToCString(context, typeValue);

        if (!type)
        {
            JS_FreeValue(context, typeValue);
            JS_FreeCString(context, returnTypeString);
            JS_FreeCString(context, name);

            return JS_EXCEPTION;
        }

        JavaScriptOpcodeType parsedType = ParseType(type);

        JS_FreeCString(context, type);
        JS_FreeValue(context, typeValue);

        arguments.push_back(parsedType);
    }

    JavaScriptOpcodeType returnType = ParseType(returnTypeString);

    JS_FreeCString(context, returnTypeString);

    const std::string opcodeName = name;

    JS_FreeCString(context, name);

    if (!Define(static_cast<uint16_t>(opcode), opcodeName, arguments, returnType))
    {
        return JS_ThrowInternalError(context, "Failed to define opcode");
    }

    return JS_UNDEFINED;
}

JSValue JavaScriptOpcode::ExecuteOpcode(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv, int magic)
{
    const uint16_t opcode = static_cast<uint16_t>(magic);

    auto redirect = _redirects.find(opcode);

    if (redirect != _redirects.end()) return redirect->second(context, argc, argv);

    const JavaScriptOpcodeDefinition* definition = FindDefinition(opcode);

    if (!definition) { return JS_ThrowInternalError(context, "Opcode definition not found: 0x%04X", opcode); }

    const int expectedArgCount = GetArgumentCount(*definition);

    if (argc != expectedArgCount)
    {
        return JS_ThrowTypeError(context, "%s expects %d arguments, got %d", definition->name.c_str(), expectedArgCount, argc);
    }

    struct Output
    {
        JavaScriptOpcodeType type;

        int intValue = 0;
        float floatValue = 0.0f;

        uint32_t boolStorage = 0;
        bool boolValue = false;
    };

    std::vector<Output> outputs;

    outputs.reserve(definition->arguments.size() + 1);

    std::vector<int> intValues;
    std::vector<float> floatValues;
    std::vector<double> doubleValues;
    std::vector<bool> boolValues;
    std::vector<std::string> stringValues;

    intValues.reserve(expectedArgCount);
    floatValues.reserve(expectedArgCount);
    doubleValues.reserve(expectedArgCount);
    boolValues.reserve(expectedArgCount);
    stringValues.reserve(expectedArgCount);

    OpcodeCaller::Reset();

    memcpy(OpcodeCaller::g_ScriptBuffer, &opcode, sizeof(opcode));

    OpcodeCaller::g_nBufferPos = 2;

    int jsArgIndex = 0;

    for (const auto type : definition->arguments)
    {
        switch (type)
        {
            case JavaScriptOpcodeType::Int:
            {
                int32_t value;

                if (JS_ToInt32(context, &value, argv[jsArgIndex]) < 0) { return JS_EXCEPTION; }

                intValues.push_back(static_cast<int>(value));

                OpcodeCaller::PushVarToBuffer(intValues.back());

                ++jsArgIndex;

                break;
            }

            case JavaScriptOpcodeType::Float:
            {
                double value;

                if (JS_ToFloat64(context, &value, argv[jsArgIndex]) < 0) { return JS_EXCEPTION; }

                floatValues.push_back(static_cast<float>(value));

                OpcodeCaller::PushVarToBuffer(floatValues.back());

                ++jsArgIndex;

                break;
            }

            case JavaScriptOpcodeType::Double:
            {
                double value;

                if (JS_ToFloat64(context, &value, argv[jsArgIndex]) < 0) { return JS_EXCEPTION; }

                doubleValues.push_back(value);

                OpcodeCaller::PushVarToBuffer(doubleValues.back());

                ++jsArgIndex;

                break;
            }

            case JavaScriptOpcodeType::Bool:
            {
                const int value = JS_ToBool(context, argv[jsArgIndex]);

                if (value < 0) return JS_EXCEPTION;

                boolValues.push_back(value != 0);

                OpcodeCaller::PushVarToBuffer(boolValues.back());

                ++jsArgIndex;

                break;
            }

            case JavaScriptOpcodeType::String:
            {
                const char* value = JS_ToCString(context, argv[jsArgIndex]);

                if (!value) return JS_EXCEPTION;

                stringValues.emplace_back(value);

                JS_FreeCString(context, value);

                OpcodeCaller::PushVarToBuffer(stringValues.back().c_str());

                ++jsArgIndex;

                break;
            }

            case JavaScriptOpcodeType::IntPointer:
            {
                outputs.push_back({ JavaScriptOpcodeType::IntPointer });

                OpcodeCaller::PushVarToBuffer(&outputs.back().intValue);

                break;
            }

            case JavaScriptOpcodeType::FloatPointer:
            {
                outputs.push_back({ JavaScriptOpcodeType::FloatPointer });

                OpcodeCaller::PushVarToBuffer(&outputs.back().floatValue);

                break;
            }

            case JavaScriptOpcodeType::BoolPointer:
            {
                outputs.push_back({ JavaScriptOpcodeType::BoolPointer });

                OpcodeCaller::PushVarToBuffer(&outputs.back().boolStorage);

                break;
            }

            default:
            {
                return JS_ThrowInternalError(context, "Unsupported argument type for opcode %s", definition->name.c_str());
            }
        }
    }

    int returnOutputIndex = -1;

    switch (definition->returnType)
    {
        case JavaScriptOpcodeType::Int:
        {
            outputs.push_back({ JavaScriptOpcodeType::IntPointer });

            returnOutputIndex = static_cast<int>(outputs.size()) - 1;

            OpcodeCaller::PushVarToBuffer(&outputs.back().intValue);

            break;
        }

        case JavaScriptOpcodeType::Float:
        case JavaScriptOpcodeType::Double:
        {
            outputs.push_back({ JavaScriptOpcodeType::FloatPointer });

            returnOutputIndex = static_cast<int>(outputs.size()) - 1;

            OpcodeCaller::PushVarToBuffer(&outputs.back().floatValue);

            break;
        }

        case JavaScriptOpcodeType::Bool:
        case JavaScriptOpcodeType::Void: break;

        default:
        {
            return JS_ThrowInternalError(context, "Unsupported return type for opcode %s", definition->name.c_str());
        }
    }

    OpcodeCaller::ExecuteScriptBuf();

    for (int i = 0; i < OpcodeCaller::g_nVarsCount; ++i)
    {
        *OpcodeCaller::g_ScriptVarPointers[i] = OpcodeCaller::g_Script.m_aLocalVars[i].uParam;
    }

    for (auto& output : outputs)
    {
        if (output.type == JavaScriptOpcodeType::BoolPointer) { output.boolValue = output.boolStorage != 0; }
    }

    if (definition->returnType == JavaScriptOpcodeType::Void)
    {
        if (outputs.empty()) return JS_UNDEFINED;

        JSValue result = JS_NewArray(context);

        for (uint32_t i = 0; i < outputs.size(); ++i)
        {
            const Output& output = outputs[i];

            if (output.type == JavaScriptOpcodeType::IntPointer)
            {
                JS_SetPropertyUint32(context, result, i, JS_NewInt32(context, output.intValue));
            }
            else if (output.type == JavaScriptOpcodeType::FloatPointer)
            {
                JS_SetPropertyUint32(context, result, i, JS_NewFloat64(context, output.floatValue));
            }
            else if (output.type == JavaScriptOpcodeType::BoolPointer)
            {
                JS_SetPropertyUint32(context, result, i, JS_NewBool(context, output.boolValue));
            }
        }

        return result;
    }

    if (returnOutputIndex != -1)
    {
        const Output& output = outputs[returnOutputIndex];

        switch (definition->returnType)
        {
            case JavaScriptOpcodeType::Int: return JS_NewInt32(context, output.intValue);

            case JavaScriptOpcodeType::Float:
            case JavaScriptOpcodeType::Double: return JS_NewFloat64(context, output.floatValue);

            default: break;
        }
    }

    if (definition->returnType == JavaScriptOpcodeType::Bool) { return JS_NewBool(context, OpcodeCaller::g_Script.m_bCondResult); }

    return JS_UNDEFINED;
}

JSValue JavaScriptOpcode::MakeAlias(JSContext* context, JSValueConst thisValue, int argc, JSValueConst* argv)
{
    if (argc < 2) { return JS_ThrowTypeError(context, "MakeAlias(opcode, name)"); }

    int32_t opcode;

    if (JS_ToInt32(context, &opcode, argv[0]) < 0) return JS_EXCEPTION;

    const char* name = JS_ToCString(context, argv[1]);

    if (!name) return JS_EXCEPTION;

    const std::string aliasName = name;

    JS_FreeCString(context, name);

    if (!CreateAlias(static_cast<uint16_t>(opcode), aliasName))
    {
        return JS_ThrowInternalError(context, "Failed to create alias for opcode 0x%04X", opcode);
    }

    return JS_UNDEFINED;
}

bool JavaScriptOpcode::CreateAlias(uint16_t opcode, const std::string& name)
{
    if (!FindDefinition(opcode))
    {
        logger->Error("Cannot create alias %s: opcode 0x%04X not found", name.c_str(), opcode);

        return false;
    }

    for (auto& alias : _aliases)
    {
        if (alias.name != name) continue;

        alias.opcode = opcode;

        logger->Info("Alias replaced globally: %s -> 0x%04X", name.c_str(), opcode);

        return true;
    }

    JavaScriptOpcodeAlias alias;

    alias.opcode = opcode;
    alias.name = name;

    _aliases.push_back(std::move(alias));

    logger->Info("Alias registered globally: %s -> 0x%04X", name.c_str(), opcode);

    return true;
}

const JavaScriptOpcodeDefinition* JavaScriptOpcode::FindDefinition(uint16_t opcode)
{
    for (const auto& definition : _definitions)
    {
        if (definition.opcode == opcode) return &definition;
    }

    return nullptr;
}

int JavaScriptOpcode::GetArgumentCount(const JavaScriptOpcodeDefinition& definition)
{
    int count = 0;

    for (const auto type : definition.arguments)
    {
        if (type != JavaScriptOpcodeType::IntPointer && type != JavaScriptOpcodeType::FloatPointer &&
            type != JavaScriptOpcodeType::BoolPointer)
        {
            ++count;
        }
    }

    return count;
}