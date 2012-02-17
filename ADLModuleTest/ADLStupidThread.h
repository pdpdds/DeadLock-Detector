#pragma once
#include "../ADLModule/ADLLock.h"
#include "../ADLModule/ADLThread.h"

class ADLStupidThread : public ADLThread
{
public:
	ADLStupidThread(ADLLock* pLock1, ADLLock* pLock2);
	virtual ~ADLStupidThread(void);

	VOID SetStopSignal(BOOL bStopSignal){m_bStopSignal = bStopSignal;}

	void Function1();
	void Function2();	

protected:
	virtual unsigned Run();

private:
	ADLLock* m_pLock1;
	ADLLock* m_pLock2;

	BOOL m_bStopSignal;
};
