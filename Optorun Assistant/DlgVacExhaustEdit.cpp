// DlgVacExhaustEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgVacExhaustEdit.h"


// CDlgVacExhaustEdit dialog

IMPLEMENT_DYNAMIC(CDlgVacExhaustEdit, CDialog)

CDlgVacExhaustEdit::CDlgVacExhaustEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgVacExhaustEdit::IDD, pParent)
	, m_strEdit(_T(""))
	, m_point(0)
{

}

CDlgVacExhaustEdit::~CDlgVacExhaustEdit()
{
}

void CDlgVacExhaustEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strEdit);
}


BEGIN_MESSAGE_MAP(CDlgVacExhaustEdit, CDialog)
//	ON_EN_KILLFOCUS(IDC_EDIT1, &CDlgVacExhaustEdit::OnEnKillfocusEdit1)
ON_WM_LBUTTONDOWN()
ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


// CDlgVacExhaustEdit message handlers

//void CDlgVacExhaustEdit::OnEnKillfocusEdit1()
//{
//	// TODO: Add your control notification handler code here
//	OnOK();
//}

void CDlgVacExhaustEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	//CRect rect;
	//GetClientRect(&rect);
	//if(!rect.PtInRect(point))
	{
		OnOK();
		return;
	}

	CDialog::OnLButtonDown(nFlags, point);
}

BOOL CDlgVacExhaustEdit::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetCapture();
	CRect rect;
	GetWindowRect(&rect);
	MoveWindow(m_point.x,m_point.y,rect.Width(),rect.Height());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVacExhaustEdit::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	//CRect rect;
	//GetClientRect(&rect);
	//if(!rect.PtInRect(point))
	{
		OnCancel();
		return;
	}
	CDialog::OnRButtonDown(nFlags, point);
}
