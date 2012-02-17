#include "StdAfx.h"
#include "MyADLCenter.h"
#include "stdio.h"

MyADLCenter::MyADLCenter(void)
{
}

MyADLCenter::~MyADLCenter(void)
{
}

void MyADLCenter::OnOutput( LPCSTR szText )
{
	printf("%s", szText);	
}