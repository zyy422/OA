#pragma once
#include "editex.hxx"


// CDlgVacErr dialog

class CDlgVacErr : public CDialog
{
	DECLARE_DYNAMIC(CDlgVacErr)

public:
	CDlgVacErr(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVacErr();

// Dialog Data
	enum { IDD = ID_DLG_VACUUM_THREAD_ERROR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEditEx m_editReason;
	CString m_strReason;
public:
	virtual BOOL OnInitDialog();
};
