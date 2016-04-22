#include "mhook.h"
#if defined(USE_SUBHOOK)
#include "subhook/hookerpref.c"
void Mhook_MyInit()
{
	if(mhooks_bool_init==TRUE)
		return;
	int i;
	for(i = 0; i < MHOOKS_MAX_SUPPORTED_HOOKS; i++)
	{
		mhooks_subhooks[i]=NULL;
	}
	mhooks_bool_init=TRUE;
}

BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction) {
     PVOID * p = ppSystemFunction == NULL ? NULL : (PVOID*)&ppSystemFunction;
     return Mhook_SetHook(p,pHookFunction);
}

BOOL Mhook_UnhookEx(PVOID ppHookedFunction) {
     PVOID * p = ppHookedFunction == NULL ? NULL : (PVOID*)&ppHookedFunction;
     return Mhook_Unhook(p);
}


BOOL Mhook_SetHook(PVOID * ppSystemFunctionP, PVOID pHookFunction) {
	Mhook_MyInit();
	if(ppSystemFunctionP==NULL||pHookFunction==NULL)
	{
		return FALSE;
	}
	PVOID ppSystemFunction=(void*)(*ppSystemFunctionP);
	if(ppSystemFunction==NULL)
		return FALSE;
	if(mhooks_subhooks_count>=MHOOKS_MAX_SUPPORTED_HOOKS)
	{
		return FALSE;
	} else if(mhooks_subhooks_count<0)
	{
		mhooks_subhooks_count=0;
	}
	mhooks_subhooks[mhooks_subhooks_count]=subhook_new(ppSystemFunction, pHookFunction);
	if(mhooks_subhooks[mhooks_subhooks_count]==NULL)
	{
		return FALSE;
	}
	subhook_install(mhooks_subhooks[mhooks_subhooks_count]);
	if(subhook_is_installed(mhooks_subhooks[mhooks_subhooks_count]))
	{
		void * rr= subhook_get_trampoline(mhooks_subhooks[mhooks_subhooks_count]);
		if(rr==NULL)
		{
			subhook_remove(mhooks_subhooks[mhooks_subhooks_count]);
			subhook_free(mhooks_subhooks[mhooks_subhooks_count]);
			mhooks_subhooks[mhooks_subhooks_count]=NULL;
			return FALSE;
		}
		*ppSystemFunctionP=(void*)rr;
	}
	mhooks_subhooks_count++;
	return TRUE;
}
BOOL Mhook_Unhook(PVOID * ppHookedFunctionP) {
	Mhook_MyInit();
	if(ppHookedFunctionP==NULL)
		return FALSE;
	PVOID ppHookedFunction=(void*)(*ppHookedFunctionP);
	if(ppHookedFunction==NULL)
		return FALSE;
	int i;
	void * src=NULL;
	if(ppHookedFunction!=NULL)
	{
		for(i = 0; i < mhooks_subhooks_count; i++)
		{
			if(mhooks_subhooks[i]==NULL)
			{
				continue;
			}
			src=(void*)subhook_get_src(mhooks_subhooks[i]);
			if(src==ppHookedFunction)
			{
				*ppHookedFunctionP=(void*)src;
				subhook_remove(mhooks_subhooks[i]);
				subhook_free(mhooks_subhooks[i]);
				mhooks_subhooks[i]=NULL;
				if(i+1==mhooks_subhooks_count)
				{
					mhooks_subhooks_count--;
				}
				return TRUE;
			}
		}
		return FALSE;
	} else {
			i = mhooks_subhooks_count - 1;
			if(i<0)
			{
				return FALSE;
			}
			if(mhooks_subhooks[i]==NULL)
				return FALSE;
			src=(void*)subhook_get_src(mhooks_subhooks[i]);
			*ppHookedFunctionP=(void*)src;
			subhook_remove(mhooks_subhooks[i]);
			subhook_free(mhooks_subhooks[i]);
			mhooks_subhooks[i]=NULL;
			mhooks_subhooks_count--;
			return TRUE;
	}
	return FALSE;
}
#elif defined(USE_MHOOK)
	#include "cpu.c"
	#include "disasm_n.c"
	#include "mhook_lib/mhook-lib/mhook.c"
	 BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction) {
                PVOID * p = ppSystemFunction == NULL ? NULL : (PVOID*)&ppSystemFunction;
                return Mhook_SetHook(p,pHookFunction);
        }

        BOOL Mhook_UnhookEx(PVOID ppHookedFunction) {
                PVOID * p = ppHookedFunction == NULL ? NULL : (PVOID*)&ppHookedFunction;
                return Mhook_Unhook(p);
        }
	
	void Mhook_MyInit()
	{
		return;
	}
#elif defined(USE_POLYHOOK)
	#include "polyhook_lib/PolyHook/PolyHook_c_version.c"
	void Mhook_MyInit()
	{
		if(mhooks_bool_init==TRUE)
			return;
		int i;
		for(i = 0; i < MHOOKS_MAX_SUPPORTED_HOOKS; i++)
		{
			mhooks_subhooks[i]=NULL;
		}
		mhooks_bool_init=TRUE;
	}

	BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction) {
		PVOID * p = ppSystemFunction == NULL ? NULL : (PVOID*)&ppSystemFunction;
		return Mhook_SetHook(p,pHookFunction);
	}

	BOOL Mhook_UnhookEx(PVOID ppHookedFunction) {
		PVOID * p = ppHookedFunction == NULL ? NULL : (PVOID*)&ppHookedFunction;
		return Mhook_Unhook(p);
	}

	BOOL Mhook_SetHook(PVOID * ppSystemFunctionP, PVOID pHookFunction) {
		Mhook_MyInit();
		if(ppSystemFunctionP==NULL||pHookFunction==NULL)
		{
			return FALSE;
		}
		PVOID ppSystemFunction=(void*)(*ppSystemFunctionP);
		if(ppSystemFunction==NULL)
			return FALSE;
		if(mhooks_subhooks_count>=MHOOKS_MAX_SUPPORTED_HOOKS)
		{
			return FALSE;
		} else if(mhooks_subhooks_count<0)
		{
			mhooks_subhooks_count=0;
		}
		mhooks_subhooks[mhooks_subhooks_count]=PLH_ALL_S_t_new();
		if(mhooks_subhooks[mhooks_subhooks_count]==NULL)
		{
			return FALSE;
		}
		PLH__AbstractDetour__init(mhooks_subhooks[mhooks_subhooks_count]);
		if(mhooks_subhooks[mhooks_subhooks_count]->last_error!=CS_ERR_OK)
		{
			PLH_ALL_S_t_delete(mhooks_subhooks[mhooks_subhooks_count]);
			mhooks_subhooks[mhooks_subhooks_count]=NULL;
			return FALSE;
		}
		PLH__AbstractDetour__SetupHook(mhooks_subhooks[mhooks_subhooks_count],(BYTE*)ppSystemFunction,(BYTE*)pHookFunction);
		BOOL is_installed = PLH__AbstractDetour__Hook(mhooks_subhooks[mhooks_subhooks_count]);
		if(is_installed)
		{
			void * rr= (void*)PLH__MyDetour__GetTrampoline(mhooks_subhooks[mhooks_subhooks_count]);
			if(rr==NULL)
			{
				PLH__AbstractDetour__UnHook(mhooks_subhooks[mhooks_subhooks_count]);
				PLH_ALL_S_t_delete(mhooks_subhooks[mhooks_subhooks_count]);
				mhooks_subhooks[mhooks_subhooks_count]=NULL;
				return FALSE;
			}
			*ppSystemFunctionP=(void*)rr;
		} else {
			return FALSE;
		}
		mhooks_subhooks_count++;
		return TRUE;
	}
		
	BOOL Mhook_Unhook(PVOID * ppHookedFunctionP) {
		Mhook_MyInit();
		if(ppHookedFunctionP==NULL)
			return FALSE;
		PVOID ppHookedFunction=(void*)(*ppHookedFunctionP);
		if(ppHookedFunction==NULL)
			return FALSE;
		int i;
		void * src=NULL;
		if(ppHookedFunction!=NULL)
		{
			for(i = 0; i < mhooks_subhooks_count; i++)
			{
				if(mhooks_subhooks[i]==NULL)
				{
					continue;
				}
				src=(void*)PLH__MyDetour__GetHkSrc(mhooks_subhooks[i]);
				if(src==ppHookedFunction)
				{
					*ppHookedFunctionP=(void*)src;
					PLH__AbstractDetour__UnHook(mhooks_subhooks[i]);
					PLH_ALL_S_t_delete(mhooks_subhooks[mhooks_subhooks_count]);
					mhooks_subhooks[mhooks_subhooks_count]=NULL;
					if(i+1==mhooks_subhooks_count)
					{
						mhooks_subhooks_count--;
					}
					return TRUE;
				}
			}
			return FALSE;
		} else {
			i = mhooks_subhooks_count - 1;
			if(i<0)
			{
				return FALSE;
			}
			if(mhooks_subhooks[i]==NULL)
				return FALSE;
			src=(void*)PLH__MyDetour__GetHkSrc(mhooks_subhooks[i]);
			*ppHookedFunctionP=(void*)src;
			PLH__AbstractDetour__UnHook(mhooks_subhooks[i]);
			PLH_ALL_S_t_delete(mhooks_subhooks[i]);
			mhooks_subhooks[i]=NULL;
			mhooks_subhooks_count--;
			return TRUE;
		}
		return FALSE;
	}
#else
	BOOL Mhook_SetHook(PVOID * ppSystemFunctionP, PVOID pHookFunction) {
		return FALSE;
	}
		
	BOOL Mhook_Unhook(PVOID * ppHookedFunctionP) {
		return FALSE;
	}
		
	BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction) {
		return FALSE;
	}
		
	BOOL Mhook_UnhookEx(PVOID ppHookedFunction) {
		return FALSE;
	}
	
	void Mhook_MyInit()
	{
		return;
	}
#endif
