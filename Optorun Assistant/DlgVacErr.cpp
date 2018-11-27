// DlgVacErr.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgVacErr.h"
#include "OACnfgDoc.h"

// CDlgVacErr dialog

IMPLEMENT_DYNAMIC(CDlgVacErr, CDialog)

CDlgVacErr::CDlgVacErr(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVacErr::IDD, pParent)
{

}

CDlgVacErr::~CDlgVacErr()
{
}

void CDlgVacErr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_VACUUM_ERROR, m_editReason);
}


BEGIN_MESSAGE_MAP(CDlgVacErr, CDialog)
END_MESSAGE_MAP()


// CDlgVacErr message handlers

BOOL CDlgVacErr::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_editReason.textColor( ::gc_clrAlert);
	m_editReason.bkColor(::gc_clrWhite);
	m_editReason.SetWindowText(m_strReason);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
