
// BEOPOPCClient.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CBEOPOPCClientApp:
// See BEOPOPCClient.cpp for the implementation of this class
//

class CBEOPOPCClientApp : public CWinApp
{
public:
	CBEOPOPCClientApp();

// Overrides
public:
	virtual BOOL InitInstance();
	BOOL FindProcessByName_Myexe(CString exefile);		//�������Ƿ������У�������
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CBEOPOPCClientApp theApp;