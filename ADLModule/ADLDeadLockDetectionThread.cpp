#include "StdAfx.h"
#include "ADLDeadLockInfo.h"
#include "ADLDeadLockDetectionThread.h"

ADLDeadLockDetectionThread::ADLDeadLockDetectionThread(DWORD dwTimerInterval)
: ADLTimerThread(dwTimerInterval)
, m_dwProcessID(0)
, m_hEvent(0)
, m_hProcess(0)
, m_dwTargetThreadID(0)
, m_pADLCenter(NULL)
, m_WasDeadLockReport(FALSE)
{	
}

ADLDeadLockDetectionThread::~ADLDeadLockDetectionThread(void)
{
	EndTimer();	
}

BOOL ADLDeadLockDetectionThread::Initialize()
{
	SetProcessID(GetCurrentProcessId());

	return TRUE;
}

BOOL ADLDeadLockDetectionThread::OnStartUp()
{
	BOOL bResult = Initialize();

	if(m_hEvent != NULL)
	{
		SetEvent(m_hEvent);
	}

	return bResult;
}

void ADLDeadLockDetectionThread::OnTimer()
{
	if(TRUE == m_WasDeadLockReport)
		return;

	ADLDeadLockInfo* pDeadLockInfo = ADLDeadLockInfo::GetInstance();

	if(NULL == pDeadLockInfo)
		return;

	if (TRUE == pDeadLockInfo->IsDeadLockDetected(GetTickCount()))
	{
		ReportDeadLock(pDeadLockInfo);
		m_WasDeadLockReport = TRUE;
	}
}

BOOL ADLDeadLockDetectionThread::ReportDeadLock(ADLDeadLockInfo* pDeadLockInfo)
{
	stDeadLockThreadInfo* pDeadLockThreadInfo = pDeadLockInfo->GetThreadInfo();

	char szBuf[100];

	for(INT iIndex = 0; iIndex < pDeadLockInfo->GetThreadSize(); iIndex++)
	{
		if(TRUE == pDeadLockThreadInfo[iIndex].bIsDeadLockThread)
		{			
			m_pADLCenter->PushDeadLockThreadID(pDeadLockThreadInfo[iIndex].dwThreadID);

			sprintf_s(szBuf, "Dead Lock Thread!! ID :%d, Time : %d\n", pDeadLockThreadInfo[iIndex].dwThreadID, pDeadLockThreadInfo[iIndex].dwTickCount);
			m_pADLCenter->OnOutput(szBuf);
			m_pADLCenter->OnOutput("------------------\n");			

			if(FALSE == Report(pDeadLockThreadInfo[iIndex].dwThreadID))
			{
				if(m_pADLCenter)
					m_pADLCenter->OnOutput("Thread CallStack Report Fail!!\n");
			}
			m_pADLCenter->OnOutput("------------------\n");			
			m_pADLCenter->OnOutput("\n");
		}
	}
	
	return TRUE;
}

BOOL ADLDeadLockDetectionThread::ProcessReportCallStackByPDB(HANDLE hThread)
{
	if(NULL == m_pADLCenter)
		return FALSE;
	
	m_StackWalker.ReportCallStackByStackWalker(m_pADLCenter, hThread);

	return TRUE;
}

BOOL ADLDeadLockDetectionThread::ProcessReportCallStackByAddresss(HANDLE hThread)
{
	if(NULL == m_pADLCenter)
		return FALSE;
	
	m_StackWalker.ReportCallStackByAddress(m_pADLCenter, hThread);

	return TRUE;
}

BOOL ADLDeadLockDetectionThread::Report( DWORD dwThreadID )
{
	DWORD dwErrorCode = 0;

	HANDLE hThread = OpenThread(THREAD_GET_CONTEXT, FALSE, dwThreadID);

	if(NULL == hThread)
	{
		dwErrorCode = GetLastError();
		return FALSE;
	}

	switch(GetReportMode())
	{
	case ADL_STACKREPORT_PDB:
		{
			ProcessReportCallStackByPDB(hThread);
		}
		break;

	case ADL_STACKREPORT_ADDRESS:
		{
			ProcessReportCallStackByAddresss(hThread);
		}
		break;
	}

	CloseHandle(hThread);

	return TRUE;
}

REPORT_MODE ADLDeadLockDetectionThread::GetReportMode()
{
	if(NULL == m_pADLCenter)
	{
		return ADL_STACKREPORT_PDB;
	}
	
	return m_pADLCenter->GetReportMode();
}