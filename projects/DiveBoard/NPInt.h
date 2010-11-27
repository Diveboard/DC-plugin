#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <npfunctions.h>
#include <npruntime.h>


#ifdef TOTOOTOT
class CNPInt
{
public:
	CNPInt(void);
	CNPInt(int i);
	CNPInt(NPVariant npv);
	~CNPInt();

	operator int();

protected:
	int value;
};

#endif