#pragma once

#include "quickjs/quickjs.h"

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

struct JavaScriptScript
{
    uint32_t id = 0;
    std::string path;
    std::string name;

    JSContext* context = nullptr;
};

class JavaScriptEngine
{
public:
    JavaScriptEngine();
    ~JavaScriptEngine();

    bool Initialize();
    void Shutdown();

    void LoadScripts(const std::string& folder);

    void OnScriptCalled(const std::string& data);
    void CallFunction(const std::string& functionName);

    bool CallScriptFunction(JavaScriptScript& script, const std::string& functionName);
    bool CallScriptFunction(JavaScriptScript& script, const std::string& functionName, int argc, JSValueConst* argv);

    void RegisterOpcodeContexts();

    std::vector<JavaScriptScript> GetScripts();

private:
    bool Execute(JSContext* context, const char* code, const char* filename);
    bool ExecuteFile(JSContext* context, const std::string& path);

    void LogException(JSContext* context, const std::string& functionName);

private:
    JSRuntime* _runtime = nullptr;

    std::mutex _mutex;

    std::mutex _mainThreadQueueMutex;
    std::queue<std::function<void()>> _mainThreadQueue;

    std::vector<JavaScriptScript> _scripts;

    uint32_t _nextScriptId = 0;
};