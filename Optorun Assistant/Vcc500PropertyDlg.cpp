// Vcc500PropertyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "Vcc500PropertyDlg.h"


// Vcc500PropertyDlg dialog

IMPLEMENT_DYNAMIC(Vcc500PropertyDlg, CDialog)

Vcc500PropertyDlg::Vcc500PropertyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Vcc500PropertyDlg::IDD, pParent)
{

}

Vcc500PropertyDlg::~Vcc500PropertyDlg()
{
}

void Vcc500PropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Vcc500PropertyDlg, CDialog)
	ON_BN_CLICKED(IDOK, &Vcc500PropertyDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// Vcc500PropertyDlg message handlers

void Vcc500PropertyDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	m_strSend.Empty();

	TCHAR ptch[1024];
	ptch[0] = _T('\0');
	int iLength = GetDlgItemText(IDC_EDIT1,ptch,1024);
	m_strSend.Format(_T("%s"),ptch);

	if( BST_CHECKED == ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck() ) 
	{
		m_strSend += TEXT("\x0D\x0A");
	}


	OnOK();
}

BOOL Vcc500PropertyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if(m_b0D0A)
	{
		((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(BST_CHECKED);
	}
	SetDlgItemText(IDC_EDIT1,m_strSend);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
