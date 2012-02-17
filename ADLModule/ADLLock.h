#pragma once

class ADLLock
{
public:
	ADLLock(void);
	virtual ~ADLLock(void);

	void Lock() {EnterCriticalSection(&m_CS);}
	void Unlock() {LeaveCriticalSection(&m_CS);}

protected:

private:
	CRITICAL_SECTION m_CS;
};

class ADLLockHelper
{
public:
	ADLLockHelper(ADLLock* pCS)
	{
		pCS->Lock();
		m_pCS = pCS;
	}

	virtual ~ADLLockHelper(void)
	{
		m_pCS->Unlock();
	}

protected:

private:
	ADLLock* m_pCS;
};