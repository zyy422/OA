// MainFrm.h : interface of the CMainFrame class
//


#pragma once
#include "../TabCtrlSource/TabSDIFrameWnd.h"

#define WM_INIT_OTHER_FORM (WM_USER + 401)
#define WM_MENU_SELECT_LNG_CMD    (WM_USER + 410)  //预留100个空间，下一个必须定义成 511


class CMainFrame : public CTabSDIFrameWnd
//class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

public:
	int			m_nTabHeight;
	int         m_nTabFontHeight;

	CMenu       m_menuLng;

	UINT        m_unOneSecondTimer;
private:
	CString     str; //仅供临时使用

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
//	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTabSelChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnInitOtherForm(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnItemXtoItemY(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMainFrmTest(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMainFrmVacuumThreadErrOccur(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnItem2DisplayVacValue(WPARAM wParam, LPARAM lParam);	
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnChangeLanguage(UINT nID);
public:
	BOOL fn_InitLngPopMenu(void);
public:
	BOOL fn_UpdateMainMenu(CStringList &psl,int nID_From);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnClose();
public:
	void fn_FullScreen(void);
};


