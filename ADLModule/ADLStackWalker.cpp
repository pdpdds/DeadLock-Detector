#include "StdAfx.h"
#include "ADLStackWalker.h"
#include <string>

ADLStackWalker::ADLStackWalker(void)
: m_pADLCenter(NULL)
{
	
}

ADLStackWalker::~ADLStackWalker(void)
{
	m_pADLCenter = NULL; 
}

BOOL ADLStackWalker::ReportCallStackByStackWalker(ADLCenter* pADLCenter, HANDLE hHandle)
{
	m_pADLCenter = pADLCenter;
	StackWalker::ShowCallstack(hHandle);

	return TRUE;
}

void ADLStackWalker::OnOutput(LPCSTR szText)
{
	if(NULL != m_pADLCenter)
		m_pADLCenter->OnOutput(szText);
}

BOOL ADLStackWalker::ReportCallStackByAddress( ADLCenter* pADLCenter, HANDLE hHandle )
{
	m_pADLCenter = pADLCenter;
	
//추가 예정.

	return TRUE;
}