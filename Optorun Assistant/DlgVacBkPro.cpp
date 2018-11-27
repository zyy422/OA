// DlgVacBkPro.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgVacBkPro.h"

#include "OACnfgDoc.h"
extern CZYGraphPlot g_CZYGraphPlot;

// CDlgVacBkPro dialog

IMPLEMENT_DYNAMIC(CDlgVacBkPro, CDialog)

CDlgVacBkPro::CDlgVacBkPro(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVacBkPro::IDD, pParent)
	, m_uiXGridNumber(30)
	, m_iYUpperExp(5)
	, m_iYLowerExp(-6)
	, m_ixMinutesPerGrid(1)
	, m_bShowXGrids(true)
{

}

CDlgVacBkPro::~CDlgVacBkPro()
{
}

void CDlgVacBkPro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_X_RANGE, m_uiXGridNumber);
	DDV_MinMaxUInt(pDX, m_uiXGridNumber, 2, 864000);
	DDX_Text(pDX, IDC_EDIT_Y_UPPER_EXP, m_iYUpperExp);
	DDV_MinMaxInt(pDX, m_iYUpperExp, -10, 7);
	DDX_Text(pDX, IDC_EDIT_Y_LOWER_EXP, m_iYLowerExp);
	DDV_MinMaxInt(pDX, m_iYLowerExp, -10, 10);
	DDX_Text(pDX, IDC_EDIT_X_MINUTES_GRID, m_ixMinutesPerGrid);
	DDV_MinMaxUInt(pDX, m_ixMinutesPerGrid, 1, 1000000);
	DDX_Check(pDX, IDC_CHECK_SHOW_X_GRIDS, m_bShowXGrids);
}


BEGIN_MESSAGE_MAP(CDlgVacBkPro, CDialog)
	ON_BN_CLICKED(IDC_BTN_BG_COLOR, &CDlgVacBkPro::OnBnClickedBtnBgColor)
	ON_BN_CLICKED(IDC_BTN_GRID_COLOR, &CDlgVacBkPro::OnBnClickedBtnGridColor)
	ON_BN_CLICKED(IDC_BTN_SCALE_TEXT_COLOR, &CDlgVacBkPro::OnBnClickedBtnScaleTextColor)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_DEFAULT, &CDlgVacBkPro::OnBnClickedBtnDefault)
END_MESSAGE_MAP()


// CDlgVacBkPro message handlers

void CDlgVacBkPro::OnBnClickedBtnBgColor()
{
	CColorDialog dlg(m_crBk, CC_FULLOPEN);;
	if (dlg.DoModal() == IDOK)
	{
		m_crBk = dlg.GetColor();
		Invalidate();		
	}
}

void CDlgVacBkPro::OnBnClickedBtnGridColor()
{
	CColorDialog dlg(m_crGrid, CC_FULLOPEN);;
	if (dlg.DoModal() == IDOK)
	{
		m_crGrid = dlg.GetColor();
		Invalidate();		
	}
}

void CDlgVacBkPro::OnBnClickedBtnScaleTextColor()
{
	// TODO: Add your control notification handler code here
	CColorDialog dlg(m_crScale, CC_FULLOPEN);;
	if (dlg.DoModal() == IDOK)
	{
		m_crScale = dlg.GetColor();
		Invalidate();
		//TRACE("RGB value of the selected color - red = %u, green = %u, blue = %u\n",GetRValue(m_cr), GetGValue(m_cr), GetBValue(m_cr));
	}
}

BOOL CDlgVacBkPro::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	GetDlgItem(IDC_STATIC_PIC)->GetWindowRect(&m_rectPic);
	ScreenToClient(&m_rectPic);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVacBkPro::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	CDC* pdc = GetDC();
	if(pdc)
	{
		g_CZYGraphPlot.fn_PlotIllustrUse1st(pdc, m_rectPic,m_crBk,m_crGrid);
		ReleaseDC(pdc);
	}
}

void CDlgVacBkPro::OnBnClickedBtnDefault()
{
	// TODO: Add your control notification handler code here
	m_crBk = RGB(0,0,0);
	m_crGrid = RGB(0,128,64);
	m_crScale =RGB(64,196,64);
	m_uiXGridNumber = 30;
	m_bShowXGrids = true;
	m_ixMinutesPerGrid = 1;
	m_iYUpperExp = 5;
	m_iYLowerExp = -6;
	UpdateData(false);
	Invalidate();
}
