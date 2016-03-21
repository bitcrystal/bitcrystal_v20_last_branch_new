#include "windows_defs.h"
#ifndef OS_WIN
#define IS_NOT_WINDOWS
#ifdef IS_NOT_WINDOWS
#ifndef IS_LINUX_OS_DEFINED
	#ifdef __linux__
		#define IS_LINUX_OS_DEFINED;
	#endif
#endif
#endif
#ifdef IS_LINUX_OS_DEFINED
#include "linux_os.h"
#include "linux_os_2.h
#endif
LPVOID WINAPI VirtualAlloc(LPVOID lpAddress,SIZE_T dwSize,DWORD flAllocationType,DWORD flProtect)
{
	#ifdef IS_LINUX_OS_DEFINED
		void * address = mmap((void*)lpAddress, (size_t)dwSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		return (LPVOID)address;
	#endif
	return NULL;
}
BOOL WINAPI VirtualProtect(LPVOID lpAddress,SIZE_T dwSize,DWORD  flNewProtect,PDWORD lpflOldProtect) {
	#ifdef IS_LINUX_OS_DEFINED
		return TRUE;
	#endif
	return FALSE;
}
SIZE_T WINAPI VirtualQuery(LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength) {
	#ifdef IS_LINUX_OS_DEFINED
		vector_c free_blocks;
		int r = GET_FREE_BLOCKS_EX(lpAddress, 0, (&free_blocks));
		if(r==0)
		{
			return (SIZE_T)r;
		}
		VECTOR_C_CAP_TYPE total = VECTOR_TOTAL_C(free_blocks);
		VECTOR_C_CAP_TYPE i;
		struct_saver * ss;
		my_memory_block * mem;
		for(i=0; i < total; i++)
		{
			ss = VECTOR_GET_C_STRUCT(free_blocks,i);
			mem = (my_memory_block*)ss->item;
			
		}
		my_memory_block blockx;
		r = TO_ONE_BLOCK_EX((&free_blocks),(&blockx));
		if(r==0)
		{
			return (SIZE_T)r;
		}
		
	#endif
	return (SIZE_T)0;
}
#endif