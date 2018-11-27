#pragma once


// Vcc500PropertyDlg dialog

class Vcc500PropertyDlg : public CDialog
{
	DECLARE_DYNAMIC(Vcc500PropertyDlg)

public:
	Vcc500PropertyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~Vcc500PropertyDlg();

public:
	CString m_strSend;
	bool    m_b0D0A; //进来时是否带 0D0A

// Dialog Data
	enum { IDD = IDD_DIALIOG_VCC500_SEND_CHAR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	virtual BOOL OnInitDialog();
};
