#pragma once
#include "ADLThread.h"

class ADLTimerThread : public ADLThread
{
public:
	ADLTimerThread(DWORD dwTick = 0);
	virtual ~ADLTimerThread(void);

	void SetTimerInterval(DWORD dwTick){m_dwInterval = dwTick;}
	DWORD GetTimerInterval(){return m_dwInterval;}

	void BeginTimer();
	void EndTimer();

protected:
	virtual unsigned Run();
	virtual void OnTimer() = 0;

private:
	HANDLE m_hWaitEvent;
	DWORD m_dwInterval;
};
