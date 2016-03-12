//Copyright by bitcrystal
#ifndef HOOKERPREF_H
#define HOOKERPREF_H
#include "my_predef.h"
#include "subhook_predef.h"
#include "subhook.h"
typedef void * PVOID;
typedef int                 BOOL;
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
#endif