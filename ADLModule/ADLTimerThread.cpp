#include "StdAfx.h"
#include "ADLTimerThread.h"

ADLTimerThread::ADLTimerThread(DWORD dwTick)
: m_hWaitEvent(NULL)
, m_dwInterval(dwTick)						   
{
	m_hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

ADLTimerThread::~ADLTimerThread(void)
{
}

void ADLTimerThread::BeginTimer()
{
	CreateThread();
}

void ADLTimerThread::EndTimer()
{
	::SetEvent(m_hWaitEvent);

	EndThread();
}

unsigned ADLTimerThread::Run()
{
	DWORD dwWaitInterval = m_dwInterval;
 
	while(TRUE)
	{
		DWORD dwRet = WAIT_FAILED;

		dwRet = ::WaitForSingleObject(m_hWaitEvent, dwWaitInterval);

		if(WAIT_TIMEOUT == dwRet)
		{
			DWORD dwBeforeTick = ::GetTickCount();

			OnTimer();

			DWORD dwAfterTick = ::GetTickCount();
			
			if(dwBeforeTick > dwAfterTick)
			{
				dwWaitInterval = m_dwInterval;
			}
			else if(m_dwInterval > (dwAfterTick - dwBeforeTick))
			{
				dwWaitInterval = m_dwInterval - (dwAfterTick - dwBeforeTick);
			}
			else
			{
				dwWaitInterval = 0;
			}
		}
		else	
		{		
			return 0;		
		}
	}

	return 0;
	
}