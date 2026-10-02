#include "OpcodeCaller_fixed.h"

#include "mod/logger.h"

inline void TestKeyPressed()
{
    OpcodeCaller::Reset();

    uint16_t opcode = 0x00E1;

    memcpy(OpcodeCaller::g_ScriptBuffer, &opcode, sizeof(opcode));

    OpcodeCaller::g_nBufferPos = 2;

    OpcodeCaller::PushVarToBuffer(5);

    int result = OpcodeCaller::ExecuteScriptBuf();

    logger->Info("[OPTEST 00E1] result=%d cond=%d", result, OpcodeCaller::g_Script.m_bCondResult);
}

inline void TestOpcode()
{
    uint32_t value = 0;

    logger->Info("[OPCODE 0DD5] BEFORE");

    OpcodeCaller::Reset();

    uint16_t opcode = 0x0DD5;
    memcpy(OpcodeCaller::g_ScriptBuffer, &opcode, sizeof(opcode));

    OpcodeCaller::g_nBufferPos = 2;

    // %1d% = output
    OpcodeCaller::PushVarToBuffer(&value);

    logger->Info("[OPCODE 0DD5] BUFFER READY pos=%d vars=%d", OpcodeCaller::g_nBufferPos, OpcodeCaller::g_nVarsCount);

    logger->Info("[BEFORE] IP=%p BASE=%p SP=%u active=%d cond=%d",
        OpcodeCaller::g_Script.m_pCurrentIP,
        OpcodeCaller::g_Script.m_pBaseIP,
        OpcodeCaller::g_Script.m_nSP,
        OpcodeCaller::g_Script.m_bIsActive,
        OpcodeCaller::g_Script.m_bCondResult);

    int result = OpcodeCaller::ExecuteScriptBuf();

    logger->Info("[AFTER] IP=%p BASE=%p SP=%u active=%d cond=%d result=%d",
        OpcodeCaller::g_Script.m_pCurrentIP,
        OpcodeCaller::g_Script.m_pBaseIP,
        OpcodeCaller::g_Script.m_nSP,
        OpcodeCaller::g_Script.m_bIsActive,
        OpcodeCaller::g_Script.m_bCondResult,
        result);

    logger->Info("[OPCODE 0DD5] EXECUTED result=%d cond=%d value=%u vars=%d",
        result,
        (int)OpcodeCaller::g_Script.m_bCondResult,
        value,
        OpcodeCaller::g_nVarsCount);
}

inline void RunTestLoop()
{
    //TestKeyPressed();
}

inline void TestOnce()
{
    int playerActor;

    Command<0x01F5>(0, &playerActor);

    Command<0x03FE>(playerActor, 100);

    logger->Info("[OPCODE 03FE] executed");
}