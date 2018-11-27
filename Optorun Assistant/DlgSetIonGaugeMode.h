#pragma once
#include "afxwin.h"


// CDlgSetIonGaugeMode dialog

class CDlgSetIonGaugeMode : public CDialog
{
	DECLARE_DYNAMIC(CDlgSetIonGaugeMode)

public:
	CDlgSetIonGaugeMode(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSetIonGaugeMode();

// Dialog Data
	enum { IDD = IDD_DLG_SET_IONGAUGE_MODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	TCHAR m_tszGroup[256]; //Group的标题,Dlg init之前初始化.
	int   m_nGaugeType;
public:
	virtual BOOL OnInitDialog();
public:
	int m_nGaugeRecorderMode;
};
