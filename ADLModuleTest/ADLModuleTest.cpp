// ADLModuleTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MyADLCenter.h"
#include "ADLStupidThread.h"

ADLLock Lock1;
ADLLock Lock2;

ADLStupidThread Thread1(&Lock1, &Lock2);
ADLStupidThread Thread2(&Lock1, &Lock2);

void LocalFunction(ADLLock& Lock1, ADLLock& Lock2)
{	
	printf("push any button for testing dead-lock\n");
	getchar();		

	Thread1.CreateThread();
	Thread2.CreateThread();	

	printf("Analyzing Thread....\n");

	printf("press button for stopping Analysis....\n");

	getchar();
	
	Thread1.SetStopSignal(TRUE);
	Thread2.SetStopSignal(TRUE);	
}

int _tmain(int argc, _TCHAR* argv[])
{		
	MyADLCenter ADLInstance;

//PDB 정보를 이용하여 콜스택을 보여준다.
	ADLInstance.SetReportMode(ADL_STACKREPORT_PDB);

	ADLInstance.Begin();

	LocalFunction(Lock1, Lock2);	

	ADLInstance.End();	

	printf("Analyzing Complete\n");	

	printf("press any key.\n");	
	getchar();

	ExitProcess(0);

	return 0;
}

