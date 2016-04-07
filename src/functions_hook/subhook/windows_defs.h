#ifndef WINDOWS_DEFS_H
#define WINDOWS_DEFS_H
#include "my_predef.h"
#ifndef OS_WIN
typedef void * PVOID;
typedef int BOOL;
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif
#ifdef _M_IX86
#define _M_IX86_X64
#elif defined _M_X64
#define _M_IX86_X64
#endif
//#define NULL ( (void *) 0)
#define NULL 0
#define __int64 long long
#define CONST const
typedef CONST void *LPCVOID;
typedef void *LPVOID;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef char CHAR;
typedef char CCHAR;
typedef long LONG;
typedef LONG *PLONG;
typedef DWORD COLORREF;
typedef BYTE BOOLEAN;
typedef unsigned __int64 DWORDLONG;
#if defined(_WIN64)
 typedef __int64 LONG_PTR; 
#else
 typedef long LONG_PTR;
#endif
typedef LONG_PTR LPARAM;
#if defined(_WIN64)
 typedef unsigned __int64 ULONG_PTR;
#else
 typedef unsigned long ULONG_PTR;
#endif
typedef ULONG_PTR SIZE_T;
typedef ULONG_PTR DWORD_PTR;
typedef unsigned __int64 DWORD64, *PDWORD64;
#if !defined(_M_IX86)
 typedef unsigned __int64 ULONGLONG;
#else
 typedef double ULONGLONG;
#endif
#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ ULONG_PTR
#endif
typedef __SIZE_TYPE__ size_t;
typedef SIZE_T size_t_in_windows;
typedef unsigned int ULONG32;
typedef unsigned long _ULONG32;
typedef unsigned __int64 ULONG64;
typedef long _LONG32;
typedef signed int LONG32;
typedef __int64 LONG64;
typedef short SHORT;
typedef unsigned short USHORT;
typedef CHAR *LPSTR;
typedef CONST CHAR *PCSTR;
typedef CHAR *PSTR;
typedef USHORT WCHAR;
typedef WCHAR *PWSTR;
typedef WCHAR *LPWSTR;
typedef DWORD *PDWORD;
typedef DWORD *LPDWORD;
typedef PVOID HANDLE;
typedef HANDLE *LPHANDLE;
#ifndef _UNICODE
#ifdef UNICODE 
#define _UNICODE UNICODE
#endif
#else
#ifndef UNICODE
#define UNICODE _UNICODE
#endif
#endif

#ifdef UNICODE
 typedef WCHAR TBYTE;
#else
 typedef unsigned char TBYTE;
#endif

#ifdef UNICODE
 typedef LPWSTR PTSTR;
#else
typedef LPSTR PTSTR;
#endif
#ifdef UNICODE
 typedef LPWSTR LPTSTR;
#else
 typedef LPSTR LPTSTR;
#endif
typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING;
typedef CONST CHAR *LPCSTR;
typedef CONST WCHAR *LPCWSTR;
#ifdef UNICODE
 typedef LPCWSTR LPCTSTR; 
#else
 typedef LPCSTR LPCTSTR;
#endif
#define MAX_PATH             260
#define INVALID_HANDLE_VALUE ((HANDLE) -1)
#define INVALID_FILE_SIZE    ((DWORD)0xFFFFFFFF)
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#ifdef _UNICODE
#define L(str) L##str
#else
#define L(str) str
#endif
#define __stdcall
#define CALLBACK __stdcall
#define WINAPI __stdcall
typedef struct _MEMORY_BASIC_INFORMATION {
  PVOID  BaseAddress;
  PVOID  AllocationBase;
  DWORD  AllocationProtect;
  SIZE_T RegionSize;
  DWORD  State;
  DWORD  Protect;
  DWORD  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
typedef union ____Win32Helper___NUM_BASE
{
	char value_char[4];
	unsigned char u_value_char[4];
	_LONG32 value : 32;
	_ULONG32 u_value : 32;
} ___Win32Helper___NUM_BASE;
typedef ___Win32Helper___NUM_BASE MY_NUM_BASE;

typedef union ____Win32Helper___NUM64_BASE
{
	char value_char[8];
	unsigned char u_value_char[8];
	LONG64 value : 64;
	ULONG64 u_value : 64;
} ___Win32Helper___NUM64_BASE;
typedef ___Win32Helper___NUM64_BASE MY_NUM64_BASE;
#ifndef CO_MODE
#if defined(OS_UNIX_STRUCT)
#define CO_MODE
#endif
#endif
#ifdef CO_MODE
/*#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>*/
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include "linux_defs.h"
#if defined(ENOMEM) && !defined(EFAULT)
#define EFAULT ENOMEM
#elif defined(EFAULT) && !defined(ENOMEM)
#define ENOMEM EFAULT
#endif
#ifndef MAP_ANON
#ifdef MAP_ANONYMOUS
#define MAP_ANON MAP_ANONYMOUS
#endif
#endif
#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

#endif

#ifndef MY__PROT_READ

#if defined(VM_PROT_READ)
#define MY__PROT_READ VM_PROT_READ
#elif defined(B_READ_AREA)
#define MY__PROT_READ B_READ_AREA
#elif defined(MA_READ)
#define MY__PROT_READ MA_READ
#elif defined(PROT_READ)
#define MY__PROT_READ PROT_READ
#else
#define MY__PROT_READ 1
#endif

#endif

#ifndef MY__PROT_WRITE

#if defined(VM_PROT_WRITE)
#define MY__PROT_WRITE VM_PROT_WRITE
#elif defined(B_WRITE_AREA)
#define MY__PROT_WRITE B_WRITE_AREA
#elif defined(MA_WRITE)
#define MY__PROT_WRITE MA_WRITE
#elif defined(PROT_WRITE)
#define MY__PROT_WRITE PROT_WRITE
#else
#define MY__PROT_WRITE 2
#endif
#endif

#ifndef MY__PROT_EXEC

#if defined(VM_PROT_EXECUTE)
#define MY__PROT_EXEC VM_PROT_EXECUTE
#elif defined(B_WRITE_AREA)
#define MY__PROT_WRITE B_WRITE_AREA
#elif defined(MA_EXEC)
#define MY__PROT_EXEC MA_EXEC
#elif defined(PROT_EXEC)
#define MY__PROT_EXEC PROT_EXEC
#else
#define MY__PROT_EXEC 4
#endif

#endif

#ifndef MY__PROT_NONE

#if defined(VM_PROT_NONE)
#define MY__PROT_NONE VM_PROT_NONE
#elif defined(B_NONE_AREA)
#define MY__PROT_NONE B_NONE_AREA
#elif defined(MA_NONE)
#define MY__PROT_NONE MA_NONE
#elif defined(PROT_NONE)
#define MY__PROT_NONE PROT_NONE
#else
#define MY__PROT_NONE 0
#endif

#endif

#ifndef PROT_NONE
#define PROT_NONE MY__PROT_NONE
#endif
#ifndef PROT_READ
#define PROT_READ MY__PROT_READ
#endif
#ifndef PROT_WRITE
#define PROT_WRITE MY__PROT_WRITE
#endif
#ifndef PROT_EXEC
#define PROT_EXEC MY__PROT_EXEC
#endif

#define MY_PAGE_READONLY PROT_READ
#define MY_PAGE_READWRITE (PROT_READ | PROT_WRITE)
#define MY_PAGE_EXECUTE PROT_EXEC
#define MY_PAGE_EXECUTE_READ (PROT_READ | PROT_EXEC)
#define MY_PAGE_EXECUTE_READWRITE (PROT_READ | PROT_WRITE | PROT_EXEC)
#define MY_PAGE_EXECUTE_WRITECOPY (PROT_READ | PROT_WRITE | PROT_EXEC)
#define MY_PAGE_NOACCESS PROT_NONE

//
// Page protection
//

#define PAGESIZE  4096

#define PAGE_NOACCESS 0x01
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE 0x10
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD 0x100
#define PAGE_NOCACHE 0x200
#define PAGE_WRITECOMBINE 0x400

#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define MEM_DECOMMIT 0x4000
#define MEM_RELEASE 0x8000
#define MEM_FREE 0x10000
#define MEM_PRIVATE 0x20000
#define MEM_MAPPED 0x40000
#define MEM_RESET 0x80000
#define MEM_TOP_DOWN 0x100000
#define MEM_LARGE_PAGES 0x20000000
#define MEM_ALIGN64K            0x10000000
#define MEM_4MB_PAGES 0x80000000

BOOL WINAPI VirtualProtect(LPVOID lpAddress,SIZE_T dwSize,DWORD  flNewProtect,PDWORD lpflOldProtect);
SIZE_T WINAPI VirtualQuery(LPCVOID lpAddress,PMEMORY_BASIC_INFORMATION lpBuffer,SIZE_T dwLength);
LPVOID WINAPI VirtualAlloc(LPVOID lpAddress,SIZE_T dwSize,DWORD flAllocationType,DWORD flProtect);
BOOL WINAPI VirtualFree(LPVOID lpAddress,SIZE_T dwSize,DWORD dwFreeType);
BOOL WINAPI FlushInstructionCache(HANDLE  hProcess, LPCVOID lpBaseAddress, SIZE_T  dwSize);
HANDLE WINAPI GetCurrentProcess();
#else
#include <windows.h>
#endif
#define _LONG_PTR(a) ((LONG_PTR)((a)))
#define _ULONG_PTR(a) ((ULONG_PTR)((a)))
typedef struct _vma_it_func
{
	unsigned long long start_address;
	unsigned long long end_address;
	unsigned long long page_size;
	unsigned long long page_alignment_size;
    unsigned long long free_pages;
    unsigned long long free_pages_size;
    unsigned long long reserved_pages_size;
    unsigned long long reserved_pages;
	unsigned long long size;
	unsigned long long base_start_address;
	unsigned long long base_end_address;
	unsigned int base_flags;
	unsigned char ret;
	unsigned char complete_free_region;
} vma_it_func;
int vma_iterate_func(void *data,unsigned long long start, unsigned long long end,unsigned int flags);
int vma_iterate_full_addressing_func(void *data,unsigned long long start, unsigned long long end,unsigned int flags);
#endif
