#pragma once


// CDlgVacBkPro dialog

class CDlgVacBkPro : public CDialog
{
	DECLARE_DYNAMIC(CDlgVacBkPro)

public:
	CDlgVacBkPro(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVacBkPro();

// Dialog Data
	enum { IDD = IDD_DLG_VACUUM_BG_PRO };

	CRect    m_rectPic;
	COLORREF m_crBk;
	COLORREF m_crGrid;
	COLORREF m_crScale;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	UINT m_uiXGridNumber;
public:
	int m_iYUpperExp;
public:
	int m_iYLowerExp;
public:
	afx_msg void OnBnClickedBtnBgColor();
public:
	afx_msg void OnBnClickedBtnGridColor();
public:
	afx_msg void OnBnClickedBtnScaleTextColor();
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnPaint();
public:
	afx_msg void OnBnClickedBtnDefault();
public:
	UINT m_ixMinutesPerGrid;
public:
	BOOL m_bShowXGrids;
};
