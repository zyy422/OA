#pragma once
#include "afxwin.h"
#include "staticex.hxx"
#include "afxcmn.h"


extern bool g_bVacuumSaved;

// COAVacuumView form view
typedef struct tagChV
{
	CString str;
	double  d;
}stChV;


class COAVacuumView : public CFormView
{
	DECLARE_DYNCREATE(COAVacuumView)

public:
	COAVacuumView();           // protected constructor used by dynamic creation
	virtual ~COAVacuumView();

public:
	enum { IDD = IDD_OA_VACUUM_FORM };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	stChV pORChamberVolume[16];//10->15,2008.07.14,参照<设计情报文书 DIM-0004>
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg LRESULT OnDisplayVacState(WPARAM wParam,  LPARAM lParam);
	afx_msg LRESULT OnDisplayVacValue(WPARAM wParam,  LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
public:
//	afx_msg void OnBnClickedButtonTest2();
public:
//	afx_msg void OnBnClickedButtonTest1();
public:
	virtual void OnInitialUpdate();
public:
	CStaticEx m_staticVACState;
public:
	void fn_DisplayVacState(void);
	void fn_DisplayVacValue(void);
public:
	void fn_SetIDsText(void);
public:
	afx_msg void OnBnClickedBtnStartVacuumTest();
public:
	afx_msg void OnBnClickedBtnCancelVacuumTest();
public:
	void fn_CancelTest(void);
public:
	afx_msg void OnBnClickedBtnIongaugeProperty();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
//	afx_msg void OnBnClickedCheckRecordVacuum();
public:
//	afx_msg void OnBnClickedCheckLeakRate();
public:
//	afx_msg void OnBnClickedCheckExhaustRate();
public:
	afx_msg void OnCbnSelchangeComboMachineType();
public:
	afx_msg void OnBnClickedRadioLeakRate();
public:
	afx_msg void OnBnClickedRadioExhaust();
public:
	afx_msg void OnBnClickedRadioRecorder();
public:
	void fn_OnItem2VacuumThread(WPARAM wParam, LPARAM lParam);
public:
	// 保存真空度页面内列表中的内容
	bool fn_SaveVacuumFile(void* pPara); //stVacuumTestEx* pstVacuumTestEx
public:
	void fn_ShowWindow(void);
public:
//	afx_msg void OnBnClickedStaticLeakRateGroup();
public:
	CListCtrl m_Listctrl;
	CRect     m_rtVacPic;
public:
//	afx_msg void OnSize(UINT nType, int cx, int cy);
public:

	CToolTipCtrl m_ToolTips;
	// 测量抽速时使用的ListCtrl,与测量漏率（定时）ListCtrl交替使用
	CListCtrl m_ListctrlExhaust;
	// 初始化显示设置，仅供 InitDialog调用
	void fn_InitView(void);
public:
	CStaticEx m_seElapsedTime;
public:
	afx_msg void OnBnClickedButtonExhaustSt();
public:
	bool fn_CheckStartCondition(void* pstVTE);
public:
//	afx_msg void OnHdnItemdblclickListVacuumContent(NMHDR *pNMHDR, LRESULT *pResult);
public:
//	afx_msg void OnNMDblclkListVacuumContent(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkListVacuumContentExhaust(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMRdblclkListVacuumContentExhaust(NMHDR *pNMHDR, LRESULT *pResult);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	afx_msg void OnBnClickedCheckUsingIongauge();
public:
	afx_msg void OnBnClickedButtonClearListctrl();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
	void fn_AppendHelpText(LPCTSTR lpText);
public:
	afx_msg void OnBnClickedButtonSaveListctrl();
public:
	// 将真空数据保存进文件,2007.12.12
	bool fn_SaveVacFile(LPCTSTR sFilename, int  iVacType);
public:
	afx_msg void OnBnClickedShiftRight();
public:
	afx_msg void OnBnClickedShiftLeft();
public:
	afx_msg void OnBnClickedBtnAddVacuumFile();
public:
	afx_msg void OnBnClickedShiftLeft2();
public:
	afx_msg void OnBnClickedShiftRight2();
public:
	// 移动图形
	int fn_MoveCurve(void* pZYGraph,double dMoveValue);
public:
	afx_msg void OnCbnSelchangeComboCurves();
public:
	CComboBox m_cmbCurves;
public:
	// 读取真空度测试文件，正常读取到 真空度-时间 数据则添加进 数据列表  去，否则返回错误
	int fn_LoadVacuumPressureFile(LPCTSTR szFileName);
public:
	// 提取形如 00:10:22 字符串中的时间秒数。pzTime为字符串，遇到'\t'或'\0'结束。
	double fn_ExtractTime(LPCTSTR pzTime);
public:
	// 根据存储的图形内容，重新初始化Combobox的内容
	void fn_ReInitGraphCombo(void);
public:
	afx_msg void OnBnClickedBtnDeleteCurve();
public:
	// 测试的真空度数据是否已经添加进 绘图用list中，初始化为false。响应首页发送来的连接plc消息后进行添加list动作，置true.
	bool m_bTestDataInList;
public:
	afx_msg void OnBnClickedBtnCurveProperty();
public:
	afx_msg void OnStnClickedStaticPenningValueName();
public:
	afx_msg void OnStnDblclickStaticPenningValueName();
public:
	afx_msg void OnBnClickedBtnGaugeSetup();
public:
	afx_msg void OnBnClickedButtonTest();
	// 向栏目中添加Text,用于Debug
	void fn_AddHelpText(LPCTSTR lpText,bool bAdd);
	afx_msg void OnEnChangeEditComment();
	afx_msg void OnEnChangeEditVacuumHelp();
	afx_msg void OnBnClickedButtonSaveListctrl2();
};


