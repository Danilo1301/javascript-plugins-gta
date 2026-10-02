#pragma once

#include "OpcodeCaller_fixed.h"
#include "mod/logger.h"

namespace OpcodeDiagnostic
{
inline void DumpBuffer(const char* name)
{
    char buffer[256];
    int pos = 0;

    for (int i = 0; i < OpcodeCaller::g_nBufferPos && pos < 240; ++i)
    {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%02X ", OpcodeCaller::g_ScriptBuffer[i]);
    }

    logger->Info("[OPTEST %s] BUFFER size=%d: %s", name, OpcodeCaller::g_nBufferPos, buffer);
}

inline void DumpScript(const char* stage)
{
    auto& script = OpcodeCaller::g_Script;

    ptrdiff_t ipOffset = 0;
    ptrdiff_t baseOffset = 0;

    if (script.m_pBaseIP) { ipOffset = script.m_pCurrentIP - script.m_pBaseIP; }

    logger->Info("[OPTEST %s] "
                 "IP=%p BASE=%p IP_OFFSET=%td "
                 "SP=%u ACTIVE=%d COND=%d "
                 "LOGICAL=%u NOT=%d "
                 "WAKE=%d "
                 "WASTED_CHECK=%d WASTED=%d "
                 "MISSION=%d",
        stage,
        script.m_pCurrentIP,
        script.m_pBaseIP,
        ipOffset,
        script.m_nSP,
        (int)script.m_bIsActive,
        (int)script.m_bCondResult,
        script.m_nLogicalOp,
        (int)script.m_bNotFlag,
        script.m_nWakeTime,
        (int)script.m_bWastedBustedCheck,
        (int)script.m_bWastedOrBusted,
        (int)script.m_bIsMission);
}

inline void DumpLocals(const char* stage)
{
    auto& script = OpcodeCaller::g_Script;

    logger->Info("[OPTEST %s] LOCALS:", stage);

    for (int i = 0; i < 8; ++i)
    {
        logger->Info("[OPTEST %s] local[%d] = 0x%08X (%u)", stage, i, script.m_aLocalVars[i].uParam, script.m_aLocalVars[i].uParam);
    }
}

inline void Prepare(uint16_t opcode)
{
    OpcodeCaller::Reset();

    memcpy(OpcodeCaller::g_ScriptBuffer, &opcode, sizeof(opcode));

    OpcodeCaller::g_nBufferPos = 2;
}

inline void Execute(const char* name)
{
    DumpBuffer(name);
    DumpScript("BEFORE");

    logger->Info("[OPTEST %s] vars=%d bufferPos=%d", name, OpcodeCaller::g_nVarsCount, OpcodeCaller::g_nBufferPos);

    int result = OpcodeCaller::ExecuteScriptBuf();

    logger->Info("[OPTEST %s] ExecuteScriptBuf returned %d", name, result);

    DumpScript("AFTER");
    DumpLocals("AFTER");
}

/*
     * 0DD5=1
     * %1d% = get_platform
     *
     * output only
     */
inline void Test_0DD5()
{
    logger->Info("[OPTEST 0DD5] ===============================");
    logger->Info("[OPTEST 0DD5] get_platform");

    uint32_t value = 0xDEADBEEF;

    Prepare(0x0DD5);

    OpcodeCaller::PushVarToBuffer(&value);

    Execute("0DD5");

    logger->Info("[OPTEST 0DD5] OUTPUT value=0x%08X (%u)", value, value);

    logger->Info("[OPTEST 0DD5] ===============================");
}

/*
     * 0DD6=1
     * %1d% = get_game_version
     *
     * output only
     */
inline void Test_0DD6()
{
    logger->Info("[OPTEST 0DD6] ===============================");
    logger->Info("[OPTEST 0DD6] get_game_version");

    uint32_t value = 0xDEADBEEF;

    Prepare(0x0DD6);

    OpcodeCaller::PushVarToBuffer(&value);

    Execute("0DD6");

    logger->Info("[OPTEST 0DD6] OUTPUT value=0x%08X (%u)", value, value);

    logger->Info("[OPTEST 0DD6] ===============================");
}

/*
     * 0DD7=1
     * %1d% = get_image_base
     *
     * output only
     */
inline void Test_0DD7()
{
    logger->Info("[OPTEST 0DD7] ===============================");
    logger->Info("[OPTEST 0DD7] get_image_base");

    uint32_t value = 0xDEADBEEF;

    Prepare(0x0DD7);

    OpcodeCaller::PushVarToBuffer(&value);

    Execute("0DD7");

    logger->Info("[OPTEST 0DD7] OUTPUT value=0x%08X (%u)", value, value);

    logger->Info("[OPTEST 0DD7] ===============================");
}

/*
     * 0DDC=2
     * set_mutex_var %1d% to %2d%
     *
     * input only
     */
inline void Test_0DDC()
{
    logger->Info("[OPTEST 0DDC] ===============================");
    logger->Info("[OPTEST 0DDC] set_mutex_var 123 = 456");

    Prepare(0x0DDC);

    OpcodeCaller::PushVarToBuffer(123);
    OpcodeCaller::PushVarToBuffer(456);

    Execute("0DDC");

    logger->Info("[OPTEST 0DDC] ===============================");
}

/*
     * 0DDD=2
     * %1d% = get_mutex_var %2d%
     *
     * output + input
     */
inline void Test_0DDD()
{
    logger->Info("[OPTEST 0DDD] ===============================");
    logger->Info("[OPTEST 0DDD] get_mutex_var 123");

    uint32_t value = 0xDEADBEEF;

    Prepare(0x0DDD);

    OpcodeCaller::PushVarToBuffer(&value);
    OpcodeCaller::PushVarToBuffer(123);

    Execute("0DDD");

    logger->Info("[OPTEST 0DDD] OUTPUT value=0x%08X (%u)", value, value);

    logger->Info("[OPTEST 0DDD] ===============================");
}

inline void Run()
{
    /*
         * Começa com os mais simples.
         */
    Test_0DD5();
    Test_0DD6();
    Test_0DD7();

    /*
         * Depois input-only.
         */
    Test_0DDC();

    /*
         * Finalmente output + input.
         */
    Test_0DDD();
}
} // namespace OpcodeDiagnostic