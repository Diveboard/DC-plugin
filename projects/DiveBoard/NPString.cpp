//kk: helper to wrap some common string tasks

#include "stdafx.h"

#include "NPString.h"

CNPString::CNPString(NPString npString) : m_pszData(0), m_len(0)
{
	Set(  npString.UTF8Characters, npString.UTF8Length );
}

CNPString::CNPString(NPVariant npv) : m_pszData(0), m_len(0)
{
	if (npv.type == NPVariantType_String)
		Set( npv.value.stringValue.UTF8Characters, npv.value.stringValue.UTF8Length );
}


CNPString::CNPString(LPCTSTR psz, int len) : m_pszData(0), m_len(0)
{
	Set( psz, len );
}

CNPString::~CNPString()
{
	delete m_pszData;
}

void CNPString::Set(LPCTSTR psz, int len)
{
	delete m_pszData;

	m_len = len + 1;
	m_pszData = new TCHAR[m_len];

	//todo find why used
	//StrCpyN( m_pszData, psz, m_len );
}

CNPString::operator LPCTSTR()
{
	return *m_pszData;
}

int CNPString::Compare(LPCTSTR psz)
{
	if( m_pszData )
	{
		//todo find why usd
		//return StrCmp( m_pszData, psz );
	}

	return -1;
}