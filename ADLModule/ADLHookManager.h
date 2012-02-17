#pragma once
#include <process.h>
#include <psapi.h>
#include "HookFunction.h"

class ADLHookManager
{
public:
	ADLHookManager(void);
	virtual ~ADLHookManager(void);

	BOOL ADLHookManager::SaveOriginalFunctionAddress(void);
	DWORD __stdcall ADLHookManager::BSUWide2Ansi(const wchar_t* szWide, CHAR* szAnsi, INT iAnsiLen);
	BOOL ADLHookManager::StartFunctionHook(HMODULE hBaseModule, DWORD& dwHookFunctionCount);
	BOOL ADLHookManager::EndFunctionHook(HMODULE hBaseModule, DWORD& dwHookFunctionCount);
	PIMAGE_IMPORT_DESCRIPTOR ADLHookManager::GetNamedImportDescriptor(HMODULE hModule, LPCSTR szImportMod);
	BOOL __stdcall ADLHookManager::HookImportedFunctionByNameW(HMODULE hModule, LPCWSTR szImportMod, UINT uiCount, LPHOOKFUNCDESC paHookArray, PROC* paOrigFuncs, LPDWORD pdwHooked);
	BOOL __stdcall ADLHookManager::HookImportedFunctionByNameA(HMODULE hModule, LPCSTR szImportMod, UINT uiCount, LPHOOKFUNCDESC paHookArray, PROC* paOrigFuncs, LPDWORD pdwHooked);

	static ADLHookManager* GetInstance()
	{
		if(NULL == m_pADLHookManager)
		{
			m_pADLHookManager = new ADLHookManager();
		}

		return m_pADLHookManager;
	}	

protected:

private:
	static ADLHookManager* m_pADLHookManager;
};
