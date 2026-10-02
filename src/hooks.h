#pragma once

#include "aml-psdk/game_sa/Events.h"
#include "aml-psdk/gta_base/RGBA.h"
#include "aml-psdk/gta_base/Rect.h"
#include "mod/logger.h"

inline void* hGTASA = nullptr;
inline uintptr_t pGTASA;

inline void (*CSprite2d_DrawRect)(CRect const& posn, CRGBA const& color);

inline void (*FontSetOrientation)(unsigned char);
inline void (*FontSetColor)(CRGBA*);
inline void (*FontSetBackground)(unsigned char, unsigned char);
inline void (*FontSetWrapx)(float);
inline void (*FontSetStyle)(unsigned char);
inline void (*FontSetScale)(float w, float h);
inline void (*FontSetProportional)(unsigned char);
inline void (*FontSetDropShadowPosition)(char);
inline void (*FontPrintString)(float, float, unsigned short*);
//inline void (*AsciiToGxtChar)(const char* txt, unsigned short* ret);
inline void (*RenderFontBuffer)(void);

void DoHooks2();

inline void DoHooks()
{
    logger->Info("Hooking...");

    hGTASA = dlopen("libGTASA.so", RTLD_LAZY);
    pGTASA = aml->GetLib("libGTASA.so");

    SET_TO(CSprite2d_DrawRect, aml->GetSym(hGTASA, "_ZN9CSprite2d8DrawRectERK5CRectRK5CRGBA"));
    SET_TO(FontSetOrientation, aml->GetSym(hGTASA, "_ZN5CFont14SetOrientationEh"));
    SET_TO(FontSetColor, aml->GetSym(hGTASA, "_ZN5CFont8SetColorE5CRGBA"));
    SET_TO(FontSetBackground, aml->GetSym(hGTASA, "_ZN5CFont13SetBackgroundEhh"));
    SET_TO(FontSetWrapx, aml->GetSym(hGTASA, "_ZN5CFont8SetWrapxEf"));
    SET_TO(FontSetStyle, aml->GetSym(hGTASA, "_ZN5CFont12SetFontStyleEh"));
    SET_TO(FontSetScale, aml->GetSym(hGTASA, "_ZN5CFont8SetScaleEf"));
    SET_TO(FontSetProportional, aml->GetSym(hGTASA, "_ZN5CFont15SetProportionalEh"));
    SET_TO(FontSetDropShadowPosition, aml->GetSym(hGTASA, "_ZN5CFont21SetDropShadowPositionEa"));
    SET_TO(FontPrintString, aml->GetSym(hGTASA, "_ZN5CFont11PrintStringEffPt"));
    //SET_TO(AsciiToGxtChar, aml->GetSym(hGTASA, "_Z14AsciiToGxtCharPKcPt"));
    SET_TO(RenderFontBuffer, aml->GetSym(hGTASA, "_ZN5CFont16RenderFontBufferEv"));

    DoHooks2();

    logger->Info("Hooks ok");
}