#include "StdAfx.h"
#include "ADLLock.h"

ADLLock::ADLLock(void) 
{
	InitializeCriticalSection(&m_CS);
}

ADLLock::~ADLLock(void) 
{
	DeleteCriticalSection(&m_CS);
}
