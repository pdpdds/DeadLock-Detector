#include "StdAfx.h"
#include "ADLThread.h"

ADLThread::ADLThread(void)
: m_hThread((HANDLE)0)
{
}

ADLThread::~ADLThread(void)
{
	EndThread();
}

BOOL ADLThread::OnStartUp()
{
	return TRUE;
}

BOOL ADLThread::CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, 
						   DWORD dwStackSize, 
						   DWORD dwCreationFlags, 
						   LPDWORD lpThreadID)
{
	m_hThread = (HANDLE) _beginthreadex((void*) lpThreadAttributes, 
										(unsigned)dwStackSize, __Run, 
										(void*)this, 
										(unsigned) dwCreationFlags, 
										(unsigned*)lpThreadID);

	return (((HANDLE)0) != m_hThread);
}

void ADLThread::EndThread()
{
	if(((HANDLE)0) == m_hThread)
	{
		return;
	}

	BOOL bRet = FALSE;

	bRet = (WAIT_FAILED != ::WaitForSingleObjectEx(m_hThread, INFINITE, FALSE));

	if(FALSE == bRet)
	{
		return;
	}

	::CloseHandle(m_hThread);

	m_hThread = (HANDLE) 0;
}

unsigned __stdcall ADLThread::__Run(void* pParameter)
{
	srand((unsigned int)time(NULL));

	if (FALSE == ((ADLThread*)pParameter)->OnStartUp())
	{
		return 1;
	}

	return ((ADLThread*)pParameter)->Run();
}