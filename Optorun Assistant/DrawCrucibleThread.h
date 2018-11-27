#pragma once
#include "afxwin.h"



// CDrawCrucibleThread

class CDrawCrucibleThread : public CWinThread
{
	DECLARE_DYNCREATE(CDrawCrucibleThread)

protected:
	CDrawCrucibleThread();           // protected constructor used by dynamic creation
	virtual ~CDrawCrucibleThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	afx_msg void OnDrawCrucible(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	CBitmap m_LEDNumber;
};