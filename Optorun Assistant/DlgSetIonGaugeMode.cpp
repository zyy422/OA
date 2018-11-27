// DlgSetIonGaugeMode.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgSetIonGaugeMode.h"


// CDlgSetIonGaugeMode dialog

IMPLEMENT_DYNAMIC(CDlgSetIonGaugeMode, CDialog)

CDlgSetIonGaugeMode::CDlgSetIonGaugeMode(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetIonGaugeMode::IDD, pParent)
	, m_nGaugeRecorderMode(0)
{
	_stprintf(m_tszGroup,TEXT("%s"),TEXT("Setup Ion Gauge Recorder Type"));
	

}

CDlgSetIonGaugeMode::~CDlgSetIonGaugeMode()
{
}

void CDlgSetIonGaugeMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_nGaugeRecorderMode);
	//DDX_Radio(pDX, IDC_RADIO1, m_nGaugeRecorderMod);
}


BEGIN_MESSAGE_MAP(CDlgSetIonGaugeMode, CDialog)
END_MESSAGE_MAP()


// CDlgSetIonGaugeMode message handlers

BOOL CDlgSetIonGaugeMode::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetDlgItemText(IDC_STATIC_SETUP_MODE,m_tszGroup);
	switch(m_nGaugeType)
	{
	case 1:   //_TYPE_ION_GAUGE_GIM2:
		SetDlgItemText(IDC_RADIO1,_T("each-digit Linear( LIN )"));
		SetDlgItemText(IDC_RADIO2,_T("pseudo-logarithmic( LOG )"));
		SetDlgItemText(IDC_RADIO3,_T("recorder hold( REC-HOLD )"));
		GetDlgItem(IDC_RADIO3)->EnableWindow(false);
		break;
	case 2:   //_TYPE_ION_GAUGE_M832
		SetDlgItemText(IDC_RADIO1,_T("combined D/A: LINEAR"));
		SetDlgItemText(IDC_RADIO2,_T("combined D/A: LOG"));
		GetDlgItem(IDC_RADIO3)->ShowWindow(SW_HIDE);
		//SetDlgItemText(IDC_RADIO3,_T(""));
		break;
	case 3:   //_TYPE_ION_GAUGE_BPG400
		SetDlgItemText(IDC_RADIO1,_T("Not used in BPG40"));
		GetDlgItem(IDC_RADIO2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO3)->ShowWindow(SW_HIDE);
		break;
	case 4:   //_TYPE_ION_GAUGE_GID7
		SetDlgItemText(IDC_RADIO1,_T("Not used in GID7"));
		GetDlgItem(IDC_RADIO2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO3)->ShowWindow(SW_HIDE);
		break;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
