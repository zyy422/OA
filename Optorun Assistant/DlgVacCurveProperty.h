#pragma once


// CDlgVacCurveProperty dialog

class CDlgVacCurveProperty : public CDialog
{
	DECLARE_DYNAMIC(CDlgVacCurveProperty)

public:
	CDlgVacCurveProperty(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVacCurveProperty();

// Dialog Data
	enum { IDD = IDD_DLG_VACUUM_CURVE_PRO };

public:
	//主要属性有，颜色，线型，名称
	COLORREF m_cr;
	int      m_nGraphStyle;
	int      m_nDotStyle;
	int      m_nPenStyle;
	CRect    m_rectPic;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnPaint();
public:
	int fn_SetComboBox(void);
public:
	afx_msg void OnCbnSelchangeCombo1();
public:
	afx_msg void OnCbnSelchangeCombo2();
public:
	CString m_strName;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	afx_msg void OnBnClickedBtnColour();
};
