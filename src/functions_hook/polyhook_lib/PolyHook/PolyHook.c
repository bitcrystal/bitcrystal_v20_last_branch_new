#include "PolyHook.h"

/*----------------------------------------------*/

/*----------------------------------------------*/
PLH::HookType PLH::VFuncSwap::GetType()
{
	return PLH::HookType::VFuncSwap;
}

bool PLH::VFuncSwap::Hook()
{
	MemoryProtect Protector(&m_hkVtable[m_hkIndex], sizeof(void*), PAGE_READWRITE);
	m_OrigVFunc = m_hkVtable[m_hkIndex];
	m_hkVtable[m_hkIndex] = m_hkDest;
	return true;
}

void PLH::VFuncSwap::UnHook()
{
	MemoryProtect Protector(&m_hkVtable[m_hkIndex], sizeof(void*), PAGE_READWRITE);
	m_hkVtable[m_hkIndex] = m_OrigVFunc;
}

void PLH::VFuncSwap::SetupHook(BYTE** Vtable, const int Index, BYTE* Dest)
{
	m_hkVtable = Vtable;
	m_hkDest = Dest;
	m_hkIndex = Index;
}

/*----------------------------------------------*/
PLH::VFuncDetour::VFuncDetour() :IHook()
{
	m_Detour =std::make_unique<Detour>();
}

PLH::VFuncDetour::~VFuncDetour()
{
	
}

PLH::HookType PLH::VFuncDetour::GetType()
{
	return PLH::HookType::VFuncDetour;
}

bool PLH::VFuncDetour::Hook()
{
	return m_Detour->Hook();
}

void PLH::VFuncDetour::UnHook()
{
	m_Detour->UnHook();
}

void PLH::VFuncDetour::SetupHook(BYTE** Vtable, const int Index, BYTE* Dest)
{
	m_Detour->SetupHook(Vtable[Index], Dest);
}

PLH::RuntimeError PLH::VFuncDetour::GetLastError() const
{
	return m_Detour->GetLastError();
}

void PLH::VFuncDetour::PostError(const RuntimeError& Err)
{
	m_Detour->PostError(Err);
}
/*----------------------------------------------*/
PLH::VTableSwap::VTableSwap() :IHook()
{
	m_NeedFree = false;
}

PLH::VTableSwap::~VTableSwap()
{
	FreeNewVtable();
}

PLH::HookType PLH::VTableSwap::GetType()
{
	return PLH::HookType::VTableSwap;
}

bool PLH::VTableSwap::Hook()
{
	MemoryProtect Protector(m_phkClass, sizeof(void*), PAGE_READWRITE);
	m_OrigVtable = *m_phkClass;
	m_hkOriginal = m_OrigVtable[m_hkIndex];
	m_VFuncCount = GetVFuncCount(m_OrigVtable);
	m_NewVtable = (BYTE**) new DWORD_PTR[m_VFuncCount];
	m_NeedFree = true;
	memcpy(m_NewVtable, m_OrigVtable, sizeof(void*)*m_VFuncCount);
	*m_phkClass = m_NewVtable;
	m_NewVtable[m_hkIndex] = m_hkDest;
	return true;
}

void PLH::VTableSwap::UnHook()
{
	MemoryProtect Protector(m_phkClass, sizeof(void*), PAGE_READWRITE);
	*m_phkClass = m_OrigVtable;
	FreeNewVtable();
}

void PLH::VTableSwap::SetupHook(BYTE* pClass, const int Index, BYTE* Dest)
{
	m_phkClass = (BYTE***)pClass; //ppp is just convenient to work with
	m_hkDest = Dest;
	m_hkIndex = Index;
}

int PLH::VTableSwap::GetVFuncCount(BYTE** pVtable)
{
	int FuncCount = 0;
	for (; ; FuncCount++)
	{
		if (!IsValidPtr(pVtable[FuncCount]))
			break;
	}
	return FuncCount;
}

void PLH::VTableSwap::FreeNewVtable()
{
	if (m_NeedFree)
	{
		delete[] m_NewVtable;
		m_NeedFree = false;
	}
}

/*----------------------------------------------*/
PLH::HookType PLH::IATHook::GetType()
{
	return PLH::HookType::IAT;
}

bool PLH::IATHook::Hook()
{
	PIMAGE_THUNK_DATA Thunk;
	if (!FindIATFunc(m_hkLibraryName.c_str(), m_hkSrcFunc.c_str(), &Thunk, m_hkModuleName.c_str()))
	{
		PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook IATHook: Failed to find import"));
		return false;
	}

	MemoryProtect Protector(Thunk, sizeof(ULONG_PTR), PAGE_EXECUTE_READWRITE);
	m_pIATFuncOrig = (void*)Thunk->u1.Function;
	Thunk->u1.Function = (ULONG_PTR)m_hkDest;
	return true;
}

void PLH::IATHook::UnHook()
{
	PIMAGE_THUNK_DATA Thunk;
	if (!FindIATFunc(m_hkLibraryName.c_str(), m_hkSrcFunc.c_str(), &Thunk))
		return;

	MemoryProtect Protector(Thunk, sizeof(ULONG_PTR), PAGE_EXECUTE_READWRITE);
	Thunk->u1.Function = (ULONG_PTR)m_pIATFuncOrig;
}

void PLH::IATHook::SetupHook(const char* LibraryName,const char* SrcFunc, BYTE* Dest,const char* Module)
{
	m_hkLibraryName = LibraryName;
	m_hkSrcFunc = SrcFunc;
	m_hkModuleName = Module;
	m_hkDest = Dest;
}

bool PLH::IATHook::FindIATFunc(const char* LibraryName,const char* FuncName, PIMAGE_THUNK_DATA* pFuncThunkOut,const char* Module)
{
	bool UseModuleName = true;
	if (Module == NULL || Module[0] == '\0')
		UseModuleName = false;

	HINSTANCE hInst = GetModuleHandleA(UseModuleName ? Module:NULL);
	if (!hInst)
	{
		PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook IATHook:Failed to find Module"));
		return false;
	}

	ULONG Sz;
	PIMAGE_IMPORT_DESCRIPTOR pImports = (PIMAGE_IMPORT_DESCRIPTOR)
		ImageDirectoryEntryToDataEx(hInst, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &Sz, nullptr);

	for (int i = 0; pImports[i].Characteristics != 0; i++)
	{
		char* _ModuleName = (char*)ResolveRVA(hInst, pImports[i].Name);
		if (_stricmp(_ModuleName, LibraryName) != 0)
			continue;

		//Original holds the API Names
		PIMAGE_THUNK_DATA pOriginalThunk = (PIMAGE_THUNK_DATA)
			ResolveRVA(hInst, pImports->OriginalFirstThunk);

		//FirstThunk is overwritten by loader with API addresses, we change this
		PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)
			ResolveRVA(hInst, pImports->FirstThunk);
		
		if (!pOriginalThunk)
		{
			PostError(RuntimeError(RuntimeError::Severity::Critical, "PolyHook IATHook:PE Files without OriginalFirstThunk are unsupported"));
			return false;
		}

		//Table is null terminated, increment both tables
		for (; pOriginalThunk->u1.Function != NULL; pOriginalThunk++,pThunk++)
		{
			if (IMAGE_SNAP_BY_ORDINAL(pOriginalThunk->u1.Ordinal))
			{
				XTrace("Import By Ordinal:[Ordinal:%d]\n",IMAGE_ORDINAL(pOriginalThunk->u1.Ordinal));
				continue;
			}

			PIMAGE_IMPORT_BY_NAME pImport = (PIMAGE_IMPORT_BY_NAME)
				ResolveRVA(hInst, pOriginalThunk->u1.AddressOfData);

			XTrace("Import By Name: [Ordinal:%d] [Name:%s]\n", IMAGE_ORDINAL(pOriginalThunk->u1.Ordinal),pImport->Name);

			//Check the name of API given by OriginalFirthThunk
			if (_stricmp(FuncName, pImport->Name) != 0)
				continue;
			
			/*Name matched in OriginalFirstThunk, return FirstThunk
			so we can changed it's address*/
			*pFuncThunkOut = pThunk;
			return true;
		}
	}
	PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook IATHook:Failed to find import"));
	return false;
}

/*----------------------------------------------*/
PLH::HookType PLH::VEHHook::GetType()
{
	return PLH::HookType::VEH;
}

std::vector<PLH::VEHHook::HookCtx> PLH::VEHHook::m_HookTargets;
std::mutex PLH::VEHHook::m_TargetMutex;
PLH::VEHHook::VEHHook()
{
	//Get size of pages
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_PageSize = si.dwPageSize;

	void* pVEH = AddVectoredExceptionHandler(1, &PLH::VEHHook::VEHHandler);
	if (pVEH == nullptr)
	{
		PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook VEH: Failed to create top level handler"));
	}
}

bool PLH::VEHHook::AreInSamePage(BYTE* Addr1, BYTE* Addr2)
{
	//If VQ fails, be safe and say they are in same page
	MEMORY_BASIC_INFORMATION mbi1;
	if (!VirtualQuery(Addr1, &mbi1, sizeof(mbi1)))
		return true;

	MEMORY_BASIC_INFORMATION mbi2;
	if (!VirtualQuery(Addr2, &mbi2, sizeof(mbi2)))
		return true;

	if (mbi1.BaseAddress == mbi2.BaseAddress)
		return true;

	return false;
}

void PLH::VEHHook::SetupHook(BYTE* Src, BYTE* Dest,VEHMethod Method)
{
	HookCtx Ctx(Src, Dest, Method);
	m_ThisCtx = Ctx;
}

bool PLH::VEHHook::Hook()
{
	//Lock the TargetMutex for thread safe vector operations
	std::lock_guard<std::mutex> m_Lock(m_TargetMutex);

	if (m_ThisCtx.m_Type == VEHMethod::INT3_BP)
	{
		//Write INT3 BreakPoint
		MemoryProtect Protector(m_ThisCtx.m_Src, 1, PAGE_EXECUTE_READWRITE);
		m_ThisCtx.m_StorageByte = *m_ThisCtx.m_Src;
		*m_ThisCtx.m_Src = 0xCC;
		m_HookTargets.push_back(m_ThisCtx);
	}else if (m_ThisCtx.m_Type == VEHMethod::HARDWARE_BP) {
		CONTEXT Ctx;
		Ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	
		if (!GetThreadContext(GetCurrentThread(), &Ctx))
		{
			PostError(PLH::RuntimeError(RuntimeError::Severity::Critical, "Failed to get context"));
			return false;
		}

		uint8_t RegIndex = 0;
		bool FoundReg = false;
		for (; RegIndex < 4; RegIndex++)
		{
			if ((Ctx.Dr7 & (1 << (RegIndex * 2))) == 0)
			{
				FoundReg = true;
				break;
			}
		}
		if (!FoundReg)
		{
			PostError(PLH::RuntimeError(RuntimeError::Severity::Critical, "Failed to find free Reg"));
			return false;
		}

		switch (RegIndex)
		{
		case 0: 
			Ctx.Dr0 = (DWORD_PTR)m_ThisCtx.m_Src;
			break;
		case 1: 
			Ctx.Dr1 = (DWORD_PTR)m_ThisCtx.m_Src; 
			break;
		case 2: 
			Ctx.Dr2 = (DWORD_PTR)m_ThisCtx.m_Src; 
			break;
		case 3: 
			Ctx.Dr3 = (DWORD_PTR)m_ThisCtx.m_Src; 
			break;
		default: 
			PostError(PLH::RuntimeError(RuntimeError::Severity::Critical, "PolyHook VEH: Invalid Debug Register Index"));
			return false;
		}
		//Turn a local register on
		Ctx.Dr7 |= 1 << (2*RegIndex);
		m_ThisCtx.m_StorageByte = RegIndex;
		//Still need to call suspend thread *TODO*
		if (!SetThreadContext(GetCurrentThread(), &Ctx))
		{
			PostError(PLH::RuntimeError(RuntimeError::Severity::Critical, "PolyHook VEH: Failed to set thread context"));
			return false;
		}
		m_HookTargets.push_back(m_ThisCtx);
	}else if (m_ThisCtx.m_Type == VEHMethod::GUARD_PAGE){
		//Read current page protection
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(m_ThisCtx.m_Src, &mbi, sizeof(mbi));

		//can't use Page Guards with NO_ACCESS flag
		if (mbi.Protect & PAGE_NOACCESS)
		{
			PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook VEH: Cannot hook page with NOACCESS Flag"));
			return false;
		}

		if (AreInSamePage((BYTE*)&PLH::VEHHook::VEHHandler, m_ThisCtx.m_Src))
		{
			PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook VEH: Cannot hook page on same page as the VEH"));
			return false;
		}

		//!!!!COMPILER SPECIFIC HACK HERE!!!!!
		bool(PLH::VEHHook::* pHookFunc)(void) = &PLH::VEHHook::Hook;
		if (AreInSamePage((BYTE*&)pHookFunc, m_ThisCtx.m_Src))
		{
			PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook VEH: Cannot hook page on same page as the hooking function"));
			return false;
		}
		
		m_HookTargets.push_back(m_ThisCtx);

		//Write Page Guard protection
		DWORD OldProtection;
		VirtualProtect(m_ThisCtx.m_Src, 1 ,PAGE_EXECUTE_READWRITE | PAGE_GUARD, &OldProtection);
	}
	return true;
}

void PLH::VEHHook::UnHook()
{
	std::lock_guard<std::mutex> m_Lock(m_TargetMutex);
	
	if (m_ThisCtx.m_Type == VEHMethod::INT3_BP)
	{
		MemoryProtect Protector(m_ThisCtx.m_Src, 1, PAGE_EXECUTE_READWRITE);
		*m_ThisCtx.m_Src = m_ThisCtx.m_StorageByte;
	}else if (m_ThisCtx.m_Type == VEHMethod::HARDWARE_BP) {
		CONTEXT Ctx;
		Ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		if (!GetThreadContext(GetCurrentThread(), &Ctx))
		{
			PostError(PLH::RuntimeError(RuntimeError::Severity::Critical, "Failed to get context"));
			return;
		}
		Ctx.Dr7 &= ~(1 << (2 * m_ThisCtx.m_StorageByte));
		//Still need to call suspend thread
		if (!SetThreadContext(GetCurrentThread(), &Ctx))
		{
			PostError(PLH::RuntimeError(RuntimeError::Severity::Critical, "Failed to set context"));
			return;
		}
	}else if (m_ThisCtx.m_Type == VEHMethod::GUARD_PAGE) {
		volatile BYTE GenerateExceptionRead = *m_ThisCtx.m_Src;
	}
	m_HookTargets.erase(std::remove(m_HookTargets.begin(), m_HookTargets.end(), m_ThisCtx), m_HookTargets.end());
}

LONG CALLBACK PLH::VEHHook::VEHHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
#ifdef _WIN64
	#define XIP Rip
#else
	#define XIP Eip
#endif // _WIN64
	std::lock_guard<std::mutex> m_Lock(m_TargetMutex);

	DWORD ExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
	if (ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		for (HookCtx& Ctx : m_HookTargets)
		{
			if(Ctx.m_Type != VEHMethod::INT3_BP)
				continue;

			//Are we at a breakpoint that we placed?
			if (ExceptionInfo->ContextRecord->XIP != (DWORD_PTR)Ctx.m_Src)
				continue;

			//Remove Int3 Breakpoint
			MemoryProtect Protector(Ctx.m_Src, 1, PAGE_EXECUTE_READWRITE);
			*Ctx.m_Src = Ctx.m_StorageByte;
			
			//Set instruction pointer to our callback
			ExceptionInfo->ContextRecord->XIP = (DWORD_PTR) Ctx.m_Dest;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}else if (ExceptionCode == EXCEPTION_SINGLE_STEP) {
		//Intel says clear Dr6, windows may do it for us, lets be safe
		ExceptionInfo->ContextRecord->Dr6 = 0; 
		for (HookCtx& Ctx : m_HookTargets)
		{
			
			if (Ctx.m_Type != VEHMethod::HARDWARE_BP)
				continue;
		
			//Are we at a breakpoint that we placed?
			if (ExceptionInfo->ContextRecord->XIP != (DWORD_PTR)Ctx.m_Src)
				continue;
		
			//Clear the Debug Register
			ExceptionInfo->ContextRecord->Dr7 &= ~(1 << (2 * Ctx.m_StorageByte));

			//Set instruction pointer to callback
			ExceptionInfo->ContextRecord->XIP = (DWORD_PTR)Ctx.m_Dest;
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}else if (ExceptionCode == EXCEPTION_GUARD_PAGE) {
		for (HookCtx& Ctx : m_HookTargets)
		{
			//still need to check if exception is in our page
			if (Ctx.m_Type != VEHMethod::GUARD_PAGE)
				continue;

			if(!AreInSamePage((BYTE*)ExceptionInfo->ContextRecord->XIP,Ctx.m_Src))
				continue;

			if (ExceptionInfo->ContextRecord->XIP == (DWORD_PTR)Ctx.m_Src)
				ExceptionInfo->ContextRecord->XIP = (DWORD_PTR)Ctx.m_Dest;

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

/*----------------------------------------------*/
PLH::MemoryProtect::MemoryProtect(void* Address, size_t Size, DWORD ProtectionFlags)
{
	m_Address = Address;
	m_Size = Size;
	m_Flags = ProtectionFlags;
	Protect(m_Address, m_Size, m_Flags);
}

bool PLH::MemoryProtect::Protect(void* Address, size_t Size, DWORD ProtectionFlags)
{
	return VirtualProtect(Address, Size, ProtectionFlags, &m_OldProtection);
}

PLH::MemoryProtect::~MemoryProtect()
{
	Protect(m_Address,m_Size, m_OldProtection);
}