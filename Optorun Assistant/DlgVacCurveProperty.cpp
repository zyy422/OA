// DlgVacCurveProperty.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgVacCurveProperty.h"
#include "OACnfgDoc.h"

extern CZYGraphPlot g_CZYGraphPlot;

// CDlgVacCurveProperty dialog

IMPLEMENT_DYNAMIC(CDlgVacCurveProperty, CDialog)

CDlgVacCurveProperty::CDlgVacCurveProperty(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVacCurveProperty::IDD, pParent)
	, m_strName(_T(""))
{
	m_nGraphStyle = 0;
	m_nDotStyle = 0;
}

CDlgVacCurveProperty::~CDlgVacCurveProperty()
{
}

void CDlgVacCurveProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strName);
}


BEGIN_MESSAGE_MAP(CDlgVacCurveProperty, CDialog)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgVacCurveProperty::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CDlgVacCurveProperty::OnCbnSelchangeCombo2)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_COLOUR, &CDlgVacCurveProperty::OnBnClickedBtnColour)
END_MESSAGE_MAP()


// CDlgVacCurveProperty message handlers

BOOL CDlgVacCurveProperty::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	GetDlgItem(IDC_STATIC_PIC)->GetWindowRect(&m_rectPic);
	ScreenToClient(&m_rectPic);

	fn_SetComboBox();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVacCurveProperty::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	CDC* pdc = GetDC();
	if(pdc)
	{
		g_CZYGraphPlot.fn_PlotIllustration(pdc,m_rectPic,m_cr,m_nGraphStyle,m_nDotStyle,m_nPenStyle);
		ReleaseDC(pdc);
	}
}

int CDlgVacCurveProperty::fn_SetComboBox(void)
{
	CComboBox* p = (CComboBox*)GetDlgItem(IDC_COMBO1);
	p->SetCurSel(m_nGraphStyle);
	p = (CComboBox*)GetDlgItem(IDC_COMBO2);
	p->SetCurSel(m_nDotStyle);
	p->EnableWindow(1==m_nGraphStyle?false:true);
	return 0;
}

//pen style
void CDlgVacCurveProperty::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	CComboBox* p = (CComboBox*)GetDlgItem(IDC_COMBO1);
	m_nGraphStyle = p->GetCurSel();
	p = (CComboBox*)GetDlgItem(IDC_COMBO2);
	p->EnableWindow(1==m_nGraphStyle?false:true);
	InvalidateRect(m_rectPic,false);
}
//dot style
void CDlgVacCurveProperty::OnCbnSelchangeCombo2()
{
	// TODO: Add your control notification handler code here
	CComboBox* p = (CComboBox*)GetDlgItem(IDC_COMBO2);
	m_nDotStyle = p->GetCurSel();
	InvalidateRect(m_rectPic,false);
}

HBRUSH CDlgVacCurveProperty::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	//if(pWnd->m_hWnd == GetDlgItem(IDC_BTN_COLOUR)->GetSafeHwnd())
	if(pWnd->GetDlgCtrlID() == IDC_STATIC_LINE_TEXT)
	{//对按钮不起作用，对静态文本有作用，此处取消
		// pDC->SetTextColor(m_cr);
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CDlgVacCurveProperty::OnBnClickedBtnColour()
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg(m_cr, CC_FULLOPEN);;
		//dlg.SetCurrentColor(m_cr);
	if (dlg.DoModal() == IDOK)
	{
		m_cr = dlg.GetColor();
		Invalidate();
		//TRACE("RGB value of the selected color - red = %u, green = %u, blue = %u\n",GetRValue(m_cr), GetGValue(m_cr), GetBValue(m_cr));
	}
}
