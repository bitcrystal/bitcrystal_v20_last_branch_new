#ifndef LINUX_DEFS_H
#define LINUX_DEFS_H
#include "my_predef.h"
#ifdef OS_UNIX_STRUCT
#if defined(MY_CPU_ARM) || defined(MY_CPU_ARM64)
#include <asm/cachectl.h>
#elif defined MY_CPU_MIPS
#include <asm/cachectl.h>
#else
#define LINUX_CACHEFLUSH_DEF
#define ICACHE
#define DCACHE
#define BCACHE (ICACHE|DCACHE)
#ifndef OS_MAC
#include <stdio.h>
#else
#include <stdlib.h>
#endif
int cacheflush(char *addr, int nbytes, int cache);
#endif
#endif
#endif