#pragma once

#include <npfunctions.h>
#include <npruntime.h>

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

