#include "mhook.h"
void Mhook_MyInit()
{
	if(mhooks_bool_init==TRUE)
		return;
	int i;
	for(i = 0; i < MHOOKS_MAX_SUPPORTED_HOOKS; i++)
	{
		mhooks_subhooks__[i]=NULL;
		mhooks_subhooks[i]=NULL;
	}
	mhooks_bool_init=TRUE;
}

BOOL Mhook_SetHook(PVOID *ppSystemFunction, PVOID pHookFunction) {
	return Mhook_SetHookEx(*ppSystemFunction,pHookFunction);
}

BOOL Mhook_Unhook(PVOID * ppHookedFunction) {
	return Mhook_UnhookEx(*ppHookedFunction);
}

BOOL Mhook_SetHookEx(PVOID ppSystemFunction, PVOID pHookFunction) {
	Mhook_MyInit();
	if(ppSystemFunction==NULL||pHookFunction==NULL)
	{
		return FALSE;
	}
	if(mhooks_subhooks_count>=MHOOKS_MAX_SUPPORTED_HOOKS)
	{
		return FALSE;
	} else if(mhooks_subhooks_count<0)
	{
		mhooks_subhooks_count=0;
	}
	mhooks_subhooks__[mhooks_subhooks_count]=ppSystemFunction;
	mhooks_subhooks[mhooks_subhooks_count]=subhook_new(ppSystemFunction, pHookFunction);
	subhook_install(mhooks_subhooks[mhooks_subhooks_count]);
	mhooks_subhooks_count++;
	return TRUE;
}
BOOL Mhook_UnhookEx(PVOID ppHookedFunction) {
	Mhook_MyInit();
	int i;
	if(ppHookedFunction!=NULL)
	{
		for(i = 0; i < mhooks_subhooks_count; i++)
		{
			if(mhooks_subhooks__[i]==NULL)
			{
				continue;
			}
			if(mhooks_subhooks__[i]==ppHookedFunction)
			{
				subhook_remove(mhooks_subhooks[i]);
				subhook_free(mhooks_subhooks[i]);
				mhooks_subhooks__[i]=NULL;
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
			subhook_remove(mhooks_subhooks[i]);
			subhook_free(mhooks_subhooks[i]);
			mhooks_subhooks__[i]=NULL;
			mhooks_subhooks_count--;
			return TRUE;
	}
	return FALSE;
}