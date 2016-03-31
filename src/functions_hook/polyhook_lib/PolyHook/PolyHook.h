#ifndef POLYHOOK_H
#define POLYHOOK_H
#include <windows.h>
#include "../Capstone/include/capstone.h"
#include <DbgHelp.h>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <utility>
#pragma comment(lib,"Dbghelp.lib")
#pragma comment(lib,"capstone.lib")

#define PLH__ASMHelper__DISP_D_INT64 8
#define PLH__ASMHelper__DISP_D_INT32 4
#define PLH__ASMHelper__DISP_D_INT16 2
#define PLH__ASMHelper__DISP_D_INT8 1
#define PLH__ASMHelper__DISP_D_INVALID -1

uint8_t PLH__ASMHelper_GetDisplacementType(const uint8_t DispVal)
{
		switch (DispVal)
		{
			case PLH__ASMHelper__DISP_D_INT8:
				return PLH__ASMHelper__DISP_D_INT8;
			case PLH__ASMHelper__DISP_D_INT16:
				return PLH__ASMHelper__DISP_D_INT16;
			case PLH__ASMHelper__DISP_D_INT32:
				return PLH__ASMHelper__DISP_D_INT32;
			case PLH__ASMHelper__DISP_D_INT64:
				return PLH__ASMHelper__DISP_D_INT64;
			default:
				return PLH__ASMHelper__DISP_D_INVALID;
		}
}

bool IsConditionalJump(const BYTE* bytes,const uint16_t Size)
{
	//http://unixwiz.net/techtips/x86-jumps.html
	if (Size < 1)
		return false;

	if (bytes[0] == 0x0F && Size > 1)
	{
		if (bytes[1] >= 0x80 && bytes[1] <= 0x8F)
			return true;
	}

	if (bytes[0] >= 0x70 && bytes[0] <= 0x7F)
		return true;

	if (bytes[0] == 0xE3)
		return true;

	return false;
}

template<typename T>
T GetDisplacement(BYTE* Instruction, const uint32_t Offset)
{
	T Disp;
	memset(&Disp, 0x00, sizeof(T));
	memcpy(&Disp, &Instruction[Offset], sizeof(T));
	return Disp;
}

	class PLH__RuntimeError
	{
	public:
		enum class Severity
		{
			Warning, //Might have an issue
			Critical, //Definitely have an issue, but it's not serious
			UnRecoverable, //Definitely have an issue, it's serious
			NoError //Default
		};
		RuntimeError();
		RuntimeError(Severity Sev, const std::string& Msg);
		virtual ~RuntimeError() = default;
		const Severity GetSeverity() const;
		const std::string GetString() const;
	private:
		Severity m_Severity;
		std::string m_Message;
	};

	enum class PLH__HookType
	{
		X86Detour,
		X64Detour,
		VFuncSwap,
		VFuncDetour,
		VTableSwap,
		IAT,
		VEH,
		UNKNOWN
	};
	class PLH__IHook
	{
	public:
		IHook() = default;
		IHook(IHook&& other) = default; //move
		IHook& operator=(IHook&& other) = default;//move assignment
		IHook(const IHook& other) = delete; //copy
		IHook& operator=(const IHook& other) = delete; //copy assignment
		virtual ~IHook() = default;

		virtual bool Hook() = 0;
		virtual void UnHook() = 0;
		virtual HookType GetType() = 0;

		virtual RuntimeError GetLastError() const;
		virtual void PrintError(const RuntimeError& Err) const;
		
	protected:
		virtual void PostError(const RuntimeError& Err);
		RuntimeError m_LastError;
	};

	class PLH_AbstractDetour :public IHook
	{
	public:
		AbstractDetour();
		AbstractDetour(const AbstractDetour& other) = delete;
		AbstractDetour& operator=(const AbstractDetour& other) = delete;
		virtual ~AbstractDetour();

		template<typename T>
		void SetupHook(T* Src, T* Dest)
		{
			SetupHook((BYTE*)Src, (BYTE*)Dest);
		}
		void SetupHook(BYTE* Src, BYTE* Dest);

		virtual void UnHook() override;

		template<typename T>
		T GetOriginal()
		{
			return (T)m_Trampoline;
		}
	protected:
		template<typename T>
		T CalculateRelativeDisplacement(DWORD64 From, DWORD64 To, DWORD InsSize)
		{
			if (To < From)
				return 0 - (From - To) - InsSize;
			return To - (From + InsSize);
		}
		DWORD CalculateLength(BYTE* Src, DWORD NeededLength);
		void RelocateASM(BYTE* Code, DWORD& CodeSize, DWORD64 From, DWORD64 To);
		void _Relocate(cs_insn* CurIns, DWORD64 From, DWORD64 To, const uint8_t DispSize, const uint8_t DispOffset);
		void RelocateConditionalJMP(cs_insn* CurIns, DWORD& CodeSize, DWORD64 From, DWORD64 To, const uint8_t DispSize, const uint8_t DispOffset);
		virtual x86_reg GetIpReg() = 0;
		virtual void FreeTrampoline() = 0;
		virtual void WriteJMP(DWORD_PTR From, DWORD_PTR To) = 0;
		virtual int GetJMPSize() = 0;
		void FlushSrcInsCache();
		void Initialize(cs_mode Mode);
		csh m_CapstoneHandle;
		ASMHelper m_ASMInfo;

		BYTE m_OriginalCode[64];
		DWORD m_OriginalLength;
		BYTE* m_Trampoline;
		bool m_NeedFree;
		BYTE* m_hkSrc;
		BYTE* m_hkDest;
		DWORD m_hkLength;
		cs_mode m_CapMode;
	};

#ifndef _WIN64
#define Detour X86Detour
	//x86 5 Byte Detour
	class PLH__X86Detour :public AbstractDetour
	{
	public:
		friend class VFuncDetour;
		X86Detour();
		X86Detour(X86Detour&& other) = default; //move
		X86Detour& operator=(X86Detour&& other) = default;//move assignment
		X86Detour(const X86Detour& other) = delete; //copy
		X86Detour& operator=(const X86Detour& other) = delete; //copy assignment
		virtual ~X86Detour();

		virtual bool Hook() override;
		virtual HookType GetType() override;
	protected:
		virtual x86_reg GetIpReg() override;
		virtual void FreeTrampoline();
		virtual void WriteJMP(DWORD_PTR From, DWORD_PTR To);
		virtual int GetJMPSize();
	private:
		void WriteRelativeJMP(DWORD Destination, DWORD JMPDestination);
	};
#else
#define Detour X64Detour
	//X64 6 Byte Detour
	class PLH_X64Detour :public AbstractDetour
	{
	public:
		friend class VFuncDetour;
		//Credits DarthTon, evolution536
		X64Detour();
		X64Detour(X64Detour&& other) = default; //move
		X64Detour& operator=(X64Detour&& other) = default;//move assignment
		X64Detour(const X64Detour& other) = delete; //copy
		X64Detour& operator=(const X64Detour& other) = delete; //copy assignment
		virtual ~X64Detour();

		virtual bool Hook() override;
		virtual HookType GetType() override;
	protected:
		virtual x86_reg GetIpReg() override;
		virtual void FreeTrampoline() override;
		virtual void WriteJMP(DWORD_PTR From, DWORD_PTR To) override;
		virtual int GetJMPSize() override;
	private:
		void WriteAbsoluteJMP(DWORD64 Destination, DWORD64 JMPDestination);
	};
#endif //END _WIN64 IFDEF

	//Swap Virtual Function Pointer to Destination
	class VFuncSwap : public IHook
	{
	public:
		VFuncSwap() = default;
		VFuncSwap(VFuncSwap&& other) = default;
		VFuncSwap& operator=(VFuncSwap&& other) = default;
		VFuncSwap(const VFuncSwap& other) = delete;
		VFuncSwap& operator=(const VFuncSwap& other) = delete;
		virtual ~VFuncSwap() = default;

		virtual bool Hook() override;
		virtual void UnHook() override;
		virtual HookType GetType() override;

		void SetupHook(BYTE** Vtable, const int Index, BYTE* Dest);
		template<typename T>
		T GetOriginal()
		{
			return (T)m_OrigVFunc;
		}
	private:
		BYTE** m_hkVtable;
		BYTE* m_hkDest;
		BYTE* m_OrigVFunc;
		int m_hkIndex;
	};

	//Detour the Function the VTable Points to
	class VFuncDetour :public IHook
	{
	public:
		VFuncDetour();
		VFuncDetour(VFuncDetour&& other) = default; //move
		VFuncDetour& operator=(VFuncDetour&& other) = default;//move assignment
		VFuncDetour(const VFuncDetour& other) = delete; //copy
		VFuncDetour& operator=(const VFuncDetour& other) = delete; //copy assignment
		virtual ~VFuncDetour();

		virtual bool Hook() override;
		virtual void UnHook() override;
		virtual HookType GetType() override;

		void SetupHook(BYTE** Vtable, const int Index, BYTE* Dest);
		template<typename T>
		T GetOriginal()
		{
			return m_Detour->GetOriginal<T>();
		}
		virtual RuntimeError GetLastError() const override;
	protected:
		virtual void PostError(const RuntimeError& Err) override;
	private:
		std::unique_ptr<Detour> m_Detour;
	};

	//Credit to Dogmatt for IsValidPtr
#ifdef _WIN64
#define _PTR_MAX_VALUE ((PVOID)0x000F000000000000)
#else
#define _PTR_MAX_VALUE ((PVOID)0xFFF00000)
#endif
	__forceinline bool IsValidPtr(PVOID p) { return (p >= (PVOID)0x10000) && (p < _PTR_MAX_VALUE) && p != nullptr; }

	class VTableSwap : public IHook
	{
	public:
		VTableSwap();
		VTableSwap(VTableSwap&& other) = default; //move
		VTableSwap& operator=(VTableSwap&& other) = default;//move assignment
		VTableSwap(const VTableSwap& other) = delete; //copy
		VTableSwap& operator=(const VTableSwap& other) = delete; //copy assignment
		virtual ~VTableSwap();

		virtual bool Hook() override;
		virtual HookType GetType() override;

		template<typename T>
		T HookAdditional(const int Index, BYTE* Dest)
		{
			//The makes sure we called Hook first
			if (!m_NeedFree)
				return nullptr;

			m_NewVtable[Index] = Dest;
			return (T)m_OrigVtable[Index];
		}
		virtual void UnHook() override;
		void SetupHook(BYTE* pClass, const int Index, BYTE* Dest);
		template<typename T>
		T GetOriginal()
		{
			return (T)m_hkOriginal;
		}
	private:
		int GetVFuncCount(BYTE** pVtable);
		void FreeNewVtable();
		BYTE** m_NewVtable;
		BYTE** m_OrigVtable;
		BYTE*** m_phkClass;
		BYTE*  m_hkDest;
		BYTE*  m_hkOriginal;
		int    m_hkIndex;
		int    m_VFuncCount;
		bool m_NeedFree;
	};

#define ResolveRVA(base,rva) (( (BYTE*)base) +rva)
	class IATHook:public IHook
	{
	public:
		IATHook() = default;
		IATHook(IATHook&& other) = default; //move
		IATHook& operator=(IATHook&& other) = default;//move assignment
		IATHook(const IATHook& other) = delete; //copy
		IATHook& operator=(const IATHook& other) = delete; //copy assignment
		virtual ~IATHook() = default;

		virtual bool Hook() override;
		virtual void UnHook() override;
		virtual HookType GetType() override;

		template<typename T>
		T GetOriginal()
		{
			return (T)m_pIATFuncOrig;
		}
		void SetupHook(const char* LibraryName,const char* SrcFunc, BYTE* Dest,const char* Module = "");
	private:
		bool FindIATFunc(const char* LibraryName,const char* FuncName,PIMAGE_THUNK_DATA* pFuncThunkOut,const char* Module = "");
		std::string m_hkSrcFunc;
		std::string m_hkLibraryName;
		std::string m_hkModuleName;
		BYTE* m_hkDest;
		void* m_pIATFuncOrig;
	};

	template<typename Func>
	class FinalAction {
	public:
		FinalAction(Func f) :FinalActionFunc(std::move(f)) {}
		~FinalAction()
		{
			FinalActionFunc();
		}
	private:
		Func FinalActionFunc;

		/*Uses RAII to call a final function on destruction
		C++ 11 version of java's finally (kindof)*/
	};

	template <typename F>
	FinalAction<F> finally(F f) {
		return FinalAction<F>(f);
	}

	class MemoryProtect
	{
	public:
		MemoryProtect(void* Address, size_t Size, DWORD ProtectionFlags);
		~MemoryProtect();
	private:
		bool Protect(void* Address, size_t Size, DWORD ProtectionFlags);
		void* m_Address;
		size_t m_Size;
		DWORD m_Flags;
		DWORD m_OldProtection;
	};

	class VEHHook : public IHook
	{
	public:
		enum class VEHMethod
		{
			INT3_BP,
			HARDWARE_BP,
			GUARD_PAGE,
			ERROR_TYPE
		};
		VEHHook();
		VEHHook(VEHHook&& other) = default; //move
		VEHHook& operator=(VEHHook&& other) = default;//move assignment
		VEHHook(const VEHHook& other) = delete; //copy
		VEHHook& operator=(const VEHHook& other) = delete; //copy assignment
		virtual ~VEHHook() = default;

		virtual bool Hook() override;
		virtual void UnHook() override;
		virtual HookType GetType() override;

		template<typename T>
		T GetOriginal()
		{
			return (T)m_ThisCtx.m_Src;
		}
		void SetupHook(BYTE* Src, BYTE* Dest,VEHMethod Method);

		auto GetProtectionObject()
		{
				//Return an object to restore INT3_BP after callback is done
			return finally([&]() {
				if (m_ThisCtx.m_Type == VEHMethod::INT3_BP)
				{
					MemoryProtect Protector(m_ThisCtx.m_Src, 1, PAGE_EXECUTE_READWRITE);
					*m_ThisCtx.m_Src = 0xCC;
				}else if (m_ThisCtx.m_Type == VEHMethod::GUARD_PAGE) {
					DWORD OldProtection;
					VirtualProtect(m_ThisCtx.m_Src, 1, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &OldProtection);
				}
			});
		}
	protected:
		struct HookCtx {
			VEHMethod m_Type;
			BYTE* m_Src;
			BYTE* m_Dest;
			BYTE m_StorageByte; 
			/*Different methods store different things in this byte,
			INT3_BP = hold the byte overwritten
			HARDWARE_BP = the index of the debug register we used
			GUARD_PAGE = unused*/

			HookCtx(BYTE* Src, BYTE* Dest,VEHMethod Method)
			{
				m_Dest = Dest;
				m_Src = Src;
				m_Type = Method;
			}
			HookCtx()
			{
				m_Type = VEHMethod::ERROR_TYPE;
			}
			friend bool operator==(const HookCtx& Ctx1, const HookCtx& Ctx2)
			{
				if (Ctx1.m_Dest == Ctx2.m_Dest && Ctx1.m_Src == Ctx2.m_Src && Ctx1.m_Type == Ctx2.m_Type)
					return true;
				return false;
			}
		};
	private:
		static bool AreInSamePage(BYTE* Addr1, BYTE* Addr2);
		static LONG CALLBACK VEHHandler(EXCEPTION_POINTERS* ExceptionInfo);
		static std::vector<HookCtx> m_HookTargets;
		static std::mutex m_TargetMutex;
		HookCtx m_ThisCtx;
		DWORD m_PageSize;
	};
#include "PolyHook.h"
void XTrace(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
#if defined(_DEBUG) || defined(PLH_SHOW_DEBUG_MESSAGES)
	vfprintf_s(stdout, fmt, args);
#endif
	va_end(args);
}

PLH::RuntimeError::RuntimeError()
{
	m_Message = "";
	m_Severity = Severity::NoError;
}

PLH::RuntimeError::RuntimeError(Severity Sev, const std::string& Msg)
{
	m_Severity = Sev;
	m_Message = Msg;
}

const std::string PLH::RuntimeError::GetString() const
{
	return m_Message;
}

const PLH::RuntimeError::Severity PLH::RuntimeError::GetSeverity() const
{
	return m_Severity;
}

void PLH::IHook::PostError(const RuntimeError& Err)
{
	m_LastError = Err;
	XTrace("Posted Error [SEVERITY:%d]:\n"
		"%s\n",Err.GetSeverity(), Err.GetString().c_str());
}

void PLH::IHook::PrintError(const RuntimeError& Err) const 
{
	std::string Severity = "";
	switch (Err.GetSeverity())
	{
	case PLH::RuntimeError::Severity::Warning:
		Severity = "Warning";
		break;
	case PLH::RuntimeError::Severity::Critical:
		Severity = "Critical";
		break;
	case PLH::RuntimeError::Severity::UnRecoverable:
		Severity = "UnRecoverable";
		break;
	case PLH::RuntimeError::Severity::NoError:
		Severity = "No Error";
		break;
	default:
		Severity = "Unknown";
	}
	XTrace("SEVERITY:[%s] %s\n", Severity.c_str(),
		Err.GetString().c_str()); 
}

PLH::RuntimeError PLH::IHook::GetLastError() const
{
	return m_LastError;
}

PLH::AbstractDetour::AbstractDetour() :IHook(), m_NeedFree(false)
{
#ifdef _WIN64
	Initialize(CS_MODE_64);
#else
	Initialize(CS_MODE_32);
#endif // _WIN64
}

PLH::AbstractDetour::~AbstractDetour()
{
	cs_close(&m_CapstoneHandle);
}

void PLH::AbstractDetour::SetupHook(BYTE* Src, BYTE* Dest)
{
	m_hkSrc = Src;
	m_hkDest = Dest;
}

void PLH::AbstractDetour::UnHook()
{
	MemoryProtect Protector(m_hkSrc, m_hkLength, PAGE_EXECUTE_READWRITE);
	memcpy(m_hkSrc, m_OriginalCode, m_OriginalLength); //Copy original from trampoline back to src
	FlushSrcInsCache();
	FreeTrampoline();
}

DWORD PLH::AbstractDetour::CalculateLength(BYTE* Src, DWORD NeededLength)
{
	//Grab First 100 bytes of function, disasm until invalid instruction
	cs_insn* InstructionInfo;
	size_t InstructionCount = cs_disasm(m_CapstoneHandle, Src, 0x100, (uint64_t)Src, 0, &InstructionInfo);

	//Loop over instructions until we have at least NeededLength's Size
	XTrace("\nORIGINAL:\n");
	DWORD InstructionSize = 0;
	bool BigEnough = false;
	for (int i = 0; i < InstructionCount && !BigEnough; i++)
	{
		cs_insn* CurIns = (cs_insn*)&InstructionInfo[i];
		InstructionSize += CurIns->size;
		if (InstructionSize >= NeededLength)
			BigEnough = true;

		XTrace("%I64X [%d]: ", CurIns->address, CurIns->size);
		for (int j = 0; j < CurIns->size; j++)
			XTrace("%02X ", CurIns->bytes[j]);
		XTrace("%s %s\n", CurIns->mnemonic, CurIns->op_str);
	}
	if (!BigEnough)
		InstructionSize = 0;

	cs_free(InstructionInfo, InstructionCount);
	return InstructionSize;
}

void PLH::AbstractDetour::RelocateASM(BYTE* Code, DWORD& CodeSize, DWORD64 From, DWORD64 To)
{
	cs_insn* InstructionInfo;
	size_t InstructionCount = cs_disasm(m_CapstoneHandle, Code, CodeSize, (uint64_t)Code, 0, &InstructionInfo);

	XTrace("\nTrampoline:\n");
	for (int i = 0; i < InstructionCount; i++)
	{
		cs_insn* CurIns = (cs_insn*)&InstructionInfo[i];
		cs_x86* x86 = &(CurIns->detail->x86);

		XTrace("%I64X: ", CurIns->address);
		for (int j = 0; j < CurIns->size; j++)
			XTrace("%02X ", CurIns->bytes[j]);
		XTrace("%s %s\n", CurIns->mnemonic,CurIns->op_str);

		for (int j = 0; j < x86->op_count; j++)
		{
			cs_x86_op* op = &(x86->operands[j]);
			if (op->type == X86_OP_MEM)
			{
				//MEM are types like lea rcx,[rip+0xdead]
				if (op->mem.base == X86_REG_INVALID)
					continue;

				//Are we relative to instruction pointer?
				if (op->mem.base != GetIpReg())
					continue;

				_Relocate(CurIns, From, To, x86->offsets.displacement_size, x86->offsets.displacement_offset);
			}else if (op->type == X86_OP_IMM) {
				//IMM types are like call 0xdeadbeef
				if (x86->op_count > 1) //exclude types like sub rsp,0x20
					continue;

				
				char* mnemonic = CurIns->mnemonic;
				if (m_ASMInfo.IsConditionalJump(CurIns->bytes,CurIns->size))
				{
					RelocateConditionalJMP(CurIns, CodeSize, From, To, x86->offsets.imm_size, x86->offsets.imm_offset);
					continue;
				}

				//types like push 0x20 slip through, check mnemonic
				if (strcmp(mnemonic, "call") != 0 && strcmp(mnemonic, "jmp") != 0) //probably more types than just these, update list as they're found
					continue;

				_Relocate(CurIns, From, To, x86->offsets.imm_size, x86->offsets.imm_offset);
			}
		}
	}

	XTrace("\nFixed Trampoline\n");
	InstructionCount = cs_disasm(m_CapstoneHandle, Code, CodeSize, (uint64_t)Code, 0, &InstructionInfo);
	for (int i = 0; i < InstructionCount; i++)
	{
		cs_insn* CurIns = (cs_insn*)&InstructionInfo[i];

		XTrace("%I64X: ", CurIns->address);
		for (int j = 0; j < CurIns->size; j++)
			XTrace("%02X ", CurIns->bytes[j]);
		XTrace("%s %s\n", CurIns->mnemonic, CurIns->op_str);
	}
	cs_free(InstructionInfo, InstructionCount);
}

void PLH::AbstractDetour::_Relocate(cs_insn* CurIns, DWORD64 From, DWORD64 To, const uint8_t DispSize, const uint8_t DispOffset)
{
	XTrace("Relocating...\n");

	ASMHelper::DISP DispType = m_ASMInfo.GetDisplacementType(DispSize);
	if (DispType == ASMHelper::DISP::D_INT8)
	{
		int8_t Disp = m_ASMInfo.GetDisplacement<int8_t>(CurIns->bytes, DispOffset);
		Disp -= (To - From);
		*(int8_t*)(CurIns->address + DispOffset) = Disp;
	}else if (DispType == ASMHelper::DISP::D_INT16) {
		int16_t Disp = m_ASMInfo.GetDisplacement<int16_t>(CurIns->bytes, DispOffset);
		Disp -= (To - From);
		*(int16_t*)(CurIns->address + DispOffset) = Disp;
	}else if (DispType == ASMHelper::DISP::D_INT32) {
		int32_t Disp = m_ASMInfo.GetDisplacement<int32_t>(CurIns->bytes, DispOffset);
		Disp -= (To - From);
		*(int32_t*)(CurIns->address + DispOffset) = Disp;
	}
}

void PLH::AbstractDetour::FlushSrcInsCache()
{
	FlushInstructionCache(GetCurrentProcess(), m_hkSrc, m_hkLength);
}

void PLH::AbstractDetour::Initialize(cs_mode Mode)
{
	if (cs_open(CS_ARCH_X86, Mode, &m_CapstoneHandle) != CS_ERR_OK)
		XTrace("Error Initializing Capstone x86\n");

	cs_option(m_CapstoneHandle, CS_OPT_DETAIL, CS_OPT_ON);
}

void PLH::AbstractDetour::RelocateConditionalJMP(cs_insn* CurIns, DWORD& CodeSize, DWORD64 From, DWORD64 To, const uint8_t DispSize, const uint8_t DispOffset)
{
	/*This function automatically begins to build a jump table at the end of the trampoline to allow relative jumps to function properly:
	-Changes relative jump to point to an absolute jump
	-Absolute jump then does the long distance to jump to where the relative jump originally went
	*/
	ASMHelper::DISP DispType = m_ASMInfo.GetDisplacementType(DispSize);
	DWORD64 TrampolineEnd = To + CodeSize;
	if (DispType == ASMHelper::DISP::D_INT8)
	{
		int8_t Disp = m_ASMInfo.GetDisplacement<int8_t>(CurIns->bytes, DispOffset);
		DWORD64 OriginalDestination = CurIns->address + (Disp - (To - From)) + CurIns->size;
		WriteJMP(TrampolineEnd, OriginalDestination);
		Disp = CalculateRelativeDisplacement<int8_t>(CurIns->address, (DWORD64)TrampolineEnd, CurIns->size); //set relative jmp to go to our absolute
		*(int8_t*)(CurIns->address + DispOffset) = Disp;
		CodeSize += GetJMPSize();
	}else if (DispType == ASMHelper::DISP::D_INT16) {
		int16_t Disp = Disp = m_ASMInfo.GetDisplacement<int16_t>(CurIns->bytes, DispOffset);
		DWORD64 OriginalDestination = CurIns->address + (Disp - (To - From)) + CurIns->size;
		WriteJMP(TrampolineEnd, OriginalDestination);
		Disp = CalculateRelativeDisplacement<int16_t>(CurIns->address, (DWORD64)TrampolineEnd, CurIns->size);
		*(int16_t*)(CurIns->address + DispOffset) = Disp;
		CodeSize += GetJMPSize();
	}else if (DispType == ASMHelper::DISP::D_INT32) {
		int32_t Disp = Disp = m_ASMInfo.GetDisplacement<int32_t>(CurIns->bytes, DispOffset);
		DWORD64 OriginalDestination = CurIns->address + (Disp - (To - From)) + CurIns->size;
		WriteJMP(TrampolineEnd, OriginalDestination);
		Disp = CalculateRelativeDisplacement<int32_t>(CurIns->address, (DWORD64)TrampolineEnd, CurIns->size);
		*(int32_t*)(CurIns->address + DispOffset) = Disp;
		CodeSize += GetJMPSize();
	}
}

/*----------------------------------------------*/
#ifndef _WIN64
PLH::X86Detour::X86Detour() : AbstractDetour()
{

}

PLH::X86Detour::~X86Detour()
{
	if (m_NeedFree)
		delete[] m_Trampoline;
}

PLH::HookType PLH::X86Detour::GetType()
{
	return PLH::HookType::X86Detour;
}

bool PLH::X86Detour::Hook()
{
	DWORD OldProtection;

	m_hkLength = CalculateLength(m_hkSrc, 5);
	m_OriginalLength = m_hkLength;
	if (m_hkLength == 0)
	{
		XTrace("Function to small to hook\n");
		return false;
	}

	m_Trampoline = new BYTE[m_hkLength + 30];   //Allocate Space for original plus extra to jump back and for jmp table
	m_NeedFree = true;
	VirtualProtect(m_Trampoline, m_hkLength + 5, PAGE_EXECUTE_READWRITE, &OldProtection); //Allow Execution
	
	memcpy(m_OriginalCode, m_hkSrc, m_hkLength);
	memcpy(m_Trampoline, m_hkSrc, m_hkLength); //Copy original into allocated space
	RelocateASM(m_Trampoline, m_hkLength, (DWORD)m_hkSrc, (DWORD)m_Trampoline);
	WriteRelativeJMP((DWORD)&m_Trampoline[m_hkLength], (DWORD)m_hkSrc + m_hkLength); //JMP back to original code

	//Change protection to allow write on original function
	MemoryProtect Protector(m_hkSrc, m_hkLength, PAGE_EXECUTE_READWRITE);
	//Encode Jump from Hooked Function to the Destination function
	WriteRelativeJMP((DWORD)m_hkSrc, (DWORD)m_hkDest);

	//Write nops over bytes of overwritten instructions
	for (int i = 5; i < m_OriginalLength; i++)
		m_hkSrc[i] = 0x90;
	FlushSrcInsCache();

	PostError(RuntimeError(RuntimeError::Severity::Warning, "PolyHook x86Detour: Some opcodes may not be relocated properly"));
	return true;
	/*Original
	-JMP Destination
	-NOP (extends to length of overwritten opcode)
	-Rest of function

	Destination
	-Do your shit
	-Return Trampoline (goes to trampoline)

	Trampoline
	-Execute Overwritten Opcodes
	-Patch original relative jmps to point to jump table (JE Jumptable entry 1)
	-JMP to rest of function (in original)
	-*BEGIN JUMPTABLE*
	-1)JMP to location of relative jmp one
	-2)...continue pattern for all relative jmps
	*/
}

x86_reg PLH::X86Detour::GetIpReg()
{
	return X86_REG_EIP;
}

void PLH::X86Detour::FreeTrampoline()
{
	if (m_NeedFree)
	{
		delete[] m_Trampoline;
		m_NeedFree = false;
	}
}

void PLH::X86Detour::WriteRelativeJMP(DWORD Destination, DWORD JMPDestination)
{
	*(BYTE*)Destination = 0xE9;       //Write jump opcode to jump back to non overwritten code
	*(long*)(Destination + 1) = CalculateRelativeDisplacement<long>(Destination, JMPDestination, 5);
}

void PLH::X86Detour::WriteJMP(DWORD_PTR From, DWORD_PTR To)
{
	WriteRelativeJMP(From, To);
}

#else
PLH::X64Detour::X64Detour() :AbstractDetour()
{

}

PLH::X64Detour::~X64Detour()
{
	FreeTrampoline();
}

PLH::HookType PLH::X64Detour::GetType()
{
	return PLH::HookType::X64Detour;
}

bool PLH::X64Detour::Hook()
{
	//Allocate Memory as close as possible to src, to minimize chance 32bit displacements will be out of range (for relative jmp type)
	MEMORY_BASIC_INFORMATION mbi;
	for (size_t Addr = (size_t)m_hkSrc; Addr > (size_t)m_hkSrc - 0x80000000; Addr = (size_t)mbi.BaseAddress - 1)
	{
		if (!VirtualQuery((LPCVOID)Addr, &mbi, sizeof(mbi)))
			break;

		if (mbi.State != MEM_FREE)
			continue;

		if (m_Trampoline = (BYTE*)VirtualAlloc(mbi.BaseAddress, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE))
			break;
	}
	if (!m_Trampoline)
		return false;
	m_NeedFree = true;

	//Decide which jmp type to use based on function size
	bool UseRelativeJmp = false;
	m_hkLength = CalculateLength(m_hkSrc, 16); //More stable 16 byte jmp
	m_OriginalLength = m_hkLength; //We modify hkLength in Relocation routine
	if (m_hkLength == 0)
	{
		UseRelativeJmp = true;
		m_hkLength = CalculateLength(m_hkSrc, 6); //Smaller, less safe 6 byte (jmp could be out of bounds)
		if (m_hkLength == 0)
		{
			PostError(RuntimeError(RuntimeError::Severity::UnRecoverable, "PolyHook x64Detour: Function to small to hook"));
			return false;
		}
	}

	memcpy(m_OriginalCode, m_hkSrc, m_hkLength);
	memcpy(m_Trampoline, m_hkSrc, m_hkLength);
	RelocateASM(m_Trampoline,m_hkLength, (DWORD64)m_hkSrc, (DWORD64)m_Trampoline);
	//Write the jmp from our trampoline back to the original
	WriteAbsoluteJMP((DWORD64)&m_Trampoline[m_hkLength], (DWORD64)m_hkSrc + m_hkLength); 

	// Build a far jump to the Destination function. (jmps not to address pointed at but to the value in the address)
	MemoryProtect Protector(m_hkSrc, m_hkLength, PAGE_EXECUTE_READWRITE);
	int HookSize = 0;
	if (UseRelativeJmp)
	{
		HookSize = 6;
		m_hkSrc[0] = 0xFF;
		m_hkSrc[1] = 0x25;
		//Write 32Bit Displacement from rip
		*(long*)(m_hkSrc + 2) = CalculateRelativeDisplacement<long>((DWORD64)m_hkSrc, (DWORD64)&m_Trampoline[m_hkLength + 16], 6);
		*(DWORD64*)&m_Trampoline[m_hkLength + 16] = (DWORD64)m_hkDest; //Write the address into memory at [RIP+Displacement]
	}else {
		HookSize = 16;
		WriteAbsoluteJMP((DWORD64)m_hkSrc, (DWORD64)m_hkDest);
	}
	//Nop Extra bytes from overwritten opcode
	for (int i = HookSize; i < m_OriginalLength; i++)
		m_hkSrc[i] = 0x90;

	FlushInstructionCache(GetCurrentProcess(), m_hkSrc, m_hkLength);
	PostError(RuntimeError(RuntimeError::Severity::Warning, "PolyHook x64Detour: Relocation can be out of range"));
	return true;
}

x86_reg PLH::X64Detour::GetIpReg()
{
	return X86_REG_RIP;
}

void PLH::X64Detour::FreeTrampoline()
{
	if (m_NeedFree)
	{
		VirtualFree(m_Trampoline, 0, MEM_RELEASE);
		m_NeedFree = false;
	}
}

void PLH::X64Detour::WriteAbsoluteJMP(DWORD64 Destination, DWORD64 JMPDestination)
{
	/*push rax
	mov rax ...   //Address to original
	xchg qword ptr ss:[rsp], rax
	ret*/
	BYTE detour[] = { 0x50, 0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x87, 0x04, 0x24, 0xC3 };
	memcpy((BYTE*)Destination, detour, sizeof(detour));
	*(DWORD64*)&((BYTE*)Destination)[3] = JMPDestination;
}

void PLH::X64Detour::WriteJMP(DWORD_PTR From, DWORD_PTR To)
{
	WriteAbsoluteJMP(From, To);
}

int PLH::X64Detour::GetJMPSize()
{
	return 16;
}
#endif

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

LONG CALLBACK PLH__VEHHook__VEHHandler(PLH__VEHHOOK_ALL_SHIT_POLYHOOK_S * all_shit, EXCEPTION_POINTERS* ExceptionInfo)
{
#ifdef _WIN64
	#define XIP Rip
#else
	#define XIP Eip
#endif // _WIN64
	std::lock_guard<std::mutex> m_Lock(all_shit->m_TargetMutex);

	DWORD ExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
	if (ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		for (int i = 0; i < all_shit->m_HookTargets_Size;i++)
		{
			if(((all_shit->m_HookTargets[i]).m_Type) != VEHMethod__INT3_BP)
				continue;

			//Are we at a breakpoint that we placed?
			if (ExceptionInfo->ContextRecord->XIP != ((DWORD_PTR)((all_shit->m_HookTargets[i]).m_Src)))
				continue;

			//Remove Int3 Breakpoint
			PLH__MemoryProtect__MemoryProtect_Protect(((all_shit->m_HookTargets[i]).m_Src), 1, PAGE_EXECUTE_READWRITE);
			*((all_shit->m_HookTargets[i]).m_Src) = ((all_shit->m_HookTargets[i]).m_StorageByte);
			
			//Set instruction pointer to our callback
			ExceptionInfo->ContextRecord->XIP = ((DWORD_PTR) ((all_shit->m_HookTargets[i]).m_Dest));
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}else if (ExceptionCode == EXCEPTION_SINGLE_STEP) {
		
		//Intel says clear Dr6, windows may do it for us, lets be safe
		ExceptionInfo->ContextRecord->Dr6 = 0; 
		for (int i = 0; i < all_shit->m_HookTargets_Size;i++)
		{
			
			if (((all_shit->m_HookTargets[i]).m_Type) != VEHMethod__HARDWARE_BP)
				continue;
		
			//Are we at a breakpoint that we placed?
			if (ExceptionInfo->ContextRecord->XIP != ((DWORD_PTR)((all_shit->m_HookTargets[i]).m_Src)))
				continue;
		
			//Clear the Debug Register
			ExceptionInfo->ContextRecord->Dr7 &= ~(1 << (2 * ((all_shit->m_HookTargets[i]).m_StorageByte)));

			//Set instruction pointer to callback
			ExceptionInfo->ContextRecord->XIP = ((DWORD_PTR)((all_shit->m_HookTargets[i]).m_Dest));
			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}else if (ExceptionCode == EXCEPTION_GUARD_PAGE) {
		for (int i = 0; i < all_shit->m_HookTargets_Size;i++)
		{
			//still need to check if exception is in our page
			if (((all_shit->m_HookTargets[i]).m_Type) != VEHMethod__GUARD_PAGE)
				continue;

			if(!PLH__VEHHook__AreInSamePage(((BYTE*)(ExceptionInfo->ContextRecord->XIP)), ((all_shit->m_HookTargets[i]).m_Src) )))
				continue;

			if (ExceptionInfo->ContextRecord->XIP == ((DWORD_PTR)((all_shit->m_HookTargets[i]).m_Dest)))
				ExceptionInfo->ContextRecord->XIP = ((DWORD_PTR)((all_shit->m_HookTargets[i]).m_Dest));

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

/*----------------------------------------------*/
void PLH__MemoryProtect__MemoryProtect_init(MEMORY_PROTECT_ALL_SHIT_POLYHOOK_S * all_shit,void* Address, size_t Size, DWORD ProtectionFlags)
{
	all_shit->m_Address = Address;
	all_shit->m_Size = Size;
	all_shit->m_Flags = ProtectionFlags;
	PLH__MemoryProtect__Protect(all_shit->m_Address, all_shit->m_Size,all_shit->m_Flags);
}

BOOL PLH__MemoryProtect__Protect(void* Address, size_t Size, DWORD ProtectionFlags)
{
	return VirtualProtect(Address, Size, ProtectionFlags, &m_OldProtection);
}

PLH__MemoryProtect__MemoryProtect_deinit(MEMORY_PROTECT_ALL_SHIT_POLYHOOK_S * all_shit)
{
	PPLH__MemoryProtect__Protect(all_shit->m_Address,all_shit->m_Size,all_shit->m_OldProtection);
}
#endif//end include guard