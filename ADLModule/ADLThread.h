#pragma once
#include <process.h>

class ADLThread
{
public:
	ADLThread(void);
	virtual ~ADLThread(void);

	BOOL CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL, 
					  DWORD dwStackSize = 0, 
					  DWORD dwCreationFlags = 0, 
					  LPDWORD lpThreadID = NULL);

	void EndThread();

	HANDLE GetHandle(){return m_hThread;}

protected:
	static unsigned __stdcall __Run(void* pParameter);
	virtual unsigned Run() = 0;

	virtual BOOL OnStartUp();

private:
	HANDLE m_hThread;
};
