#ifndef MY_MHOOK_H
#define MY_MHOOK_H
#include "subhook/hookerpref.h"
BOOL Mhook_SetHook(PVOID *ppSystemFunction, PVOID pHookFunction);
BOOL Mhook_Unhook(PVOID *ppHookedFunction);
BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction);
BOOL Mhook_UnhookEx(PVOID ppHookedFunction);

#define MHOOKS_MAX_SUPPORTED_HOOKS	64
//Old Hooks
static subhook_t mhooks_subhooks[MHOOKS_MAX_SUPPORTED_HOOKS];
static PVOID mhooks_subhooks__[MHOOKS_MAX_SUPPORTED_HOOKS];
static BOOL mhooks_bool_init = FALSE;
static int mhooks_subhooks_count = 0;
#endif