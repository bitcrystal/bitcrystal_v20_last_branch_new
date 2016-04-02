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
void Mhook_MyInit();
BOOL Mhook_SetHook(PVOID *ppSystemFunctionP, PVOID pHookFunction);
BOOL Mhook_Unhook(PVOID *ppHookedFunctionP);
BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction);
BOOL Mhook_UnhookEx(PVOID ppHookedFunction);

#define MHOOKS_MAX_SUPPORTED_HOOKS	64
//Old Hooks
subhook_t mhooks_subhooks[MHOOKS_MAX_SUPPORTED_HOOKS];
BOOL mhooks_bool_init = FALSE;
int mhooks_subhooks_count = 0;
#else
	#ifdef OS_WINX
		#include "cpu.h"
		#include "disasm_n.h"
		#include "mhook_lib/mhook-lib/mhook.h"
	#elif defined(OS_UNIX_STRUCT)||defined(OS_WIN)
		#include "polyhook_lib/PolyHook/PolyHook_c_version.h"
		#define MHOOKS_MAX_SUPPORTED_HOOKS	64
		//Old Hooks
		PLH_ALL_S_t mhooks_subhooks[MHOOKS_MAX_SUPPORTED_HOOKS];
		BOOL mhooks_bool_init = FALSE;
		int mhooks_subhooks_count = 0; 
	#endif
	void Mhook_MyInit();
	BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction);
	BOOL Mhook_UnhookEx(PVOID ppHookedFunction);
#endif
#endif