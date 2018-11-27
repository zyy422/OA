// OASysInfoView.h : interface of the COASysInfoView class
//
#include "EditEx.hxx"
#include "staticex.hxx"
#include "afxcmn.h"

#pragma once
#include "afxwin.h"


class COASysInfoView : public CFormView
{
public: // create from serialization only
	COASysInfoView();
	DECLARE_DYNCREATE(COASysInfoView)

public:
	enum{ IDD = IDD_OPTORUNASSISTANT_FORM };

// Attributes

public:
	COptorunAssistantDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~COASysInfoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg LRESULT OnDisplayPlcState(WPARAM wParam,  LPARAM lParam);
	afx_msg LRESULT OnAddRunHistroy(WPARAM wParam,  LPARAM lParam);
	afx_msg LRESULT OnGetNames(WPARAM wParam,  LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
//	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
public:
	// 设定页面文本，这些文本是从 xml文件中读取来的
	bool fn_SetIDsText(void);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	CEditEx m_editHistory;
	CEditEx m_editHelp;
	int	    m_uiHelpNumber; //当前应该显示的帮助序号.
	UINT    m_unSysOneSecondTimer;
public:
	void fn_AppendHistoryText(LPCTSTR lpText);
	
public:
	afx_msg void OnBnClickedButtonHelpLeft();
public:
	afx_msg void OnBnClickedButtonHelpRight();
//	afx_msg void OnChangeLanguage(UINT nID);
public:
	CStaticEx m_staticPLCstate;
public:
	void fn_DisplayPlcState(void);
public:
	void fn_ShowWindow(void);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
//	afx_msg void OnClose();
public:
	afx_msg void OnDestroy();
public:
	CStaticEx m_staticHelpTilte;
public:
	afx_msg void OnBnClickedButtonCloseConnection();
public:
	afx_msg void OnBnClickedButtonConnectPlc();
public:
	// int nBeginWord, int nReadNumber, 有 Event 检测.
	CString GetDMstrEvent(int nBeginWord, int nReadNumber);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	// 进入Debug state,改变一些程序运行的关键参数，不对普通用户开放
	void fn_InDebugState(void);
public:
//	afx_msg void OnBnClickedStatic();
public:
//	afx_msg void OnBnClickedStaticSerialPortTest();
public:
	afx_msg void OnBnClickedButtonCom2Open();
public:
	afx_msg void OnBnClickedButtonCom2Close();
public:
	afx_msg void OnBnClickedButtonCom2Test();
public:
	afx_msg void OnBnClickedButtonCom2Property();
public:
	afx_msg void OnBnClickedButtonCom3Open();
public:
	afx_msg void OnBnClickedButtonCom3Close();
public:
	afx_msg void OnBnClickedButtonCom3Test();
public:
	afx_msg void OnBnClickedButtonCom3Property();
public:
	afx_msg void OnBnClickedButtonCom4Open();
public:
	afx_msg void OnBnClickedButtonCom4Close();
public:
	afx_msg void OnBnClickedButtonCom4Test();
public:
	afx_msg void OnBnClickedButtonCom4Property();
public:
	afx_msg void OnBnClickedButtonCom5Open();
public:
	afx_msg void OnBnClickedButtonCom5Close();
public:
	afx_msg void OnBnClickedButtonCom5Test();
public:
	afx_msg void OnBnClickedButtonCom5Property();
public:
	// 显示COM2~COM5的相关界面（根据内部参数）
	int fn_ShowCom(void);
public:
//	afx_msg void OnBnDoubleclickedStaticSerialPortTest();
public:
//	afx_msg void OnBnClickedStaticSerialPortTest();
public:
	afx_msg void OnStnClickedStaticCom2Text();
public:
//	afx_msg void OnBnClickedStaticSerialPortTest();
public:
	afx_msg void OnStnClickedStaticCom3Text();
public:
	afx_msg void OnStnClickedStaticCom4Text();
public:
	afx_msg void OnStnClickedStaticCom5Text();
public:
	afx_msg void OnBnClickedCheckRecordVacuum();
public:
//	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	afx_msg void OnStnClickedStaticHelpTitle();
};

#ifndef _DEBUG  // debug version in OASysInfoView.cpp
inline COptorunAssistantDoc* COASysInfoView::GetDocument() const
   { return reinterpret_cast<COptorunAssistantDoc*>(m_pDocument); }
#endif

