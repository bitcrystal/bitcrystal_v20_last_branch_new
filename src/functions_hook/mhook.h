#ifndef MY_MHOOK_H
#define MY_MHOOK_H
#include "subhook/subhook.h"
void Mhook_MyInit();
typedef void * PVOID;
typedef int                 BOOL;
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif
//Old Hooks
#ifdef _M_IX86
#define _M_IX86_X64
#elif defined _M_X64
#define _M_IX86_X64
#endif
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
//#define NULL ( (void *) 0)
#define NULL 0
#endif