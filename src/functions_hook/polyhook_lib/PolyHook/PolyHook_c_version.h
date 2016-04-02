#ifndef POLYHOOK_C_VERSION_H
#define POLYHOOK_C_VERSION_H
#define PLH__ASMHelper__D_INT64 8
#define PLH__ASMHelper__D_INT32 4
#define PLH__ASMHelper__D_INT16 2
#define PLH__ASMHelper__D_INT8 1
#define PLH__ASMHelper__D_INVALID -1
#include "../../subhook/windows_defs.h"
#include "../Capstone/include/Capstone.h"

typedef enum _PLH__ASMHelper__HookType
{
	X86Detour,
	X64Detour,
	UNKNOWN
} PLH__ASMHelper__HookType;

typedef struct _PLH_ABSTRACT_DETOUR_S
{
	csh m_CapstoneHandle;
	BYTE * m_hkSrc;
	BYTE * m_hkDst;
	BYTE * m_Trampoline;
	BYTE * m_OriginalCode;
	SIZE_T m_OriginalLength;
	SIZE_T m_hkLength;
	BOOL m_NeedFree;
	PLH__ASMHelper__HookType plhAsmHelperHookType;
} PLH_ABSTRACT_DETOUR_S;


typedef struct _PLH_X86_DETOUR_S
{
	
} PLH_X86_DETOUR_S;

typedef struct _PLH_X64_DETOUR_S
{
	
} PLH_X64_DETOUR_S;

typedef struct _PLH_MEMORY_PROTECT_S
{
	void* m_Address;
	size_t m_Size;
	DWORD m_Flags;	
	DWORD m_OldProtection;
} PLH_MEMORY_PROTECT_S;

typedef struct _PLH_ALL_S 
{
	PLH_ABSTRACT_DETOUR_S plhAbstractDetourS;
	PLH_MEMORY_PROTECT_S plhMemoryProtectS;
	PLH_X86_DETOUR_S plhX86DetourS;
	PLH_X64_DETOUR_S plhX64DetourS;
	int last_error;
} PLH_ALL_S, *PLH_ALL_S_t;

PLH_ALL_S_t PLH_ALL_S_t_new();

BOOL PLH_ALL_S_t_delete(PLH_ALL_S_t p);

int8_t PLH__ASMHelper__GetDisplacementType(PLH_ALL_S_t a, const uint8_t DispVal);

BOOL PLH__ASMHelper__IsConditionalJump(PLH_ALL_S_t a, const BYTE* bytes,const uint16_t Size);

int8_t PLH__ASMHelper__GetDisplacement__int8_t(PLH_ALL_S_t a, BYTE* Instruction, const uint32_t Offset);

int16_t PLH__ASMHelper__GetDisplacement__int16_t(PLH_ALL_S_t a, BYTE* Instruction, const uint32_t Offset);

int32_t PLH__ASMHelper__GetDisplacement__int32_t(PLH_ALL_S_t a, BYTE* Instruction, const uint32_t Offset);

BOOL PLH__MyDetour__GetNeedFree(PLH_ALL_S_t a);

void PLH__MyDetour__SetNeedFree(PLH_ALL_S_t a,BOOL needFree);

void PLH__MyDetour__SetHookType(PLH_ALL_S_t a,PLH__ASMHelper__HookType hookType);

PLH__ASMHelper__HookType PLH__MyDetour__GetHookType(PLH_ALL_S_t a);

BYTE * PLH__MyDetour__GetTrampoline(PLH_ALL_S_t a);

void PLH__MyDetour__SetTrampoline(PLH_ALL_S_t a,BYTE*trampoline);

BYTE * PLH__MyDetour__GetOriginalCode(PLH_ALL_S_t a);

void PLH__MyDetour__SetOriginalCode(PLH_ALL_S_t a,BYTE*originalcode);

BYTE * PLH__MyDetour__GetHkSrc(PLH_ALL_S_t a);

void PLH__MyDetour__SetHkSrc(PLH_ALL_S_t a,BYTE*hkSrc);

BYTE * PLH__MyDetour__GetHkDst(PLH_ALL_S_t a);

void PLH__MyDetour__SetHkDst(PLH_ALL_S_t a,BYTE*hkDst);

SIZE_T PLH__MyDetour__GetHkLength(PLH_ALL_S_t a);

void PLH__MyDetour__SetHkLength(PLH_ALL_S_t a,SIZE_T hkLength);

SIZE_T PLH__MyDetour__GetOriginalLength(PLH_ALL_S_t a);

void PLH__MyDetour__SetOriginalLength(PLH_ALL_S_t a,SIZE_T originalLength);

csh PLH__MyDetour__GetCapstoneHandleValue(PLH_ALL_S_t a);

void PLH__MyDetour__SetCapstoneHandleValue(PLH_ALL_S_t a,csh capstoneHandle);

HANDLE PLH__MyDetour__GetCapstoneHandle(PLH_ALL_S_t a);

void PLH__MyDetour__SetCapstoneHandle(PLH_ALL_S_t a,HANDLE capstoneHandle);

x86_reg PLH__AbstractDetour__GetIpReg(PLH_ALL_S_t a);

void PLH__AbstractDetour__RelocateASM(PLH_ALL_S_t a,BYTE* Code, DWORD * CodeSize, DWORD64 From, DWORD64 To);

void PLH__AbstractDetour__Relocate(PLH_ALL_S_t a,cs_insn* CurIns, DWORD64 From, DWORD64 To, const uint8_t DispSize, const uint8_t DispOffset);

void PLH__MemoryProtect__SetAddress(PLH_ALL_S_t a,void * Address);

void * PLH__MemoryProtect__GetAddress(PLH_ALL_S_t a);

void PLH__MemoryProtect__SetSize(PLH_ALL_S_t a,size_t size);

size_t PLH__MemoryProtect__GetSize(PLH_ALL_S_t a);

void PLH__MemoryProtect__SetFlags(PLH_ALL_S_t a,DWORD flags);

DWORD PLH__MemoryProtect__GetFlags(PLH_ALL_S_t a);

void PLH__MemoryProtect__SetOldProtection(PLH_ALL_S_t a,DWORD oldProtection);

DWORD PLH__MemoryProtect__GetOldProtection(PLH_ALL_S_t a);

void PLH__MemoryProtect__init(PLH_ALL_S_t a,void* Address, size_t Size, DWORD ProtectionFlags);

BOOL PLH__MemoryProtect__Protect(PLH_ALL_S_t a,void* Address, size_t Size, DWORD ProtectionFlags);

PLH__MemoryProtect__deinit(PLH_ALL_S_t a);

void PLH__AbstractDetour__Initialize(PLH_ALL_S_t a,cs_mode Mode);

DWORD PLH__AbstractDetour__CalculateLength(PLH_ALL_S_t a,BYTE* Src, DWORD NeededLength);

long PLH__AbstractDetour__CalculateRelativeDisplacement__long(PLH_ALL_S_t a,DWORD64 From, DWORD64 To, DWORD InsSize);

int8_t PLH__AbstractDetour__CalculateRelativeDisplacement__int8_t(PLH_ALL_S_t a,DWORD64 From, DWORD64 To, DWORD InsSize);

int16_t PLH__AbstractDetour__CalculateRelativeDisplacement__int16_t(PLH_ALL_S_t a,DWORD64 From, DWORD64 To, DWORD InsSize);

int32_t PLH__AbstractDetour__CalculateRelativeDisplacement__int32_t(PLH_ALL_S_t a,DWORD64 From, DWORD64 To, DWORD InsSize);

void PLH__AbstractDetour__WriteJMP(PLH_ALL_S_t a,DWORD_PTR From, DWORD_PTR To);

int PLH__AbstractDetour__GetJMPSize(PLH_ALL_S_t a);
		
void PLH__AbstractDetour__RelocateConditionalJMP(PLH_ALL_S_t a,cs_insn* CurIns, DWORD * CodeSize, DWORD64 From, DWORD64 To, const uint8_t DispSize, const uint8_t DispOffset);

void PLH__AbstractDetour__SetupHook(PLH_ALL_S_t a,BYTE* Src, BYTE* Dest);

void PLH__AbstractDetour__FlushSrcInsCache(PLH_ALL_S_t a);

void PLH__AbstractDetour__FreeTrampoline(PLH_ALL_S_t a);

BOOL PLH__AbstractDetour__Hook(PLH_ALL_S_t a);

void PLH__AbstractDetour__UnHook(PLH_ALL_S_t a);

PLH__AbstractDetour__init(PLH_ALL_S_t a);

PLH__AbstractDetour__deinit(PLH_ALL_S_t a);

void PLH__MyDetour__init(PLH_ALL_S_t a);

void PLH__MyDetour__deinit(PLH_ALL_S_t a);

void PLH__X86Detour__WriteRelativeJMP(PLH_ALL_S_t a,DWORD Destination, DWORD JMPDestination);

void PLH__X86Detour__WriteJMP(PLH_ALL_S_t a,DWORD_PTR From, DWORD_PTR To);

int PLH__X86Detour__GetJMPSize();

void PLH__X86Detour__init(PLH_ALL_S_t a);

void PLH__X86Detour__deinit(PLH_ALL_S_t a);

PLH__ASMHelper__HookType PLH__X86Detour__GetType(PLH_ALL_S_t a);

BOOL PLH__X86Detour__Hook(PLH_ALL_S_t a);

x86_reg PLH__X86Detour__GetIpReg(PLH_ALL_S_t a);

void PLH__X86Detour__FreeTrampoline(PLH_ALL_S_t a);

void PLH__X86Detour__WriteRelativeJMP(PLH_ALL_S_t a,DWORD Destination, DWORD JMPDestination);

void PLH__X86Detour__WriteJMP(PLH_ALL_S_t a,DWORD_PTR From, DWORD_PTR To);

int PLH__X86Detour__GetJMPSize(PLH_ALL_S_t a);

void PLH__X64Detour__init(PLH_ALL_S_t a);

void PLH__X64Detour__deinit(PLH_ALL_S_t a);

PLH__ASMHelper__HookType PLH__X64Detour__GetType();

BOOL PLH__X64Detour__Hook(PLH_ALL_S_t a);

x86_reg PLH__X64Detour__GetIpReg(PLH_ALL_S_t a);

void PLH__X64Detour__FreeTrampoline(PLH_ALL_S_t a);

void PLH__X64Detour__WriteAbsoluteJMP(PLH_ALL_S_t a,DWORD64 Destination, DWORD64 JMPDestination);

void PLH__X64Detour__WriteJMP(PLH_ALL_S_t a,DWORD_PTR From, DWORD_PTR To);

int PLH__X64Detour__GetJMPSize(PLH_ALL_S_t a);
#endif