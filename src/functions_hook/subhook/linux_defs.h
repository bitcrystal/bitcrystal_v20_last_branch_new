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
#ifndef   ICACHE
#define   ICACHE   (1<<0)      /* flush instruction cache        */
#endif
#ifndef   DCACHE
#define   DCACHE   (1<<1)      /* writeback and flush data cache */
#endif
#ifndef   BCACHE
#define   BCACHE   (ICACHE|DCACHE)   /* flush both caches              */
#endif
#ifndef OS_MAC
#include <stdio.h>
#else
#include <stdlib.h>
#endif
int cacheflush(char *addr, int nbytes, int cache);
#endif
#endif
#endif
