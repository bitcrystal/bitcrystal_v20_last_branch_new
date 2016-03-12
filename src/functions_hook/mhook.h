#ifndef MY_MHOOK_H
#define MY_MHOOK_H
//#define USE_SUBHOOK_IN_WINDOWS
#include "subhook/my_predef.h"
#if !defined(OS_WIN)
#define USE_SUBHOOK
#elif defined(USE_SUBHOOK_IN_WINDOWS)
#define USE_SUBHOOK
#else
#define USE_NOT_SUBHOOK
#endif
#ifdef USE_SUBHOOK
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
#else
	#include "cpu.h"
	#include "disasm_n.h"
	#include "mhook_lib/mhook-lib/mhook.h"
#endif
#endif