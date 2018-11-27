// DlgSetSendChar.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgSetSendChar.h"


// CDlgSetSendChar dialog

IMPLEMENT_DYNAMIC(CDlgSetSendChar, CDialog)

CDlgSetSendChar::CDlgSetSendChar(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetSendChar::IDD, pParent)
{
//	DDX_Text(pDX, IDC_STATIC_COMMENT, m_strComment);
//	DDX_Text(pDX, IDC_EDIT1, m_strSend);
}

CDlgSetSendChar::~CDlgSetSendChar()
{
}

void CDlgSetSendChar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}


BEGIN_MESSAGE_MAP(CDlgSetSendChar, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgSetSendChar::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgSetSendChar message handlers

void CDlgSetSendChar::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_strSend.Empty();

	TCHAR ptch[1024];
	ptch[0] = _T('\0');
	int iLength = GetDlgItemText(IDC_EDIT1,ptch,1024);
	m_strSend.Format(_T("%s"),ptch);

	if( BST_CHECKED == ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck() ) 
	{
		m_strSend += TEXT("\x0D");
	}
	OnOK();
}

BOOL CDlgSetSendChar::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	// TODO:  Add extra initialization here
	if(m_b0D0A)
	{
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(BST_CHECKED);
	}
	SetDlgItemText(IDC_EDIT1,m_strSend);
	if(!m_strComment.IsEmpty())
	{
		SetDlgItemText(IDC_STATIC_COMMENT,m_strComment);
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
