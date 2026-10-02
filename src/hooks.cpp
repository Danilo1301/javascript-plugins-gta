#include "hooks.h"
#include "mod/iaml.h"

extern void Render();

DECL_HOOK(void, PreRenderEnd, void* self)
{
    PreRenderEnd(self);

    Render();
}

void DoHooks2()
{
    HOOK(PreRenderEnd, aml->GetSym(hGTASA, "_ZN6CDebug22DebugDisplayTextBufferEv"));
}