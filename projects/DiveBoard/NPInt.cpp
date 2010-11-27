#include "NPInt.h"
#include "NPString.h"

#ifdef TOTOOTT

CNPInt::CNPInt(void)
{
	value=0;
}
CNPInt::CNPInt(int i)
{
	value = i;
}

CNPInt::CNPInt(NPVariant npv)
{
	if( npv.type == NPVariantType_Int32 )
	{
		value = npv.value.intValue;
	}
	else if( npv.type == NPVariantType_String )
	{
		CNPString s = npv;
		value = atoi( s ); //todo : handle conversion errors
	}
	else value=-1;//TODO : an error happenend
}

CNPInt::~CNPInt()
{
}

CNPInt::operator int()
{
	return(value);
}

#endif