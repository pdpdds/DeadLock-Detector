#include "StdAfx.h"
#include "ADLDeadLockInfo.h"

ADLDeadLockInfo* ADLDeadLockInfo::m_pDeadLockInfo = NULL;

ADLDeadLockInfo::ADLDeadLockInfo(void)
: m_iTopThreadPos(0)
{	
	memset(m_pDeadLockThreadInfo, 0, sizeof(stDeadLockThreadInfo) * ADL_MAX_THREAD);	
}

ADLDeadLockInfo::~ADLDeadLockInfo(void)
{
}

void ADLDeadLockInfo::SetInfomation(BYTE btEnterCritical)
{
	INT iIndex = 0;
	BOOL bNewThread = TRUE;
	INT iTopThreadPos = m_iTopThreadPos;

	DWORD dwThreadID = GetCurrentThreadId();
	DWORD dwTickCount = GetTickCount();

	if(m_iTopThreadPos >= ADL_MAX_THREAD)
		return;

	for(iIndex = 0; iIndex < iTopThreadPos; iIndex++)
	{
		if(m_pDeadLockThreadInfo[iIndex].dwThreadID == dwThreadID)
		{
			bNewThread = FALSE;
			break;
		}
	}

	if(ADL_MAX_THREAD <= iIndex)
		return;

	if(TRUE == bNewThread) //새 쓰레드의 정보를 등록하는 상황
	{
	
		DWORD Value = InterlockedCompareExchange((LONG*)&m_iTopThreadPos, iTopThreadPos + 1, iTopThreadPos);

		if(Value != iTopThreadPos) // 실패했음 다음에 함수 훅이 일어날 때 다시 시도하자. 정말 확률이 0%이지만 시작하자마자 데드락이 걸린다면...
		{
			return;	
		}
	}

	m_pDeadLockThreadInfo[iIndex].dwThreadID = dwThreadID;
	m_pDeadLockThreadInfo[iIndex].dwTickCount = dwTickCount;
	m_pDeadLockThreadInfo[iIndex].btEnterCriticalSection = btEnterCritical;
}


BOOL ADLDeadLockInfo::IsDeadLockDetected(DWORD dwTickCount)
{
	INT iIndex = 0;
	BOOL bRet = FALSE;
	
	INT iTopThreadPos = m_iTopThreadPos;

	for(iIndex = 0; iIndex < iTopThreadPos; iIndex++)
	{
		if(m_pDeadLockThreadInfo[iIndex].dwThreadID == 0)
			continue;
		
//LeaveCriticalSection의 상황. 보통은 EnterCriticalSection의 상황에서 멈추니까 이 경우는 멈추지 않는 상황이라고 판단
//쓰레드 풀 상태에서는 당연히 btEnterCriticalSection = 0;
		if(m_pDeadLockThreadInfo[iIndex].btEnterCriticalSection == 0) 
			continue;

//락을 잡고 있고 그 이후로 함수 후킹이 일어나지 않았다는 것은 데드락이 일어났다는 증거..
//브레이크 포인트에 걸은후 풀면 당연히 시간차가 커지므로 데드락으로 오보할 수 있다.
//이 모듈을 활용한다면 되도록 디버깅을 하지 말 것
		if(dwTickCount - m_pDeadLockThreadInfo[iIndex].dwTickCount > ADL_DEADLOCK_TIMEOUT)
		{
			m_pDeadLockThreadInfo[iIndex].bIsDeadLockThread = TRUE;
			bRet = TRUE;
		}
	}

	return bRet;
}