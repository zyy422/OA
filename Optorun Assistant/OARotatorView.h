#pragma once
#include "afxwin.h"
#include "EditEx.hxx"
#include "staticex.hxx"
#include "afxcmn.h"

//#include "ZzyWaitingThread.h" //2007.12.28

#define _OPTORUN_HEARTH_MAX 2

// COARotatorView form view
	struct _stOAHearth
	{
		int iHearthType; //坩埚种类，   　　　　　//一次性初始化
		
		int iHearthMonitorType; //Hearth Monitor 厂商类别//2008.07.15，对于 OMRON，环形坩埚的话，则转速表达为RPH 0.5~30的限制.
		// hearth1 [DM9089]==2，则为OMRON. RPH 
		// hearth2 [DM9091]==2，则为OMRON. RPH    //一次性初始化
		
		int iCrucibleMaxNumber; //点坩埚最大坩埚号　//一次性初始化, 无需

		int iCoderVal; //编码器 当前值　　//监控数据值
		union{
		int iCurCrucible; //当前　坩埚号   //监控数据结果，一直更改中. 只在主线程中改写
		int iSpeed;
		};
	};

class COARotatorView : public CFormView
{
	DECLARE_DYNCREATE(COARotatorView)

public:
	COARotatorView();           // protected constructor used by dynamic creation
	virtual ~COARotatorView();

public:
	enum { IDD = IDD_OA_ROTATOR_FORM };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
public:
	virtual void OnInitialUpdate();
public:
//	afx_msg void OnBnClickedBtnInitCrucible();
public:
	// 仅供OnInitialDialog中调用一次，初始化 RotatorView界面
	void fn_InitView(void);
public:
	CListCtrl m_ListctrlCrucible;
	bool m_bInit;

struct _stOAHearth m_OAHearth[_OPTORUN_HEARTH_MAX];

	//CZzyWaitingThread* m_pAnnularHearthWaitingThread; //Init成功后启动
	

public:
	void fn_ShowWindow(void);
	void fn_ShowDomeWindow(void);//更新显示与Dome相关的界面
public:
	afx_msg void OnBnClickedBtnInitHearth();
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
	afx_msg void OnBnClickedButtonTest();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	// 仅在初始化Hearth成功后，调用一次。用于显示Hearth的初始状态，比如，是否显示，以及什么内容.
	void fn_AfterInitHearth(void);
public:
	afx_msg void OnBnClickedBtnInsertHearth1();
public:
	afx_msg void OnBnClickedBtnInsertHearth2();
public:
	afx_msg void OnBnClickedBtnDeleteHearthRow();
public:
	afx_msg void OnBnClickedBtnSaveHearthList();
public:
	afx_msg void OnBnClickedBtnStartHearthTest();
public:
	afx_msg void OnBnClickedBtnCancelHearthTest();
public:
	void fn_OnHearthMonitorThread(WPARAM wParam, LPARAM lParam);
	void fn_OnDomeMonitorThread(WPARAM wParam, LPARAM lParam);
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	bool fn_StartHearthTest(int iItem);
	bool fn_StartDomeTest(int iItem);
public:
	afx_msg void OnBnClickedBtnCreateHearth1();
public:
	afx_msg void OnBnClickedBtnCreateHearth2();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	void fn_CreateHearthTestOrder(int iHearth, int iStartPos);
public:
	CStaticEx m_staticTestHearthTime;
public:
	void fn_SetIDsText(void);
public:
	int fn_SaveHearthList(LPCTSTR pszFileName);
	int fn_SaveDomeList(LPCTSTR pszFileName);
public:
	// 自动生成文件名，并保存当前Hearth列表内容，供测量停止时自动保存
	int fn_AutoSaveHearthList(void);
	int fn_AutoSaveDomeList(void);
	bool fn_SaveRestoreDomeConfigure(bool bSave);
	
	bool fn_Is_x_near( double* x,int n,double d_epsilon);
	int fn_FigureDomeThreadData( double& dAveSecPR, double& dFiguredSecPR);
	int	fn_CalcDomeRotationData(double& dT0, int& n1, int& n2,int& n3,int& n4, double dStandValue);

public:
	afx_msg void OnBnClickedBtnStatisticsEvaluation();
public:
	CListCtrl m_ListctrlDome;
public:
	afx_msg void OnBnClickedBtnAutocreateDomeSpeed();
public:
	afx_msg void OnBnClickedBtnInsertDomeSpeed();
public:
	afx_msg void OnBnClickedBtnDeleteDomeRow();
public:
	afx_msg void OnBnClickedBtnSaveDomeList();
public:
	afx_msg void OnBnClickedBtnStartDomeTest();
public:
	afx_msg void OnBnClickedBtnCancelDomeTest();
public:
	CStaticEx m_staticexDomeThreadState;
};