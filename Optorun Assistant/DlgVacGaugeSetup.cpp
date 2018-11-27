// DlgVacGaugeSetup.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgVacGaugeSetup.h"
#include "OACnfgDoc.h"

// CDlgVacGaugeSetup dialog

IMPLEMENT_DYNAMIC(CDlgVacGaugeSetup, CDialog)

CDlgVacGaugeSetup::CDlgVacGaugeSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVacGaugeSetup::IDD, pParent)
	, m_iAPCGauge(0)
	, m_iIonGauge(0)
{

}

CDlgVacGaugeSetup::~CDlgVacGaugeSetup()
{
}

void CDlgVacGaugeSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_APC_GAUGE, m_iAPCGauge);
	DDX_CBIndex(pDX, IDC_COMBO_ION_GAUGE, m_iIonGauge);
}


BEGIN_MESSAGE_MAP(CDlgVacGaugeSetup, CDialog)
END_MESSAGE_MAP()


// CDlgVacGaugeSetup message handlers

BOOL CDlgVacGaugeSetup::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CComboBox* pcbx = (CComboBox*)GetDlgItem(IDC_COMBO_APC_GAUGE);
	pcbx->AddString(_T("0 = PSD && Penning"));
	pcbx->AddString(_T("1 = UNKNOWN"));
	pcbx->AddString(_T("2 = PEG100"));
	pcbx->AddString(_T("3 = MPG400"));
	pcbx->AddString(_T("4 = ULVAC BMR2/SC1"));

/*	0=Ÿo
1=GI-M2
2=IOZ_M832
3=IOZ_BPG400
4=GI-D7
5=BPG402
*/
	pcbx = (CComboBox*)GetDlgItem(IDC_COMBO_ION_GAUGE);
	pcbx->AddString(_T("0 = No Ion Gauge"));
	pcbx->AddString(_T("1 = GI-M2"));
	pcbx->AddString(_T("2 = IOZ_M832"));
	pcbx->AddString(_T("3 = IOZ_BPG400"));
	pcbx->AddString(_T("4 = GI-D7"));
	pcbx->AddString(_T("5 = BPG402"));


	m_iAPCGauge = g_APCGauge.GetGaugeType();
	m_iIonGauge = g_IonGauge.GetGaugeType();
	UpdateData(false);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
