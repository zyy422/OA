#pragma once
#include "atltypes.h"


// CDlgVacExhaustEdit dialog

class CDlgVacExhaustEdit : public CDialog
{
	DECLARE_DYNAMIC(CDlgVacExhaustEdit)

public:
	CDlgVacExhaustEdit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVacExhaustEdit();

// Dialog Data
	enum { IDD = IDD_DLG_VACUUM_EXHAUST_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strEdit;
public:
//	afx_msg void OnEnKillfocusEdit1();
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
public:
	CPoint m_point;
};
