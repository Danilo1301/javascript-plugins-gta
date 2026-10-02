#pragma once

#include "interfaces.h"

enum PluginOperation : unsigned int
{
    TEST_OP = 165123,
    op_CallOpcode,
    op_ProcessScripts,
    op_OnDraw,
    op_Script_callFunction
};

#define BEGIN_OPERATION(operation) menuSZK->BeginOperation(operation, "JavascriptPlugins_" #operation, "")
#define BEGIN_OPERATION_DESC(operation, description) menuSZK->BeginOperation(operation, "JavascriptPlugins_" #operation, description)
#define END_OPERATION(operation) menuSZK->EndOperation(operation)
#define END_OPERATION_RESULT(operation, result) menuSZK->EndOperation(operation, result)