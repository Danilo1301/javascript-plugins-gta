#include "javaScriptEngine.h"

#include "javaScriptBindings.h"
#include "javaScriptOpcode.h"
#include "logUtils.h"
#include "mod/logger.h"
#include "renderSystem.h"

#include <dirent.h>
#include <fstream>
#include <regex>
#include <sstream>

JavaScriptEngine::JavaScriptEngine()
{
}

JavaScriptEngine::~JavaScriptEngine()
{
    Shutdown();
}

bool JavaScriptEngine::Initialize()
{
    _runtime = JS_NewRuntime();

    if (!_runtime) return false;

    return true;
}

void JavaScriptEngine::Shutdown()
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto& script : _scripts)
    {
        auto* data = static_cast<JavaScriptContextData*>(JS_GetContextOpaque(script.context));

        delete data;

        JS_FreeContext(script.context);
        script.context = nullptr;
    }

    _scripts.clear();

    if (_runtime)
    {
        JS_FreeRuntime(_runtime);
        _runtime = nullptr;
    }

    _nextScriptId = 0;
}

bool JavaScriptEngine::Execute(JSContext* context, const char* code, const char* filename)
{
    if (!context) return false;

    JS_UpdateStackTop(_runtime);

    JSValue result = JS_Eval(context, code, strlen(code), filename, JS_EVAL_TYPE_GLOBAL);

    if (JS_IsException(result))
    {
        LogException(context, filename);
        JS_FreeValue(context, result);

        return false;
    }

    JS_FreeValue(context, result);

    return true;
}

bool JavaScriptEngine::ExecuteFile(JSContext* context, const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        logger->Error("Failed to open JavaScript file: %s", path.c_str());

        return false;
    }

    std::ostringstream stream;
    stream << file.rdbuf();

    std::string code = stream.str();

    static const std::regex moduleMarker(R"(^\s*export\s*\{\s*\}\s*;?)");

    code = std::regex_replace(code, moduleMarker, "");

    return Execute(context, code.c_str(), path.c_str());
}

bool JavaScriptEngine::CallScriptFunction(JavaScriptScript& script, const std::string& functionName)
{
    return CallScriptFunction(script, functionName, 0, nullptr);
}

bool JavaScriptEngine::CallScriptFunction(JavaScriptScript& script, const std::string& functionName, int argc, JSValueConst* argv)
{
    if (!script.context) return false;

    BEGIN_OPERATION_DESC(op_Script_callFunction, functionName);

    JSContext* context = script.context;

    JS_UpdateStackTop(_runtime);

    JSValue globalObject = JS_GetGlobalObject(context);

    JSValue callback = JS_GetPropertyStr(context, globalObject, functionName.c_str());

    if (!JS_IsFunction(context, callback))
    {
        JS_FreeValue(context, callback);
        JS_FreeValue(context, globalObject);

        END_OPERATION(op_Script_callFunction);

        return false;
    }

    RenderSystem::BeginScript(script.id);

    JSValue result = JS_Call(context, callback, JS_UNDEFINED, argc, argv);

    RenderSystem::EndScript(script.id);

    if (JS_IsException(result))
    {
        LogException(context, functionName);

        JS_FreeValue(context, result);
        JS_FreeValue(context, callback);
        JS_FreeValue(context, globalObject);

        END_OPERATION(op_Script_callFunction);

        return false;
    }

    JS_FreeValue(context, result);
    JS_FreeValue(context, callback);
    JS_FreeValue(context, globalObject);

    END_OPERATION(op_Script_callFunction);

    return true;
}

void JavaScriptEngine::LoadScripts(const std::string& folder)
{
    DIR* directory = opendir(folder.c_str());

    if (!directory)
    {
        logger->Error("Failed to open JavaScript folder: %s", folder.c_str());

        return;
    }

    struct dirent* entry;

    while ((entry = readdir(directory)) != nullptr)
    {
        const std::string name = entry->d_name;

        if (name == "." || name == "..") continue;

        if (name.size() < 3 || name.substr(name.size() - 3) != ".js") { continue; }

        const std::string path = folder + "/" + name;

        logger->Info("Loading JavaScript: %s", path.c_str());

        JSContext* context = JS_NewContext(_runtime);

        if (!context)
        {
            logger->Error("Failed to create JavaScript context: %s", path.c_str());

            continue;
        }

        JavaScriptScript script;

        script.id = _nextScriptId++;
        script.path = path;
        script.context = context;
        script.name = name;

        JavaScriptBindings::Register(script.context, script.id);
        JavaScriptOpcode::Initialize(context);

        if (!ExecuteFile(context, path))
        {
            logger->Error("Failed to load JavaScript: %s", path.c_str());

            JS_FreeContext(context);
            continue;
        }

        _scripts.push_back(std::move(script));
    }

    closedir(directory);
}

void JavaScriptEngine::CallFunction(const std::string& functionName)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_runtime) return;

    for (auto& script : _scripts) { CallScriptFunction(script, functionName); }
}

void JavaScriptEngine::RegisterOpcodeContexts()
{
    for (auto& script : _scripts) { JavaScriptOpcode::RegisterContext(script.context); }
}

void JavaScriptEngine::OnScriptCalled(const std::string& data)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!_runtime) return;

    for (auto& script : _scripts)
    {
        JSContext* context = script.context;

        JS_UpdateStackTop(_runtime);

        JSValue dataValue = JS_NewString(context, data.c_str());

        CallScriptFunction(script, "onScriptCalled", 1, &dataValue);

        JS_FreeValue(context, dataValue);
    }
}

void JavaScriptEngine::LogException(JSContext* context, const std::string& functionName)
{
    JSValue exception = JS_GetException(context);

    const char* error = JS_ToCString(context, exception);

    if (error)
    {
        logger->Error("%s: %s", functionName.c_str(), error);

        JS_FreeCString(context, error);
    }

    JSValue stack = JS_GetPropertyStr(context, exception, "stack");

    if (!JS_IsUndefined(stack))
    {
        const char* stackString = JS_ToCString(context, stack);

        if (stackString)
        {
            logger->Error("JS Stack:\n%s", stackString);

            JS_FreeCString(context, stackString);
        }
    }

    JS_FreeValue(context, stack);
    JS_FreeValue(context, exception);
}

std::vector<JavaScriptScript> JavaScriptEngine::GetScripts()
{
    return _scripts;
}