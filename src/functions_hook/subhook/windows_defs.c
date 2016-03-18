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
#endif
LPVOID WINAPI VirtualAlloc(LPVOID lpAddress,SIZE_T dwSize,DWORD flAllocationType,DWORD flProtect)
{
	#ifdef IS_LINUX_OS_DEFINED
		void * address = mmap((void*)lpAddress, (size_t)dwSize, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
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
		if (maps_init (&mi, getpid()) < 0)
			return (SIZE_T)0;

		unsigned long offset;
		while (maps_next (&mi, &low, &hi, &offset)) {
		struct dl_phdr_info info;
		info.dlpi_name = mi.path;
		info.dlpi_addr = low;
			return (SIZE_T)1;
	#endif
	return (SIZE_T)0;
}
#endif