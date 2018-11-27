#pragma once


// CDlgExhaustSetup dialog

class CDlgExhaustSetup : public CDialog
{
	DECLARE_DYNAMIC(CDlgExhaustSetup)

public:
	CDlgExhaustSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgExhaustSetup();

// Dialog Data
	enum { IDD = IDD_DLG_EXHAUST_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
