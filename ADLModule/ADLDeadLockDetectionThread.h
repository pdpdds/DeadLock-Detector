#pragma once
#include "ADLTimerThread.h"
#include "ADLStackWalker.h"
#include "ADLCenter.h"
#include "ADLDeadLockInfo.h"

class ADLDeadLockDetectionThread : public ADLTimerThread
{	

public:
	ADLDeadLockDetectionThread(DWORD dwTimerInterval);
	virtual ~ADLDeadLockDetectionThread(void);

	BOOL OnStartUp() override;
	void OnTimer() override;
	void SetADLSenter(ADLCenter* pCenter){m_pADLCenter = pCenter;}

	BOOL Initialize();
	BOOL ReportDeadLock(ADLDeadLockInfo* pDeadLockInfo);
	BOOL Report(DWORD dwThreadID);

	void SetProcessID(DWORD dwProcessID){m_dwProcessID = dwProcessID;}
	void SetProcessHandle(HANDLE hProcess){m_hProcess = hProcess;}

	DWORD GetProcessID(){return m_dwProcessID;}
	HANDLE GetProcessHandle(HANDLE hProcess){return m_hProcess;}

	void SetWaitEvent(HANDLE hEvent){m_hEvent = hEvent;}	

	BOOL AttachADLCenter(ADLCenter* pCenter);
	BOOL DetachAdLCenter();
	
	REPORT_MODE GetReportMode();

protected:
	BOOL ProcessReportCallStackByPDB(HANDLE hThread);
	BOOL ProcessReportCallStackByAddresss(HANDLE hThread);

private:	
	DWORD m_dwProcessID;
	HANDLE m_hProcess;
	HANDLE m_hEvent;
	DWORD m_dwTargetThreadID;

	ADLStackWalker m_StackWalker;
	ADLCenter* m_pADLCenter;	

	BOOL m_WasDeadLockReport;
};
