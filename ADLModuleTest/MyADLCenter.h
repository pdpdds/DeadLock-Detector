#pragma once
#include "../ADLModule/ADLCenter.h"

class MyADLCenter : public ADLCenter
{
public:
	MyADLCenter(void);
	virtual ~MyADLCenter(void);

	virtual void OnOutput(LPCSTR szText) override;
};
