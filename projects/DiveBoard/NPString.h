#pragma once

#include "stdafx.h"
//Mozilla-API
#include <npfunctions.h>
#include <npruntime.h>

class CNPString
{
public:
	CNPString(LPCTSTR psz, int len);
	CNPString(NPVariant npv);
	CNPString(NPString npString);
	~CNPString();

	void Set(LPCTSTR psz, int len);
	operator LPCTSTR ();
	int Compare(LPCTSTR psz);

protected:
	TCHAR *m_pszData;
	int m_len;
};

