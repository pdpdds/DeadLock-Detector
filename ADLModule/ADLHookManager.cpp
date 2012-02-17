#include "StdAfx.h"
#include "ADLHookManager.h"

ADLHookManager* ADLHookManager::m_pADLHookManager = NULL;

ADLHookManager::ADLHookManager(void)
{
}

ADLHookManager::~ADLHookManager(void)
{
}

BOOL ADLHookManager::SaveOriginalFunctionAddress(void)
{
	HMODULE hKernel = GetModuleHandle(k_KERNEL32);

	if(NULL != hKernel)
	{
		for (INT iIndex = 0; iIndex < NUMBER_KERNEL_FUNCS; iIndex++)
		{
			if (NULL == g_stRealKernelFuncs[iIndex].pProc)
			{
				g_stRealKernelFuncs[iIndex].pProc = (PROC)GetProcAddress(hKernel, g_stRealKernelFuncs[iIndex].szFunc);
			}   
		}
	}

	return TRUE;
}

DWORD __stdcall ADLHookManager::BSUWide2Ansi(const wchar_t* szWide, CHAR* szAnsi, INT iAnsiLen)
{
	//멀티바이트쪽도 버퍼를 할당해서 사용해야 한다.

	if(NULL == szWide || NULL == szAnsi)
		return 1000;

	DWORD dwRet = WideCharToMultiByte(GET_THREAD_ACP(), 0, szWide, -1, szAnsi, iAnsiLen, NULL, NULL);

	return dwRet;
}

BOOL ADLHookManager::StartFunctionHook(HMODULE hBaseModule, DWORD& dwHookFunctionCount)
{
	dwHookFunctionCount = 0;

	if((FALSE == HookImportedFunctionByName(hBaseModule, k_KERNEL32, NUMBER_KERNEL_FUNCS, g_stDDKernelFuncs, NULL, &dwHookFunctionCount)))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL ADLHookManager::EndFunctionHook(HMODULE hBaseModule, DWORD& dwHookFunctionCount)
{
	dwHookFunctionCount = 0;

	if((FALSE == HookImportedFunctionByName(hBaseModule, k_KERNEL32, NUMBER_KERNEL_FUNCS, g_stRealKernelFuncs, NULL, &dwHookFunctionCount)))
	{
		return FALSE;
	}

	return TRUE;
}

PIMAGE_IMPORT_DESCRIPTOR ADLHookManager::GetNamedImportDescriptor(HMODULE hModule, LPCSTR szImportMod)
{
	if(NULL == szImportMod || NULL == hModule)
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return NULL;
	}

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;

	if((TRUE == IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER))) || (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic))
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return NULL;
	}

	PIMAGE_NT_HEADERS pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDosHeader, pDosHeader->e_lfanew);

	if((TRUE == IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS))) || (IMAGE_NT_SIGNATURE != pNTHeader->Signature))	
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return NULL;
	}

	if(0 == pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress)
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return NULL;	
	}

	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDosHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	while(NULL != pImportDesc->Name)
	{
		PSTR szCurrMod = MakePtr(PSTR, pDosHeader, pImportDesc->Name);

		if(0 == _stricmp(szCurrMod, szImportMod))
		{
			break;
		}

		pImportDesc++;
	}

	if(NULL == pImportDesc->Name)
	{
		return NULL;
	}


	return pImportDesc;
}

BOOL __stdcall ADLHookManager::HookImportedFunctionByNameW(HMODULE hModule, LPCWSTR szImportMod, UINT uiCount, LPHOOKFUNCDESC paHookArray, PROC* paOrigFuncs, LPDWORD pdwHooked)
{
	INT iLen = lstrlenW(szImportMod);

	CHAR* pAnsiStr = (CHAR*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, iLen + 1);

	if(FALSE == BSUWide2Ansi(szImportMod, pAnsiStr, iLen + 1))
	{
		return FALSE;
	}

	BOOL bRet = HookImportedFunctionByNameA(hModule, pAnsiStr, uiCount, paHookArray, paOrigFuncs, pdwHooked);

	HeapFree(GetProcessHeap(), 0, pAnsiStr);

	return bRet;
}

BOOL __stdcall ADLHookManager::HookImportedFunctionByNameA(HMODULE hModule, LPCSTR szImportMod, UINT uiCount, LPHOOKFUNCDESC paHookArray, PROC* paOrigFuncs, LPDWORD pdwHooked)
{
	if((0 == uiCount) || (NULL == szImportMod) || (TRUE == IsBadReadPtr(paHookArray, sizeof(HOOKFUNCDESC) * uiCount)))
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return FALSE;	
	}

	if((NULL != paOrigFuncs) && (TRUE == IsBadWritePtr(paOrigFuncs, sizeof(PROC) * uiCount)))
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return FALSE;	
	}

	if((NULL != pdwHooked) && (TRUE == IsBadWritePtr(pdwHooked, sizeof(UINT))))
	{
		SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
		return FALSE;	
	}

	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = GetNamedImportDescriptor(hModule, szImportMod);

	if(NULL == pImportDesc)
	{
//해당 모듈이 임포트 되어 있지 않으므로 TRUE 리턴
		return TRUE;
	}

	if((NULL == pImportDesc->OriginalFirstThunk) || (NULL == pImportDesc->FirstThunk))
	{
//임포트 모듈이 있지만 실제 어떤 함수도 임포트 하지 않으므로 TRUE 리턴
		SetLastError(ERROR_SUCCESS);
		return FALSE;	
	}

	PIMAGE_THUNK_DATA pOrigThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->OriginalFirstThunk);
	PIMAGE_THUNK_DATA pRealThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->FirstThunk);

	while(NULL != pOrigThunk->u1.Function)
	{
		if(IMAGE_ORDINAL_FLAG != (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG))
		{

			PIMAGE_IMPORT_BY_NAME pByName;

			pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, hModule, pOrigThunk->u1.AddressOfData);

			if('\0' == pByName->Name[0])
			{
				pOrigThunk++;
				pRealThunk++;
				continue;
			}


			BOOL bDoHook = FALSE;

			UINT uiIndex = 0;


			for(uiIndex = 0; uiIndex < uiCount; uiIndex++)
			{
				if((paHookArray[uiIndex].szFunc[0] == pByName->Name[0]) && (0 == _strcmpi(paHookArray[uiIndex].szFunc, (CHAR*)pByName->Name)))
				{
					if(NULL != paHookArray[uiIndex].pProc)
					{
						bDoHook = TRUE;
					}
					break;
				}
			}

//후킹할 함수엔트리에 새 함수를 쓰기 위해 메모리 속성을 읽기 쓰기 가능으로 바꾼다.
			if(TRUE == bDoHook)
			{
				MEMORY_BASIC_INFORMATION mbi_thunk;

				VirtualQuery(pRealThunk, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION));

				if(FALSE == VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect))
				{		
					SetLastErrorEx(ERROR_INVALID_HANDLE, SLE_ERROR);
					return FALSE;		
				}

				if (NULL != paOrigFuncs)
				{
					paOrigFuncs[uiIndex] = (PROC)((INT_PTR)pRealThunk->u1.Function);
				}

				DWORD_PTR* pTemp = (DWORD_PTR*)&pRealThunk->u1.Function;
				*pTemp = (DWORD_PTR)(paHookArray[uiIndex].pProc);

				DWORD dwOldProtect;

				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);

				if(NULL != pdwHooked)
				{
					*pdwHooked += 1;
				}
				
			}
		}

		pOrigThunk++;
		pRealThunk++;
	}

	SetLastError(ERROR_SUCCESS);
	return TRUE;	
}


