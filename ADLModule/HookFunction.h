#pragma once

#pragma optimize("", off)
#pragma warning (disable : 4127)

void ADL_EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void ADL_LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

#define NAKEDDEF __declspec(naked)

//현재는 CriticalSection에 대해서만 처리하고 있고 그 외의 것은 필요 없어 보이나
//미래에 다른 동기화 프로젝트도 처리가 필요할 경우를 대비하여 선언을 남겨 둔다.
//Naked Function의 프롤로그와 에필로그 처리는 존 로빈스씨가 만든 것임

#define DDOPT_WAIT 0x00000000
#define DDOPT_THREADS 0x00000001
#define DDOPT_CRITSEC 0x00000002
#define DDOPT_MUTEX 0x00000004
#define DDOPT_SEMAPORE 0x00000010
#define DDOPT_EVENT 0x00000020

#define DDOPT_ALL (DDOPT_THREADS | \
				   DDOPT_CRITSEC | \
				   DDOPT_MUTEX | \
				   DDOPT_SEMAPHORE | \
				   DDOPT_EVENT)

typedef enum tag_eFuncEnum
{
	eUNINITIALIZEDFE = 0,
	eEnterCriticalSection,
	eLeaveCriticalSection,
} eFuncEnum;

typedef enum tag_ePrePostEnum
{
	eUNINITIALIZEDPP = 0,
	ePreCall,
	ePostCall,
	eMAXPREPOSTENUM
} ePrePostEnum;

typedef struct tag_REGSTATE
{
	DWORD dwEAX;
	DWORD dwEBX;
	DWORD dwECX;
	DWORD dwEDX;
	DWORD dwEDI;
	DWORD dwESI;
	DWORD dwEFL; //Flag Register;
} REGSTATE, *PREGSTATE;

typedef struct tag_DDEVENTINFO
{
	eFuncEnum eFunc;
	ePrePostEnum ePrePost;
	DWORD dwAddr;
	DWORD dwThreadId;
	DWORD dwRetValue;
	DWORD dwParams;
} DDEVENTINFO, *LPDDEVENTINFO;

typedef struct tag_HOOKFUNCDESC
{
	LPCSTR szFunc;
	PROC pProc;
}HOOKFUNCDESC, *LPHOOKFUNCDESC;

const LPCTSTR k_KERNEL32 = _T("KERNEL32.DLL");

#define NUMBER_KERNEL_FUNCS 2

static HOOKFUNCDESC g_stDDKernelFuncs[NUMBER_KERNEL_FUNCS] = 
{
	{"EnterCriticalSection", (PROC)ADL_EnterCriticalSection},
	{"LeaveCriticalSection", (PROC)ADL_LeaveCriticalSection},
};

static HOOKFUNCDESC g_stRealKernelFuncs[NUMBER_KERNEL_FUNCS] = 
{
	{"EnterCriticalSection", (PROC)EnterCriticalSection},
	{"LeaveCriticalSection", (PROC)LeaveCriticalSection},
};

#if _WIN32 >= 0x500
#define GET_THREAD_ACP() CP_THREAD_ACP
#else
#define GET_THREAD_ACP() GetACP()
#endif

#define MakePtr(cast, ptr, AddValue) \
			   (cast)((DWORD_PTR)(ptr) + (DWORD_PTR)(AddValue))

#ifdef UNICODE
#define HookImportedFunctionByName HookImportedFunctionByNameW
#else
#define HookImportedFunctionByName HookImportedFunctionByNameA
#endif

#ifdef _DEBUG
#define SAVE_ESI()  __asm PUSH ESI
#define RESTORE_ESI() __asm POP ESI
#else
#define SAVE_ESI()
#define RESTORE_ESI()
#endif

#define HOOKFN_PROLOG() \
DDEVENTINFO stEvtInfo; \
DWORD dwLastError;\
REGSTATE stRegState; \
{\
	__asm PUSH EBP\
	__asm MOV EBP, ESP\
	__asm MOV EAX, ESP\
	SAVE_ESI()\
	__asm SUB ESP, __LOCAL_SIZE\
	__asm ADD EAX, 04h + 04h\
\
	__asm MOV [stEvtInfo.dwParams], EAX\
	__asm SUB EAX, 04h\
	__asm MOV EAX, [EAX]\
\
	__asm MOV [stEvtInfo.dwAddr], EAX\
	__asm MOV dwLastError, 0\
	__asm MOV [stEvtInfo.eFunc], eUNINITIALIZEDFE\
	__asm MOV [stRegState.dwEDI], EDI\
	__asm MOV [stRegState.dwESI], ESI\
}

#define HOOKFN_EPILOG(iNumParams) \
{\
	SetLastError(dwLastError);\
	__asm ADD ESP, __LOCAL_SIZE\
	__asm MOV EBX, [stRegState.dwEBX]\
	__asm MOV ECX, [stRegState.dwECX]\
	__asm MOV EDX, [stRegState.dwEDX]\
	__asm MOV EDI, [stRegState.dwEDI]\
	__asm MOV ESI, [stRegState.dwESI]\
	__asm MOV EAX, [stRegState.dwEFL]\
	__asm SAHF\
	__asm MOV EAX, [stRegState.dwEAX]\
RESTORE_ESI()\
	__asm MOV	ESP, EBP\
	__asm POP EBP\
	__asm RET iNumParams * 4\
}

#define REAL_FUNC_PRE_CALL() \
{\
	__asm MOV EDI, [stRegState.dwEDI] \
	__asm MOV ESI, [stRegState.dwESI] \
}

#define REAL_FUNC_POST_CALL() \
{\
	__asm MOV [stRegState.dwEAX], EAX \
	__asm MOV [stRegState.dwEBX], EBX \
	__asm MOV [stRegState.dwECX], ECX \
	__asm MOV [stRegState.dwEDX], EDX \
	__asm MOV [stRegState.dwEDI], EDI \
	__asm MOV [stRegState.dwESI], ESI \
	__asm XOR EAX, EAX \
	__asm LAHF\
	__asm MOV [stRegState.dwEFL], EAX \
}\
dwLastError = GetLastError();\
{\
	__asm MOV EAX, [stRegState.dwEAX]\
	__asm MOV [stEvtInfo.dwRetValue], EAX\
}

#define HOOKFN_STARTUP(eFunc, SynchClassType, bRecordPreCall) \
	HOOKFN_PROLOG();\
	REAL_FUNC_PRE_CALL();

#define HOOKFN_SHUTDOWN(iNumParams, SynchClass)\
	REAL_FUNC_POST_CALL(); \
	HOOKFN_EPILOG(iNumParams);


typedef VOID (WINAPI* PFNENTERCRITSEC) (LPCRITICAL_SECTION lpCriticalSection);
typedef VOID (WINAPI* PFNLEAVECRITSEC) (LPCRITICAL_SECTION lpCriticalSection);

static VOID NAKEDDEF ADL_EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	HOOKFN_STARTUP(eEnterCriticalSection, DDOPT_CRITSEC, TRUE);
	ADLDeadLockInfo::GetInstance()->SetInfomation(1);
	PFNENTERCRITSEC(g_stRealKernelFuncs[eEnterCriticalSection - 1].pProc)(lpCriticalSection);
	HOOKFN_SHUTDOWN(1, DDOPT_CRITSEC);
}

static VOID NAKEDDEF ADL_LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{
	HOOKFN_STARTUP(eLeaveCriticalSection, DDOPT_CRITSEC, FALSE);
	ADLDeadLockInfo::GetInstance()->SetInfomation(0);
	PFNLEAVECRITSEC(g_stRealKernelFuncs[eLeaveCriticalSection - 1].pProc)(lpCriticalSection);
	HOOKFN_SHUTDOWN(1, DDOPT_CRITSEC);
}
