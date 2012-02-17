#pragma once

typedef struct tag_DeadLockThreadInfo
{
	DWORD dwThreadID;
	DWORD dwTickCount;
	BYTE btEnterCriticalSection;
	BOOL bIsDeadLockThread;

	tag_DeadLockThreadInfo()
	{
		dwThreadID = 0;
		dwTickCount = 0;
		btEnterCriticalSection = 0;
		bIsDeadLockThread = FALSE;
	}
}stDeadLockThreadInfo;

#define ADL_DEADLOCK_TIMEOUT 5000 //데드락 걸렸다고 판단되는 시간 5초동안 시간 업데이트가 없었으며 Lock 카운트가 하나라도 있으면 데드락이다.
#define ADL_MAX_THREAD 1000 //테스트에 쓰일 쓰레드 최대 개수. 그냥 크게 잡았음

class ADLDeadLockInfo
{
public:
	ADLDeadLockInfo(void);
	virtual ~ADLDeadLockInfo(void);

	static ADLDeadLockInfo* GetInstance()
	{
		if(NULL == m_pDeadLockInfo)
		{
			m_pDeadLockInfo = new ADLDeadLockInfo();
		}

		return m_pDeadLockInfo;
	}

	BOOL IsDeadLockDetected(DWORD dwTickCount);
	void SetInfomation(BYTE btEnterCritical);	

	stDeadLockThreadInfo* GetThreadInfo(){return m_pDeadLockThreadInfo;}
	INT GetThreadSize(){return m_iTopThreadPos;}

protected:

private:
	static ADLDeadLockInfo* m_pDeadLockInfo;
	stDeadLockThreadInfo m_pDeadLockThreadInfo[ADL_MAX_THREAD];
	INT m_iTopThreadPos;
};
