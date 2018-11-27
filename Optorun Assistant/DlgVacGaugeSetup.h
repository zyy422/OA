#pragma once


// CDlgVacGaugeSetup dialog

class CDlgVacGaugeSetup : public CDialog
{
	DECLARE_DYNAMIC(CDlgVacGaugeSetup)

public:
	CDlgVacGaugeSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVacGaugeSetup();

// Dialog Data
	enum { IDD = IDD_DLG_VACUUM_ABNOMAL_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_iAPCGauge;
public:
	int m_iIonGauge;
public:
	virtual BOOL OnInitDialog();
};
