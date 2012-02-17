#include "StdAfx.h"
#include "ADLStupidThread.h"

ADLStupidThread::ADLStupidThread(ADLLock* pLock1, ADLLock* pLock2)
{
	m_pLock1 = pLock1;
	m_pLock2 = pLock2;

	m_bStopSignal = FALSE;
}

ADLStupidThread::~ADLStupidThread(void)
{	
}

void ADLStupidThread::Function1()
{	
	m_pLock1->Lock();
	m_pLock2->Lock();
	m_pLock2->Unlock();
	m_pLock1->Unlock();
}

void ADLStupidThread::Function2()
{
	m_pLock2->Lock();
	m_pLock1->Lock();
	m_pLock1->Unlock();
	m_pLock2->Unlock();
}

unsigned ADLStupidThread::Run()
{	
	while(m_bStopSignal == FALSE)
	{		
		Function1();
		Function2();		
	}

	return 0;	
}
