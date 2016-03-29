//Copyright by bitcrystal
#ifndef MY_PREDEF_H
#define MY_PREDEF_H
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__) || defined(__MINGW64__)
#define OS_WIN
#elif defined(__linux__)
#define OS_LINUX
#elif defined(__APPLE__)
#define OS_APPLE
#elif defined(__unix__)
#define OS_UNIX
#elif defined(__MACH__)
#define OS_MACH
#elif defined(__FreeBSD__)
#define OS_FREE_BSD
#elif defined(__NetBSD__)
#define OS_NET_BSD
#elif defined(HAVE_MQUERY)
#define OS_OPEN_BSD
#elif defined(__BEOS__)
#define OS_BEOS
#elif defined(__HAIKU__)
#define OS_HAIKU
#else
#define OS_UNKNOWN
#endif
#if defined(OS_APPLE) || defined(OS_UNIX) || defined(OS_MACH)
#define OS_AUFGEGESSENER_APFEL
#endif
#ifdef OS_AUFGEGESSENER_APFEL
#define OS_EATEN_APPLE
#define OS_MAC
#endif
#if defined(OS_FREE_BSD)||defined(OS_NET_BSD)||defined(OS_OPEN_BSD)
#define OS_BSD_STRUCT
#endif
#if defined(OS_MAC)||defined(OS_BSD_STRUCT)||defined(OS_LINUX)
#define OS_UNIX_STRUCT
#endif
#if defined(_M_X64) || defined(_M_AMD64) || defined(__amd64__) || defined(_LP64) || defined(_ILP64) || defined(LP64) || defined(ILP64) || defined(__MINGW64__) || defined(_WIN64) || defined(WIN64)
#define IS_X64
#elif defined(_M_IX86) || defined(__i386__) || defined(__MINGW32__) || defined(_WIN32) || defined(WIN32)
#define IS_X86
#else
#define IS_X86
#endif
#endif
#if defined(OS_WIN)
	#define _WIN32
	#define WIN32
	#define WIN
	#define _WIN
	#define BITCRYSTAL_OS_VERSION 1
#elif defined(OS_LINUX)
	#define __linux__
	#define BITCRYSTAL_OS_VERSION 2
#elif defined(OS_MAC)
	#if defined(OS_UNIX)
		#define __unix__
	#elif defined(OS_APPLE)
		#define __APPLE__
	#elif defined(OS_MACH)
		#define __MACH__
	#else
		#define OS_UNKNOWN
	#endif
	#define BITCRYSTAL_OS_VERSION 3
#endif
#if defined(IS_X86)
	#define _M_IX86
	#define __i386__
//	#defined __MINGW32__
//	#defined _WIN32
//	#define WIN32
	#define BITCRYSTAL_OS_ARCH 1
#elif defined(IS_X64)
	#define _M_X64
	#define _M_AMD64
	#define __amd64__
	#define _LP64
	#define _ILP64
	#define LP64
	#define ILP64
//	#define __MINGW64__
	#define _WIN64
	#define WIN64
	#define BITCRYSTAL_OS_ARCH 2
#endif