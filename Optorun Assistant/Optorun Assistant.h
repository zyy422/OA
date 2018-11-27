// Optorun Assistant.h : main header file for the Optorun Assistant application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// COptorunAssistantApp:
// See Optorun Assistant.cpp for the implementation of this class
//

class COptorunAssistantApp : public CWinApp
{
public:
	COptorunAssistantApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	afx_msg LRESULT OnMainFrmTest(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

extern COptorunAssistantApp theApp;
