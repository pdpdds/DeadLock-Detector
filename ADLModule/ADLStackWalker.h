#pragma once
#include "StackWalker.h"
#include "ADLCenter.h"

class ADLStackWalker : public StackWalker
{
public:
	ADLStackWalker(void);
	virtual ~ADLStackWalker(void);

	void OnOutput(LPCSTR szText) override;
	BOOL ReportCallStackByStackWalker(ADLCenter* pADLCenter, HANDLE hHandle);
	BOOL ReportCallStackByAddress(ADLCenter* pADLCenter, HANDLE hHandle);

protected:
	
private:
	ADLCenter* m_pADLCenter;
};
