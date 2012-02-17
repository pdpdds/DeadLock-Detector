#include "StdAfx.h"
#include "ADLCenter.h"
#include "ADLDeadLockInfo.h"
#include "ADLHookManager.h"
#include "ADLDeadLockDetectionThread.h"

ADLCenter::ADLCenter(void)
: m_pDeadLockDetectionThread(NULL)
{
	SetReportMode(ADL_STACKREPORT_PDB);
}

ADLCenter::~ADLCenter(void)
{		
	m_vecDeadLockThreadList.clear();
}

BOOL ADLCenter::Begin()
{
	m_pDeadLockDetectionThread = new ADLDeadLockDetectionThread(ADL_THREAD_INTERVAL);

	if(NULL == m_pDeadLockDetectionThread)
	{
		return FALSE;
	}

	HANDLE hEvent;
	hEvent = CreateEvent(NULL, TRUE, FALSE, _T("DeadLockDetectionThreadEvent"));

	m_pDeadLockDetectionThread->SetWaitEvent(hEvent);
	m_pDeadLockDetectionThread->SetADLSenter(this);

	m_pDeadLockDetectionThread->BeginTimer();

	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);

	ADLDeadLockInfo* pADLDeadLockInfo = ADLDeadLockInfo::GetInstance();

	if(NULL == pADLDeadLockInfo)
	{
		return FALSE;
	}
	else
	{
		HINSTANCE hBaseModule = GetModuleHandle(NULL);

		ADLHookManager* pADLHookManager = ADLHookManager::GetInstance();

		pADLHookManager->SaveOriginalFunctionAddress();

		DWORD dwHookFunctionCount = 0;
		BOOL bResult = pADLHookManager->StartFunctionHook(hBaseModule, dwHookFunctionCount);
		

		if(FALSE == bResult)
			return FALSE;

		OnOutput("Function Hook Ok.\n");
	}

	OnOutput("Start ADL Thread\n");

	return TRUE;
}

BOOL ADLCenter::End()
{
	ADLHookManager* pADLHookManager = ADLHookManager::GetInstance();
	ADLDeadLockInfo* pADLDeadLockInfo = ADLDeadLockInfo::GetInstance();

	HINSTANCE hBaseModule = GetModuleHandle(NULL);
	DWORD dwHookFunctionCount = 0;
	BOOL bResult = pADLHookManager->StartFunctionHook(hBaseModule, dwHookFunctionCount);

	pADLHookManager->EndFunctionHook(hBaseModule, dwHookFunctionCount);

	OnOutput("Function Hook Rollback Ok\n");
	
	if(NULL != m_pDeadLockDetectionThread)
	{
		delete m_pDeadLockDetectionThread;
	}
	
	delete pADLHookManager;
	delete pADLDeadLockInfo;

	OnOutput("End ADL Thread\n");

	return TRUE;
}

void ADLCenter::OnOutput( LPCSTR szText )
{	
	printf("%s", szText);
}

void ADLCenter::PushDeadLockThreadID( DWORD dwThreadID )
{
	ADLLockHelper LockHelper(&m_ADCLock);
	m_vecDeadLockThreadList.push_back(dwThreadID);
}

BOOL ADLCenter::KillDeadLockThread()
{
	ADLLockHelper LockHelper(&m_ADCLock);

	if(0 == m_vecDeadLockThreadList.size())
	{
		OnOutput("DeadLock Thread Not Detected\n");
	}
	else
	{
		char szBuf[100];
		std::vector<DWORD>::iterator iter = m_vecDeadLockThreadList.begin();

		for(;iter != m_vecDeadLockThreadList.end(); iter++)
		{
			DWORD dwThreadID = (*iter);
			sprintf_s(szBuf, "Kiil DeadLock Thread. ID %d\n", dwThreadID);
			OnOutput(szBuf);

			HANDLE hThread = OpenThread(THREAD_GET_CONTEXT, FALSE, dwThreadID);

			if(NULL != hThread)
			{
				TerminateThread(hThread, 0);				
			}
		}
	}

	return TRUE;
}