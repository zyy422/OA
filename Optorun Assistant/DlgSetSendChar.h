#pragma once
#include "afxwin.h"


// CDlgSetSendChar dialog

class CDlgSetSendChar : public CDialog
{
	DECLARE_DYNAMIC(CDlgSetSendChar)

public:
	CDlgSetSendChar(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSetSendChar();

// Dialog Data
	enum { IDD = IDD_DIALIOG_SEND_CHAR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// 对本页面的解释
	CString m_strComment;
public:
	CString m_strSend;
	BOOL	m_b0D0A;
public:
	afx_msg void OnBnClickedOk();
public:
	virtual BOOL OnInitDialog();
};
