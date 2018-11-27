// OARotatorView.cpp : implementation file
//2008.07.12 环形坩埚默认Hearth1逆时针，Hearth2顺时针.添加选择. S589是本软件遇到的第一台双环形坩埚
//Hearth2为环形埚，开始测试会出现错误："在规定时间内未能进入手动排气模式."
// CW, 编码器值减少，坩埚从上看为逆时针！ CW的理解上可能是从下往上看，与从上看是反的！
// 添加环形坩埚慢速超过１０分钟，最少２０度测试 2008.04.02
// 环形坩埚 FIX 模式下，CW/CCW不能转动。process中PITCH起作用

/*
Dome转动测试，t为单圈转动时间(seconds)， n为每分钟转动圈数。
n = 60 / t;
要求 n = 50时，精度满足 +/-0.1. 

故有 dn/n = -dt/t  . 不同转速下,对单圈时间精度要求不同. 
即 abs(dt/t) = abs(dn/n) < 0.1 / 50 = 0.002.
单圈精度dt = t * dn/n = (60/n) * 0 = 

若偏差 dt = 1 seconds, 测试总时间 t > 1/0.002 = 500 seconds
n=50, 圈数 = 500/(60/50) = 417 圈.
n=40, 圈数 = 500/(60/40) = 334 
n=30, 圈数 = 500/(60/30) = 250
n=20, 圈数 = 500/(60/20) = 167
n=10, 圈数 = 500/(60/10) = 84
*/

/*
PLC DA->Dome控制器->马达
计算算法更改如下:
1, 扣除前30圈数据
2, 再记录　speed*10　圈，比如: speed = 50，则记录　500圈
3, 实际总记录圈数　=  30 + speed*10.
4, 数据分析:
   a, 基本预测时间 t0 = 60/speed.
   b, 对记录圈数的时间进行排序, 找出落在 t0 ,2*t0, 3*t0, 4*t0一定范围内的数据个数(n1,n2,n3,n4).(暂时未发现 4倍t0,也就不考虑 5倍t0. ) 
   c, 平均每圈时间 = 总时间 / ( n1 + 2*n2 + 3*n3 + 4*n4 )
   d, 输出10圈时间 = xx.xx秒, RPM
*/
// Edit by: 张子业
#include "stdafx.h"
#include "Optorun Assistant.h"
#include "OARotatorView.h"
#include "OACnfgDoc.h"
#include "DrawCrucibleThread.h"
#include "RotatorThread.h"
#include "DomeRotateThread.h"
#include <math.h>
#include <stdlib.h>


#define _OA_HEARTH_TYPE_NONE    0 //没有坩埚
#define _OA_HEARTH_TYPE_ANNULAR    1 //环形坩埚
#define _OA_HEARTH_TYPE_CRUCIBLE   2 //点坩埚

#define _HEARTH_MONITOR_TYPE_OMRON 2

#define _ROTATOR_CRUCIBLE_LIST_COL_NO     0
#define _ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE 1
//#define _ROTATOR_CRUCIBLE_LIST_COL_RESUTL 2
#define _ROTATOR_CRUCIBLE_LIST_COL_TARGET   1
#define _ROTATOR_CRUCIBLE_LIST_COL_START 2
#define _ROTATOR_CRUCIBLE_LIST_COL_END   3
#define _ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME 4
#define _ROTATOR_CRUCIBLE_LIST_COL_RESULT 5

#define _ROTATOR_DOME_LIST_COL_SPEED		0
#define _ROTATOR_DOME_LIST_COL_START_CNT	1
#define _ROTATOR_DOME_LIST_COL_END_CNT		2
#define _ROTATOR_DOME_LIST_COL_WAITING_TIME	3
#define _ROTATOR_DOME_LIST_COL_TEST_TIME	4
#define _ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION	5
#define _ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE	6


#define _ROTATE_CRUCIBLE_STOP_JUDGE_TIME  5000 //可判断点坩埚的确属于停止状态，需要的最短时间。判断条件是编码器值不变


#define _ROTATE_DOME_NOT_COUNT	30//不加入Dome总计算的前n圈

struct _stReadDomeThreadData
{
	WORD	wDomeRotationCount;
	DWORD	dwTickTime;
};
#include <vector>
std::vector<_stReadDomeThreadData> g_stReadDomeThreadData; //记录Dome转动数据
//vector 空间不够时，自动递增；递增规律是 新空间大小 = 原空间大小*1.5

// COARotatorView
//环形坩埚，启动 转动 10秒后开始监测  180度旋转角度.
struct _stHearthRotationState
{
	int iCurListItem;
	
	int iTargetCoder;  //点坩埚，在启动之前确定
	                   //环形坩埚，在启动并且接收到数据之后开始计时， 等待10秒钟后（用户可高级定制）定出 下一个
	           
	union{
	int iTargetSpeed;
	int iTargetPos;    //对环形坩埚，是目标转动速度。而对点坩埚则是目标坩埚号
	};

	union{
	int iStartPos;     //启动时的位置.仅对点坩埚有效
	int iStartCoder;   //环形坩埚时的实际计算起始编码器值
	};

	int iCurHearth;    //当前坩埚号
	int iCurCoder;
	int iCurCrucible;  //
	
	DWORD dwStartTime; //启动时的 tickCount;
    int iRotationTime; //当前已经转动的时间, ms单位. 命令发出(启动)时清 0.
	
	int iLastCoder;    //上次转动的 编码器值. 启动时检查一次. 
	int iLastCrucible; //上次转达的 坩埚号.   启动时检查一次. //08.01.03
	
	bool bStop;        //启动时为 false.
	DWORD dwStopTime;  //ms, 当前没有判断到运动的时刻. 启动时清 0. 
	                   //停止转动的条件 1: Q10.11, Q10.12都为零， 同时 iCurCoder == iLastCoder
	                   //判断到不在运转时 bstop = true, 记录此时的 tickCount. 以后计时。在运转， 则清除开始标志。
	
	bool bReversal;    //是否逆转，  点坩埚启动时判断. 


	//环形坩埚
	bool bAnnularIntervalWaiting;  //转动完成停止后，到下一次转动开始前(停止状态)
	bool bAnnularBeginingWaiting;  //刚开始转动等待，二者不可能同时为真(转动状态)
	DWORD dwStartingWaitingTick;   //


	//环形坩埚的 辅助参数。由用户设置，需初始化
	int  iAnnularStartWaitTime;    //启动后开始查询的等待时间
	int  iAnnularIntervalWaitTime; //启动中间等待的时间
	bool bAnnularPositive;         //环形坩埚旋转的方向, true对应顺时针CW
	int  iAnnularTestAngle;        //测试角度 ( 30 ~ 360 ), 取编码器读数即可.

	//08.01.03
	int  iCoderChangeCounts;       //记录编码器变化次数， 变大则 +1，变小则 -1，不变则保持.
	                               //这样，程序可以据此知道真实的 编码器变化方向(仅正负号有用)
	                               //每次初始化时清0， 08.01.03添加，对应有的机台不清楚 正反转
	
	int  iCoderNotChangeTime;      //开始后，没有检查到 Coder 变化的时间长度. 如果Coder发生变化，则立即清0.开始时清0.
	DWORD dwCoderNotChangeStartTickTime; //没有检查到 Coder 变化的起始时间. //开始时记录当前的TciTime.每次Coder发生变化则更新
	int  iCrucibleChangeCounts;    //转动中记录到的坩埚变化数目. //因为可能存在反转，所以结果可能与实际转动坩埚数不同，但最多相差1.
	                               //开始后清0.
	enum {_enHearthReversalUnknown,_enHearthReversalTrue,_enHearthReversalFalse}enHaveReversalFunction;     //程序启动后，初始化为 false. 转达测量中，添加判断.

}g_stHearthRotationState; //仅在本界面线程内使用，供判断 转动 坩埚的流程使用.

#define _ROTATE_DOME_ABNOMAL_TIMES 4 //最大异常间隔倍数. 最多这么多圈才检测到一次转动
struct _stDomeRotationState
{
	int iCurListItem;  //当前检测的Item
	int iTargetSpeed;  //目标速度
	DWORD dwStartTime; //启动时的 tickCount;
    int iRotationTime; //当前已经转动的时间, ms单位. 命令发出(启动)时清 0.
	bool bStop;        //启动时为 false.

	
	DWORD dwTotalCountTime;
	DWORD dwLimit[_ROTATE_DOME_ABNOMAL_TIMES][2]; // [0]下限值,[1]上限制.在每次测量开始阶段初始化一次
	int	  n[_ROTATE_DOME_ABNOMAL_TIMES];	//倍数时间对应的有效个数.[0]对应1倍.　每次测量开始时清零一次.有效测量中途进行累积
	DWORD dwStandardValue;
	DWORD dwEpsilon;// = 500; //ms, 偏差值, 每次转动间隔　应分布在 标准时间的n倍　+/- 偏差值范围之内. 估计值
	int   nAbnomalCount;      //不在范围内的数据计数

	//以下两个时间供分析函数作参数使用。
	double dAveSecPR;	//平均每圈转动所需要的时间.单位为 <Second>秒
	double dFiguredSecPR;//推算出来的每圈转动所需要的时间.单位为 <Second>秒

}g_stDomeRotationState;

bool gb_AnnularCW[2];     //2008.07.14，环形坩埚旋转方向。默认为Hearth1 逆时针转(CW，见上)，Hearth2正转(CCW)。
                          //true:CW

//只在　本　cpp中用到的变量.因为不希望在头文件中引用　"OACnfgDoc.h"，所以没有定义成类成员变量.在 InitDialog中初始化
#define _OA_ROTATOR_DRAW_HEARTH_BUFFER_SIZE 20
struct _stDrawHearth g_stDrawCrucibleBuffer[_OA_ROTATOR_DRAW_HEARTH_BUFFER_SIZE][_OPTORUN_HEARTH_MAX]; //_OPTORUN_CRUCIBLE_MAX = 2
_stDrawHearth* GetOneDrawCrucibleBuffer(int iHearth)
{
	static int iBufPos[_OPTORUN_HEARTH_MAX] = { 0, 0 };

	ASSERT( (UINT)iHearth < _OPTORUN_HEARTH_MAX );

	iBufPos[iHearth]++;
	if(iBufPos[iHearth] >= _OA_ROTATOR_DRAW_HEARTH_BUFFER_SIZE)
	{
		iBufPos[iHearth] = 0;
	}

	return &g_stDrawCrucibleBuffer[iBufPos[iHearth]][iHearth];
}

CDrawCrucibleThread* g_pDrawCrucibleThread = NULL;
CRotatorThread* g_pRotateThread = NULL;
CDomeRotateThread* g_pDomeRotateThread = NULL;

	TCHAR gszRotatorBuf[512];


IMPLEMENT_DYNCREATE(COARotatorView, CFormView)

COARotatorView::COARotatorView()
	: CFormView(COARotatorView::IDD)
{
	m_bInit = false;
	m_OAHearth[0].iHearthType = _OA_HEARTH_TYPE_NONE;
	m_OAHearth[1].iHearthType = _OA_HEARTH_TYPE_NONE;

	g_stHearthRotationState.bAnnularPositive = true;
	g_stHearthRotationState.iAnnularStartWaitTime = 5000; //ms
	g_stHearthRotationState.iAnnularIntervalWaitTime = 5000;
	g_stHearthRotationState.iAnnularTestAngle = 180; //测试 180度 (半圈)

	g_stHearthRotationState.enHaveReversalFunction = _stHearthRotationState::_enHearthReversalUnknown;//没有反转功能.

	gb_AnnularCW[0] = true;     //true:CW,2008.07.14，环形坩埚旋转方向。默认为Hearth1 CW(从上看为逆时针！)，Hearth2 CCW。
	gb_AnnularCW[1] = false;    //false:CCW

	g_stReadDomeThreadData.reserve(1000); //
	/*
	_stReadDomeThreadData ss;
	ss.dwTickTime = DWORD(0xFFFFFFFF);
	ss.wDomeRotationCount = 1;
	g_stReadDomeThreadData.push_back(ss);

	ss.dwTickTime = DWORD(1234);
	ss.wDomeRotationCount = 15;
g_stReadDomeThreadData.push_back(ss);


	double dAveSecPR = (double) ((DWORD)(g_stReadDomeThreadData.back().dwTickTime-g_stReadDomeThreadData.front().dwTickTime))/((WORD)(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData.front().wDomeRotationCount));

	int x = g_stReadDomeThreadData.size();
	//x = g_stReadDomeThreadData.capacity();
	

	ss.dwTickTime = GetTickCount();
	ss.wDomeRotationCount = 1234;
	g_stReadDomeThreadData.push_back(ss);

	for(int i=0;i<100;i++)
	{
		ss.dwTickTime = GetTickCount();
		ss.wDomeRotationCount = i;
		g_stReadDomeThreadData.push_back(ss);
		x = g_stReadDomeThreadData.size();
		//TRACE2("size = %d		capacity = %d\r\n",x,g_stReadDomeThreadData.capacity());
	}
	
	g_stReadDomeThreadData.clear();
	TRACE2("After clear size = %d		capacity = %d\r\n",g_stReadDomeThreadData.size(),g_stReadDomeThreadData.capacity());

//	TRACE2("After clear at(0)dwTickTime = %d		at(0).wDomeRotationCount = %d\r\n",g_stReadDomeThreadData.at(0).dwTickTime,g_stReadDomeThreadData.at(0).wDomeRotationCount);
	*/
}

COARotatorView::~COARotatorView()
{

}

void COARotatorView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ROTATOR_CRUCIBLE, m_ListctrlCrucible);
	DDX_Control(pDX, IDC_STATIC_TEST_TIME, m_staticTestHearthTime);
	DDX_Control(pDX, IDC_LIST_ROTATOR_DOME, m_ListctrlDome);
	DDX_Control(pDX, IDC_STATIC_DOME_THREAD_STATE, m_staticexDomeThreadState);
}

BEGIN_MESSAGE_MAP(COARotatorView, CFormView)
//	ON_BN_CLICKED(IDC_BTN_INIT_CRUCIBLE, &COARotatorView::OnBnClickedBtnInitCrucible)
ON_BN_CLICKED(IDC_BTN_INIT_HEARTH, &COARotatorView::OnBnClickedBtnInitHearth)
ON_BN_CLICKED(IDC_BUTTON_TEST, &COARotatorView::OnBnClickedButtonTest)
ON_WM_TIMER()
ON_WM_ERASEBKGND()
ON_BN_CLICKED(IDC_BTN_INSERT_HEARTH1, &COARotatorView::OnBnClickedBtnInsertHearth1)
ON_BN_CLICKED(IDC_BTN_INSERT_HEARTH2, &COARotatorView::OnBnClickedBtnInsertHearth2)
ON_BN_CLICKED(IDC_BTN_DELETE_HEARTH_ROW, &COARotatorView::OnBnClickedBtnDeleteHearthRow)
ON_BN_CLICKED(IDC_BTN_SAVE_HEARTH_LIST, &COARotatorView::OnBnClickedBtnSaveHearthList)
ON_BN_CLICKED(IDC_BTN_START_HEARTH_TEST, &COARotatorView::OnBnClickedBtnStartHearthTest)
ON_BN_CLICKED(IDC_BTN_CANCEL_HEARTH_TEST, &COARotatorView::OnBnClickedBtnCancelHearthTest)
ON_BN_CLICKED(IDC_BTN_CREATE_HEARTH1, &COARotatorView::OnBnClickedBtnCreateHearth1)
ON_BN_CLICKED(IDC_BTN_CREATE_HEARTH2, &COARotatorView::OnBnClickedBtnCreateHearth2)
ON_BN_CLICKED(IDC_BTN_STATISTICS_EVALUATION, &COARotatorView::OnBnClickedBtnStatisticsEvaluation)
ON_BN_CLICKED(IDC_BTN_AUTOCREATE_DOME_SPEED, &COARotatorView::OnBnClickedBtnAutocreateDomeSpeed)
ON_BN_CLICKED(IDC_BTN_INSERT_DOME_SPEED, &COARotatorView::OnBnClickedBtnInsertDomeSpeed)
ON_BN_CLICKED(IDC_BTN_DELETE_DOME_ROW, &COARotatorView::OnBnClickedBtnDeleteDomeRow)
ON_BN_CLICKED(IDC_BTN_SAVE_DOME_LIST, &COARotatorView::OnBnClickedBtnSaveDomeList)
ON_BN_CLICKED(IDC_BTN_START_DOME_TEST, &COARotatorView::OnBnClickedBtnStartDomeTest)
ON_BN_CLICKED(IDC_BTN_CANCEL_DOME_TEST, &COARotatorView::OnBnClickedBtnCancelDomeTest)
END_MESSAGE_MAP()

// COARotatorView diagnostics
#ifdef _DEBUG
void COARotatorView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void COARotatorView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// COARotatorView message handlers
BOOL COARotatorView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class

	return CFormView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void COARotatorView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class
	fn_InitView();

	fn_ShowDomeWindow();
	fn_ShowWindow();
}

// 仅供OnInitialDialog中调用一次，初始化 RotatorView界面
void COARotatorView::fn_InitView(void)
{
	fn_SetIDsText();


	COLORREF g_HearthTextColor = RGB(0,128,255);//GetSysColor(COLOR_WINDOWTEXT);
	//m_staticHearth1Coder.textColor(g_HearthTextColor);
	//m_staticHearth1CruciblePos.textColor(g_HearthTextColor);
	//m_staticHearth2CruciblePos.textColor(g_HearthTextColor);

	//#define _OA_ROTATOR_DRAW_CRUCIBLE_BUFFER_SIZE 20
	//获取作图区
	CRect prect[_OPTORUN_HEARTH_MAX];
	GetDlgItem(IDC_STATIC_HEARTH1_PIC)->GetWindowRect(&prect[0]);
	GetDlgItem(IDC_STATIC_HEARTH2_PIC)->GetWindowRect(&prect[1]);
	ScreenToClient(&prect[0]); //转化成本页面坐标
	ScreenToClient(&prect[1]);

	HWND hWnd = GetSafeHwnd();
	ASSERT(IsWindow(hWnd));
	int i,j;
	for(i=0;i<_OPTORUN_HEARTH_MAX;i++)
	{
		for(j=0;j<_OA_ROTATOR_DRAW_HEARTH_BUFFER_SIZE;j++)
		{
			g_stDrawCrucibleBuffer[j][i].hwnd = hWnd;
			g_stDrawCrucibleBuffer[j][i].rect = prect[i];
			g_stDrawCrucibleBuffer[j][i].iHearth = i;
		}
	}

	g_pDrawCrucibleThread = (CDrawCrucibleThread*)AfxBeginThread(RUNTIME_CLASS(CDrawCrucibleThread), THREAD_PRIORITY_NORMAL,0,0);
	ASSERT(g_pDrawCrucibleThread);

	g_pRotateThread = (CRotatorThread*)AfxBeginThread(RUNTIME_CLASS(CRotatorThread), THREAD_PRIORITY_NORMAL,0,0);
	ASSERT(g_pRotateThread);
	g_pRotateThread->m_hWnd = GetSafeHwnd();

	g_pDomeRotateThread =(CDomeRotateThread*)AfxBeginThread(RUNTIME_CLASS(CDomeRotateThread), THREAD_PRIORITY_NORMAL,0,0);
	ASSERT(g_pDomeRotateThread);
	g_pDomeRotateThread->m_hWnd = GetSafeHwnd();

	CRect rt1,rt2,rt3;
	CDC* pdc = m_ListctrlCrucible.GetDC();
	CSize sizet = pdc->GetTextExtent(CString(TEXT("Hearth ")));
	CSize sizec = pdc->GetTextExtent(CString(TEXT(" 00:00:00 ")));
	m_ListctrlCrucible.ReleaseDC(pdc);
	m_ListctrlCrucible.SetExtendedStyle(m_ListctrlCrucible.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
//	CRect rect;
//	m_ListctrlCrucible.GetClientRect(rect);	
	m_ListctrlCrucible.InsertColumn(_ROTATOR_CRUCIBLE_LIST_COL_NO, TEXT("Hearth"),LVCFMT_LEFT,sizet.cx);
	m_ListctrlCrucible.InsertColumn(_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE, TEXT("Target"),LVCFMT_CENTER,sizet.cx);
	m_ListctrlCrucible.InsertColumn(_ROTATOR_CRUCIBLE_LIST_COL_START,TEXT("Start"),LVCFMT_CENTER,sizet.cx);
	m_ListctrlCrucible.InsertColumn(_ROTATOR_CRUCIBLE_LIST_COL_END,TEXT("End"),LVCFMT_CENTER,sizet.cx);
	m_ListctrlCrucible.InsertColumn(_ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME,TEXT("Test Time"),LVCFMT_CENTER,sizec.cx);
	m_ListctrlCrucible.GetClientRect(&rt3);
	m_ListctrlCrucible.InsertColumn(_ROTATOR_CRUCIBLE_LIST_COL_RESULT, TEXT("Result"),LVCFMT_LEFT, rt3.Width()-sizet.cx*4-sizec.cx - GetSystemMetrics(SM_CXVSCROLL) );


	//验证实际的 column 数
	CHeaderCtrl* pmyHeaderCtrl = m_ListctrlCrucible.GetHeaderCtrl();
	int iTest = pmyHeaderCtrl->GetItemCount();

	//移动 m_ListctrlCrucible
	
	GetWindowRect(&rt1); //页面窗口
	m_ListctrlCrucible.GetWindowRect(&rt2); //列表的位置
	rt3.bottom = rt1.bottom - 3;
	rt3.right  = rt2.right;
	rt3.left   = rt2.left;
	rt3.top    = rt2.top;
	ScreenToClient(&rt3);
	m_ListctrlCrucible.MoveWindow(&rt3);
	
	ClientToScreen(&rt3);//还原，供后面对齐使用

	m_ListctrlDome.GetWindowRect(&rt2);//
	//rt3.bottom//rt3.top 保持不变
	rt3.right = rt1.right-3;
	rt3.left  = rt2.left;
	ScreenToClient(&rt3);
	m_ListctrlDome.MoveWindow(&rt3);

	//左右分割线定位
	GetDlgItem(IDC_STATIC_DOME_HEARTH_SPACE)->GetWindowRect(&rt2);
	rt2.top = rt1.top + 3;
	rt2.bottom = rt1.bottom - 3;
	ScreenToClient(&rt2);
	GetDlgItem(IDC_STATIC_DOME_HEARTH_SPACE)->MoveWindow(&rt2);



	m_ListctrlDome.SetExtendedStyle(m_ListctrlCrucible.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_SPEED, TEXT("Speed"),LVCFMT_LEFT,sizet.cx-1);
	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_START_CNT, TEXT("Start"),LVCFMT_CENTER,sizet.cx);
	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_END_CNT,TEXT("End"),LVCFMT_CENTER,sizet.cx);
	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_WAITING_TIME,TEXT("Wait"),LVCFMT_CENTER,sizet.cx);
	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_TEST_TIME,TEXT("Test Time"),LVCFMT_CENTER,sizec.cx);
	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION,TEXT("SecPR"),LVCFMT_CENTER,sizec.cx);
	m_ListctrlDome.GetClientRect(&rt3);
	m_ListctrlDome.InsertColumn(_ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE, TEXT("RPM"),LVCFMT_LEFT, rt3.Width()-sizet.cx*4-sizec.cx*2 - GetSystemMetrics(SM_CXVSCROLL) );




	CString str;
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_DOME_SPEED);
	for(int i=10;i<=50;i+=5)
	{//环形坩埚下，填写转动速度. 
		str.Format(_T("%d"),i);
		pComboBox->AddString(str);
	}
	pComboBox->SetCurSel(0);

	//08.01.04
	m_staticTestHearthTime.GetWindowRect(&rt3);
	m_staticTestHearthTime.setFont(rt3.Height()-4);

	m_staticexDomeThreadState.GetWindowRect(&rt3);
	m_staticexDomeThreadState.setFont(rt3.Height()-4);

}

void COARotatorView::fn_ShowWindow(void)
{
	if(m_bInit)
	{//初始化成功
		bool bTest;
		EnterCriticalSection(&gCriticalSection_HearthState);
			bTest = g_bTestHearthRotation;
		LeaveCriticalSection(&gCriticalSection_HearthState);
		CString str;
		if(m_OAHearth[0].iHearthType != _OA_HEARTH_TYPE_NONE)
		{
			if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[0].iHearthType)
			{//点坩埚
				GetDlgItem(IDC_COMBO_HEARTH_12)->EnableWindow(!bTest);			    
			}
			else
			{//环形坩埚
				GetDlgItem(IDC_COMBO_HEARTH_1)->EnableWindow(!bTest);
			}
			GetDlgItem(IDC_BTN_CREATE_HEARTH1)->EnableWindow(!bTest);	
			GetDlgItem(IDC_BTN_INSERT_HEARTH1)->EnableWindow(!bTest);
		}

		if(m_OAHearth[1].iHearthType != _OA_HEARTH_TYPE_NONE)
		{
			if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[1].iHearthType)
			{
				GetDlgItem(IDC_COMBO_HEARTH_22)->EnableWindow(!bTest);
			}
			else
			{
				GetDlgItem(IDC_COMBO_HEARTH_2)->EnableWindow(!bTest);
			}

			GetDlgItem(IDC_BTN_CREATE_HEARTH2)->EnableWindow(!bTest);			//20160329jiang
			GetDlgItem(IDC_BTN_INSERT_HEARTH2)->EnableWindow(!bTest);
			// 2007.12.18，暂时禁止掉 Hearth2 环形方式
			if(m_OAHearth[1].iHearthType == _OA_HEARTH_TYPE_ANNULAR)
			{
				GetDlgItem(IDC_BTN_INSERT_HEARTH2)->EnableWindow(true); //2008.07.14
				GetDlgItem(IDC_BTN_CREATE_HEARTH2)->EnableWindow(true);
				GetDlgItem(IDC_CHECK_HEARTH1_DIRECTION)->EnableWindow(!bTest);//运行时不允许再选择
				GetDlgItem(IDC_CHECK_HEARTH2_DIRECTION)->EnableWindow(!bTest);
			}
			else
			{				//08.01.04 Debugging
				GetDlgItem(IDC_BTN_CREATE_HEARTH2)->EnableWindow(!bTest);
				//GetDlgItem(IDC_COMBO_HEARTH_2)->EnableWindow(!bTest);
				GetDlgItem(IDC_BTN_INSERT_HEARTH2)->EnableWindow(!bTest);
			}
			
		}
		
		GetDlgItem(IDC_BTN_START_HEARTH_TEST)->EnableWindow(!bTest);
		GetDlgItem(IDC_BTN_CANCEL_HEARTH_TEST)->EnableWindow(bTest);
		GetDlgItem(IDC_BTN_DELETE_HEARTH_ROW)->EnableWindow(!bTest);
		//GetDlgItem(IDC_BTN_INSERT_HEARTH2)->EnableWindow(!bTest);
		if(bTest)
		{
			m_staticTestHearthTime.textColor(RGB(255,0,0));
		}
		else
		{
			//m_staticTestHearthTime.textColor(RGB(0,0,0));
			m_staticTestHearthTime.textColor(GetSysColor(COLOR_3DFACE));
		}
	}
	else
	{//尚未初始化
		GetDlgItem(IDC_BTN_INIT_HEARTH)->EnableWindow(true);
		GetDlgItem(IDC_BTN_DELETE_HEARTH_ROW)->EnableWindow(false);
		//GetDlgItem(IDC_BTN_SAVE_HEARTH_LIST)->EnableWindow(false);
		GetDlgItem(IDC_BTN_START_HEARTH_TEST)->EnableWindow(false);
		GetDlgItem(IDC_BTN_CANCEL_HEARTH_TEST)->EnableWindow(false);
	}

	//暂时禁止Hearth1,因为还不清楚 Hearth1的转动方式 debugging 2007.12.18
//	GetDlgItem(IDC_BTN_INSERT_HEARTH1)->EnableWindow(false);
//	GetDlgItem(IDC_BTN_CREATE_HEARTH1)->EnableWindow(false);
}

/*
主要任务：
	１，读取坩埚及其当前状态，据此初始化界面
	２，给　/6248 发送0000，以清除plc内部状态
	３
本函数，在主线程执行，执行一次后禁止第二次．
*/

//int _stVacuumThreadData::iTest = 30; 
void COARotatorView::OnBnClickedBtnInitHearth()
{
	//TEST
	// WORD wTest = ( (1<<1) | (1<<(1+4)) );
	//int iSpeed = 160;
  //  WORD wValue = ((iSpeed/100)<<8) + (((iSpeed/10)%10)<<4) +iSpeed%10;
	WORD wTest = 1<<2;
	WORD wValue = 1<<2 | 1<<3;
	WORD wX = wTest & wValue;
/*
	DWORD dwEndTime = 0x1234;
	DWORD dwStartTime = 0x012;
	int iX = 3;
	int iTemp = 0.5 + (double)(dwEndTime-dwStartTime)/iX/1000 ;
		//(g_stHearthRotationState.dwCoderNotChangeStartTickTime - g_stHearthRotationState.dwStartTime)/iTurnBit/1000;
*/
	//正式开始
	bool btrue = true;
	bool bExist = false;
	for(int i =0;i<_OPTORUN_HEARTH_MAX;i++)
	{
		btrue &= g_pRotateThread->fn_GetHearthInfo(i,bExist,m_OAHearth[i].iCrucibleMaxNumber);
		if(btrue)
		{
			if(bExist)
			{
				g_pRotateThread->fn_GetHearthMonitorType(i,m_OAHearth[i].iHearthMonitorType);//2008.07.15

				if(0==m_OAHearth[i].iCrucibleMaxNumber)
				{
					m_OAHearth[i].iHearthType = _OA_HEARTH_TYPE_ANNULAR;
					g_pRotateThread->fn_GetHearthSpeed(i,m_OAHearth[i].iSpeed);
				}
				else
				{
					m_OAHearth[i].iHearthType = _OA_HEARTH_TYPE_CRUCIBLE;
					g_pRotateThread->fn_GetHearthCrucibleNumber(i,m_OAHearth[i].iCurCrucible );	
				}

				g_pRotateThread->fn_GetHearthCoderValue(i,m_OAHearth[i].iCoderVal);
						
				//发送一次更新
				//if( m_OAHearth[i].iHearthType != _OA_HEARTH_TYPE_ANNULAR )
				{
					_stDrawHearth* pDrawCrucible = ::GetOneDrawCrucibleBuffer(i);
					pDrawCrucible->iCrucibleMaxNumber = m_OAHearth[i].iCrucibleMaxNumber ;
					pDrawCrucible->iCoderVal = m_OAHearth[i].iCoderVal;
					g_pDrawCrucibleThread->PostThreadMessage(WM_ITEM3_DRAW_CRUCIBLE,0,LPARAM(pDrawCrucible));
				}
			}
			else
			{
				m_OAHearth[i].iHearthType = _OA_HEARTH_TYPE_NONE;
				m_OAHearth[i].iCrucibleMaxNumber = 0;
				g_pRotateThread->fn_GetHearthSpeed(i,m_OAHearth[i].iSpeed);
			}
		}
		else
		{
			AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR].IsEmpty()?
				TEXT("出错：plc通信不正常！\r\n plc communication failed."):g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);
			break;
		}
		
		//初始化一下
		//g_pRotateThread->fn_InitHearthToPT(i); //2007.12.29, 并不起到正面作用.
	}

	m_bInit = btrue;

	//ASSERT(m_bInit  &&  _OA_HEARTH_TYPE_NONE != m_OAHearth[0].iHearthType);
	//TESTing debugging 2007.12.20
	/*
	m_OAHearth[0].iHearthType = _OA_HEARTH_TYPE_ANNULAR;
	m_OAHearth[0].iCrucibleMaxNumber = 0;
	m_OAHearth[0].iCoderVal = 180;
	m_OAHearth[0].iSpeed = 123;
	m_OAHearth[1].iHearthType = _OA_HEARTH_TYPE_CRUCIBLE;//_OA_HEARTH_TYPE_ANNULAR;
	m_OAHearth[1].iCrucibleMaxNumber = 12;
	m_OAHearth[1].iCoderVal = 270;
	m_OAHearth[1].iCurCrucible = 10;
	m_bInit = true;
	SetDlgItemText(IDC_CHECK_START_FROM_SELECTION,TEXT("编译测试版，不可使用！"));
	//g_bTestHearthRotation = true;
	*/
	//Test End
	
	
	if(m_bInit)
	{
		fn_AfterInitHearth();
	
	//not-debug 2007.12.18
	//	AfxMessageBox(TEXT("Debug: fn_AfterInitHearth\r\nInitiate Success!"));
	}
	else
	{
		//AfxMessageBox(TEXT("Debug: Initiate Error! m_bInit==false!"),MB_OK|MB_ICONSTOP);
	}

	fn_ShowWindow();
}

void COARotatorView::OnDraw(CDC* pDC)
{
	//pDC->TextOut(100,100,CString(TEXT("xxx")));
	if(m_bInit)
	{	
		for(int i=0;i<_OPTORUN_HEARTH_MAX;i++)
		{
			if( m_OAHearth[i].iHearthType != _OA_HEARTH_TYPE_NONE )
			{
				_stDrawHearth* pDrawCrucible = ::GetOneDrawCrucibleBuffer(i);
				pDrawCrucible->iCrucibleMaxNumber = m_OAHearth[i].iCrucibleMaxNumber ;
				pDrawCrucible->iCoderVal = m_OAHearth[i].iCoderVal;
				pDrawCrucible->iCurCurcible = m_OAHearth[i].iCurCrucible;
				g_pDrawCrucibleThread->PostThreadMessage(WM_ITEM3_DRAW_CRUCIBLE,0,LPARAM(pDrawCrucible));
			}
		}	
	}
}

//测试旋转
void COARotatorView::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here
	static bool timerBegin = false;
	if(!timerBegin)
	{
		SetTimer(1,500,NULL);
		timerBegin = true;
	}
//	OnBnClickedBtnInitHearth();
}

void COARotatorView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	/*
	static int iCoderVal = 0;
	if(1==nIDEvent)
	{
		iCoderVal +=1;
		iCoderVal = iCoderVal%360;
		for(int i=0;i<_OPTORUN_HEARTH_MAX;i++)
		{			
				_stDrawHearth* pDrawCrucible = ::GetOneDrawCrucibleBuffer(i);
				pDrawCrucible->iCrucibleMaxNumber =12 ;
				pDrawCrucible->iCoderVal = iCoderVal;
				g_pDrawCrucibleThread->PostThreadMessage(WM_ITEM3_DRAW_CRUCIBLE,0,LPARAM(pDrawCrucible));			
		}		
	}
	*/

	//调试一下，以后更改.
	// OnBnClickedBtnInitHearth();

	CFormView::OnTimer(nIDEvent);
}

BOOL COARotatorView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return CFormView::OnEraseBkgnd(pDC);
}



/*
#define _OA_HEARTH_TYPE_NONE    0 //没有坩埚
#define _OA_HEARTH_TYPE_ANNULAR    1 //环形坩埚
#define _OA_HEARTH_TYPE_CRUCIBLE   2 //点坩埚
IDC_STATIC_CRUCIBLE1_EXIST_TEXT
IDC_STATIC_CODER1_TEXT
IDC_STATIC_CORDER1_VALUE
IDC_STATIC_CRUCIBLE1_POS_TEXT
IDC_STATIC_CRUCIBLE1_POS_NUMBER
*/
// 仅在初始化Hearth成功后，调用一次。用于显示Hearth的初始状态，比如，是否显示，以及什么内容.
void COARotatorView::fn_AfterInitHearth(void)
{

	ASSERT(m_bInit);
	GetDlgItem(IDC_BTN_INIT_HEARTH)->ShowWindow(SW_HIDE); //隐藏初始化按钮
	GetDlgItem(IDC_STATIC_TEST_TIME)->ShowWindow(SW_SHOW);
	//Hearth 1
	//IDC_COMBO_HEARTH_1  ANNULAR
	//IDC_COMBO_HEARTH_12 点坩埚
	if( _OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[0].iHearthType )
	{
		CString str;
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HEARTH_12);
		pComboBox->ShowWindow(SW_SHOW);
		for(int i=0;i<m_OAHearth[0].iCrucibleMaxNumber;i++)
		{
			str.Format(_T("%d"),i+1);
			pComboBox->AddString(str);
		}
		pComboBox->SetCurSel(0);

		GetDlgItem(IDC_BTN_INSERT_HEARTH1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_CREATE_HEARTH1)->ShowWindow(SW_SHOW);
	}
	else if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[0].iHearthType)
	{
		CString str;
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HEARTH_1);
		pComboBox->ShowWindow(SW_SHOW);
		for(int i=900;i>=50;i-=50)
		{//环形坩埚下，填写转动速度. 
			str.Format(_T("%d"),i);
			pComboBox->AddString(str);
		}
		pComboBox->SetCurSel(0);
		GetDlgItem(IDC_BTN_INSERT_HEARTH1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_CREATE_HEARTH1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHECK_HEARTH1_DIRECTION)->ShowWindow(SW_SHOW);
		CButton* btn = (CButton*)GetDlgItem(IDC_CHECK_HEARTH1_DIRECTION);
		btn->SetCheck(gb_AnnularCW[0]==true?BST_CHECKED:BST_UNCHECKED);

		GetDlgItem(IDC_STATIC_MONITOR_ALERT)->ShowWindow(m_OAHearth[0].iHearthMonitorType==_HEARTH_MONITOR_TYPE_OMRON ?SW_SHOW:SW_HIDE );
	}
	else
	{ //no hearth
		//GetDlgItem(IDC_STATIC_HEARTH1_TEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_HEARTH_1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_INSERT_HEARTH1)->ShowWindow(SW_HIDE);
	}

	//Hearth2
	//IDC_COMBO_HEARTH_2  ANNULAR
	//IDC_COMBO_HEARTH_22 点坩埚
	if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[1].iHearthType)
	{
		CString str;
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HEARTH_22);
		pComboBox->ShowWindow(SW_SHOW);
		for(int i=0;i<m_OAHearth[1].iCrucibleMaxNumber;i++)
		{
			str.Format(_T("%d"),i+1);
			pComboBox->AddString(str);
		}	
		pComboBox->SetCurSel(0);
		GetDlgItem(IDC_BTN_INSERT_HEARTH2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_CREATE_HEARTH2)->ShowWindow(SW_SHOW);
	}
	else if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[1].iHearthType)
	{
		CString str;
		CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_COMBO_HEARTH_2);
		pComboBox->ShowWindow(SW_SHOW);
		for(int i=900;i>=50;i-=50)
		{//环形坩埚下，填写转动速度. 
			str.Format(_T("%d"),i);
			pComboBox->AddString(str);
		}
		pComboBox->SetCurSel(0);
		GetDlgItem(IDC_BTN_INSERT_HEARTH2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_CREATE_HEARTH2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHECK_HEARTH2_DIRECTION)->ShowWindow(SW_SHOW);
		CButton* btn = (CButton*)GetDlgItem(IDC_CHECK_HEARTH2_DIRECTION);
		btn->SetCheck(gb_AnnularCW[1]==true?BST_CHECKED:BST_UNCHECKED);
		GetDlgItem(IDC_STATIC_MONITOR_ALERT)->ShowWindow(m_OAHearth[1].iHearthMonitorType==_HEARTH_MONITOR_TYPE_OMRON ?SW_SHOW:SW_HIDE );
	}
	else
	{ //no hearth
		//GetDlgItem(IDC_STATIC_HEARTH2_TEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_HEARTH_2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_INSERT_HEARTH2)->ShowWindow(SW_HIDE);
	}
}


void COARotatorView::OnBnClickedBtnInsertHearth1()
{
	// TODO: Add your control notification handler code here
	ASSERT(m_bInit  &&  _OA_HEARTH_TYPE_NONE != m_OAHearth[0].iHearthType);
	CString str;
	
	bool bPassCheck = false;
	if( _OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[0].iHearthType)
	{//点坩埚 指定目标坩埚位置		
		GetDlgItemText(IDC_COMBO_HEARTH_12,str);
		int iCrucible = _ttoi(str);
		if( iCrucible>0 && iCrucible<= m_OAHearth[0].iCrucibleMaxNumber)
		{ // [1,iCrucibleMaxNumber]
			bPassCheck = true;
		}
	}
	else if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[0].iHearthType)
	{//环形坩埚指定 转动速度 (0~999)
		GetDlgItemText(IDC_COMBO_HEARTH_1,str);
		int iCoderVal = _ttoi(str);
		if( iCoderVal>0 && iCoderVal<= 999)
		{ //
			bPassCheck = true;
		}
	}

	if(bPassCheck)
	{
		int iItems;
		POSITION pos = m_ListctrlCrucible.GetFirstSelectedItemPosition();
		if(pos)
		{//在当前选择项之前插入
			iItems = m_ListctrlCrucible.GetNextSelectedItem(pos);
		}
		else
		{//没有选择项，则插入到最后一行
			iItems = m_ListctrlCrucible.GetItemCount();
		}
		m_ListctrlCrucible.InsertItem(iItems,TEXT("1"));
		m_ListctrlCrucible.SetItemText(iItems,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,str);
	}
}

void COARotatorView::OnBnClickedBtnInsertHearth2()
{
	// TODO: Add your control notification handler code here
	ASSERT(m_bInit  &&  _OA_HEARTH_TYPE_NONE != m_OAHearth[1].iHearthType);
	CString str;
	
	bool bPassCheck = false;
	if( _OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[1].iHearthType)
	{//点坩埚 指定目标坩埚位置		
		GetDlgItemText(IDC_COMBO_HEARTH_22,str);
		int iCrucible = _ttoi(str);
		if( iCrucible>0 && iCrucible<= m_OAHearth[1].iCrucibleMaxNumber)
		{ // [1,iCrucibleMaxNumber]
			bPassCheck = true;
		}
	}
	else if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[1].iHearthType)
	{//环形坩埚指定 转动速度
		GetDlgItemText(IDC_COMBO_HEARTH_2,str);
		int iCoderVal = _ttoi(str);
		if( iCoderVal>=0 && iCoderVal< 1000)
		{ //[0,360)
			bPassCheck = true;
		}
	}

	if(bPassCheck)
	{
		int iItems;
		POSITION pos = m_ListctrlCrucible.GetFirstSelectedItemPosition();
		if(pos)
		{//在当前选择项之前插入
			iItems = m_ListctrlCrucible.GetNextSelectedItem(pos);
		}
		else
		{//没有选择项，则插入到最后一行
			iItems = m_ListctrlCrucible.GetItemCount();
		}
		m_ListctrlCrucible.InsertItem(iItems,TEXT("2"));
		m_ListctrlCrucible.SetItemText(iItems,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,str);
	}
}

//删除当前选择的第一项
void COARotatorView::OnBnClickedBtnDeleteHearthRow()
{
		POSITION pos = m_ListctrlCrucible.GetFirstSelectedItemPosition();
		if(pos)
		{//在当前选择项之前插入
			int iItems = m_ListctrlCrucible.GetNextSelectedItem(pos);
			m_ListctrlCrucible.DeleteItem(iItems);

			if( iItems < m_ListctrlCrucible.GetItemCount())
			{
				m_ListctrlCrucible.SetItemState(iItems,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
				m_ListctrlCrucible.EnsureVisible(iItems,false);
			}
		}
}

void COARotatorView::OnBnClickedBtnSaveHearthList()
{
	//TEST Start
/*
	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = TEXT("print");
	ShExecInfo.lpFile = TEXT("c:\\ipwuxian_1.pdf");		
	ShExecInfo.lpParameters = TEXT("");	
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;	
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
*/
//	fn_AutoSaveHearthList();
	//TEST End

	// TODO: Add your control notification handler code here
//	int iLength;	
	TCHAR  lptchTmp[512];  //不可重入
	TCHAR cbuf[512];
	DWORD bufsize = 512;
	
	int iCnt = m_ListctrlCrucible.GetItemCount();
	if(iCnt==0)
	{	
		SYSTEMTIME   st;   
		CString str;
		GetLocalTime(&st);   
		str.Format(TEXT("Error: no data! \r\n%4d-%02d-%02d %02d:%02d:%02d\r\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		AfxMessageBox(str);
		return ; 
	}

	::GetModuleFileName(NULL,lptchTmp,512);
	CString str(lptchTmp);
	int ibackslash = str.ReverseFind(TEXT('\\'));
	lptchTmp[ibackslash + 1]= TEXT('\0');

	bufsize = 512-2;
	::GetComputerName(cbuf,&bufsize);

	SYSTEMTIME   st;   
	GetLocalTime(&st);   
	str.Format(TEXT("-OA-HEARTH-%4d%02d%02d %02d%02d%02d.log"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);

	_tcscat(cbuf,str);
	_tcscat(lptchTmp,cbuf);
		CFileDialog dlg(FALSE);
	dlg.m_ofn.lpstrTitle = _T("Save Hearth Rotation Data into File");
	dlg.m_ofn.lpstrFilter = _T("Optorun Assitant log(*.log)\0*.log\0");
	dlg.m_ofn.lpstrInitialDir = lptchTmp; 
	dlg.m_ofn.lpstrFile = lptchTmp;

	if(dlg.DoModal()==IDOK)
	{
		str = dlg.GetPathName();
		if(-1==str.Find(_T(".log")))
		{str +=_T(".log");}

		fn_SaveHearthList(str);
/*		
			CFile fFile;
			if(!fFile.Open(str,CFile::modeCreate|CFile::modeWrite))
			{
				return ;
			}

			CString str;

			fFile.SeekToBegin();
			//Save unicode file head
#ifdef UNICODE
			lptchTmp[0] = 0xFEFF; //FF在低 FE在高
			fFile.Write(lptchTmp,1*sizeof(TCHAR)); 
#endif

			TCHAR* p = TEXT("OPTORUN ASSITANT TEST FILE\x0d\x0a");
			fFile.Write(p,_tcslen(p)*sizeof(TCHAR));

			p = TEXT("FILE TYPE: HEARTH ROTATION\x0d\x0a");
			fFile.Write(p,_tcslen(p)*sizeof(TCHAR));


			int iLength;
			iLength = _stprintf(lptchTmp,TEXT("PROGRAM VERSION: %.3f\x0d\x0a"),g_dProgramVersion);
			if(iLength)
				fFile.Write( lptchTmp,iLength * sizeof(TCHAR) );

			MultiByteToWideChar(CP_UTF8,0,__DATE__,strlen(__DATE__) + 1,cbuf,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			iLength = _stprintf(lptchTmp,TEXT("PROGRAM DATE: %s\x0d\x0a"),cbuf);
			if(iLength)
				fFile.Write( lptchTmp,iLength * sizeof(TCHAR) );

			iLength = _stprintf(lptchTmp,TEXT("PROGRAM DATA VERSION: %.3f\x0d\x0a"),g_dDataVersion);
			if(iLength)
				fFile.Write(lptchTmp, iLength * sizeof(TCHAR));

			//文件保存时刻
			DWORD bufsize = 256;
			::GetComputerName(cbuf,&bufsize);
			iLength = wsprintf(lptchTmp,TEXT("%s%s\x0d\x0a"),TEXT("Computer Name: "),cbuf);
			fFile.Write(lptchTmp,iLength*sizeof(TCHAR));

			SYSTEMTIME   st;   
			GetLocalTime(&st); 
			str.Format(TEXT("Save Time: %4d-%02d-%02d %02d:%02d:%02d\x0d\x0a"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
			fFile.Write(str,sizeof(TCHAR) * str.GetLength()); 

			//坩埚类型
			iLength = _stprintf(lptchTmp,TEXT("Hearth 1 TYPE: %s\x0d\x0a"),_OA_HEARTH_TYPE_NONE == m_OAHearth[0].iHearthType ? TEXT("No Hearth") :(_OA_HEARTH_TYPE_ANNULAR==m_OAHearth[0].iHearthType?TEXT("Annular"):TEXT("Cup")) );
			if(iLength)
				fFile.Write(lptchTmp, iLength * sizeof(TCHAR));

			iLength = _stprintf(lptchTmp,TEXT("Hearth 2 TYPE: %s\x0d\x0a"),_OA_HEARTH_TYPE_NONE == m_OAHearth[1].iHearthType ? TEXT("No Hearth") :(_OA_HEARTH_TYPE_ANNULAR==m_OAHearth[1].iHearthType?TEXT("Annular"):TEXT("Cup")) );
			if(iLength)
				fFile.Write(lptchTmp, iLength * sizeof(TCHAR));

			//数据起始标志
			p=TEXT("<DATA>\x0d\x0a");
			fFile.Write(p,_tcslen(p)*sizeof(TCHAR));


				HDITEM hdi;
				CHeaderCtrl* pmyHeaderCtrl = m_ListctrlCrucible.GetHeaderCtrl();

				//列表头
				str.Empty();
				hdi.mask = HDI_TEXT;
				hdi.pszText = cbuf;
				hdi.cchTextMax = 256;
				for(int i = 0 ; i< pmyHeaderCtrl->GetItemCount();i++)
				{
					pmyHeaderCtrl->GetItem(i,&hdi);
					str += hdi.pszText;
					str += TEXT('\t');
				}
				str += TEXT("\x0d\x0a");
				fFile.Write(str, str.GetLength() * sizeof(TCHAR));

				//列表内容
				for(int i=0;i<iCnt;i++)
				{
					iLength = _stprintf(lptchTmp,TEXT("%s\t%s\t%s\t%s\t%s\t%s\r\n"),m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_NO),
						m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE),
						m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_START),
						m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_END),
						m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME),
						m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_RESULT));
					fFile.Write(lptchTmp,iLength*sizeof(TCHAR));
				}

			p=TEXT("</DATA>\x0d\x0a");
			fFile.Write(p,_tcslen(p)*sizeof(TCHAR));
			*/
	}
}

void COARotatorView::OnBnClickedBtnStartHearthTest()
{
	// >> 获取当前转动列表起始行. 以及待转动位置
	int iItem = 0;
	if( BST_CHECKED == ((CButton*)GetDlgItem(IDC_CHECK_START_FROM_SELECTION ))->GetCheck())
	{
		POSITION pos = m_ListctrlCrucible.GetFirstSelectedItemPosition();
		if(pos)
		{//在当前选择项之前插入
			iItem = m_ListctrlCrucible.GetNextSelectedItem(pos);
		}
	}

	//确认当前行小于总行数
	if(iItem >= m_ListctrlCrucible.GetItemCount() )
	{//
		return;
	}

	//当前是否有环形坩埚,有环形坩埚的话,Control 模式是否为 Rotate.2008.07.15
	if(m_OAHearth[0].iHearthType==_OA_HEARTH_TYPE_ANNULAR || m_OAHearth[1].iHearthType==_OA_HEARTH_TYPE_ANNULAR)
	{
		bool bHearth1 = false;
		bool bHearth2 = false;
		CString str;
		for(int i=iItem;i<m_ListctrlCrucible.GetItemCount();i++)
		{
			str = m_ListctrlCrucible.GetItemText(i,0);
			if(str == TEXT("1"))
				bHearth1 = true;
			else if(str==TEXT("2"))
				bHearth2 = true;
		}

		int iCtrlState=0;
		bool bAlreadyWarning = false;//确保不连续提醒
		if( (bHearth1&&m_OAHearth[0].iHearthType==_OA_HEARTH_TYPE_ANNULAR) )
		{
			if( !g_pRotateThread->fn_GetAnnularHearthCntrl(0,iCtrlState))
			{//没有plc通信！//报错，并返回
				//AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);
				AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR].IsEmpty()?TEXT("错误：（坩埚控制错误） PLC 没有正常响应.测试停止！\r\nError: plc communication failed."):g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);
				return;
			}
			if(iCtrlState==0) // Fix模式，则警告，由用户选择是否继续，默认为否
			{
				//内容---提示：环形坩埚的Control模式不是Rotate，本程序将不能转动它，你确定继续吗？
				if(IDNO==AfxMessageBox(g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_CONTROL_WARNING].IsEmpty()?TEXT("Hearth Control不是Rotate模式，将不能转动．\r\nTOGGLE SWITCH->HEARTH CONTROL\r\n继续吗？"):g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_CONTROL_WARNING]
				,MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
				//if(IDNO==AfxMessageBox(g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_CONTROL_WARNING] ,MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
				{
					return;
				}
				bAlreadyWarning = true;
			}
		}
		if( bHearth2&&m_OAHearth[1].iHearthType==_OA_HEARTH_TYPE_ANNULAR &&!bAlreadyWarning)
		{
			if( !g_pRotateThread->fn_GetAnnularHearthCntrl(1,iCtrlState))
			{//没有plc通信！//报错，并返回
				//AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);
				AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR].IsEmpty()?TEXT("错误：（坩埚控制错误） PLC 没有正常响应.测试停止！\r\nError: plc communication failed."):g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);
				return;
			}
			if(iCtrlState==0) // Fix模式，则警告，由用户选择是否继续，默认为否
			{
				if(IDNO==AfxMessageBox(g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_CONTROL_WARNING].IsEmpty()?TEXT("Hearth Control不是Rotate模式，将不能转动．\r\nTOGGLE SWITCH->HEARTH CONTROL\r\n继续吗？"):g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_CONTROL_WARNING]
				,MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
				{
					return;
				}
			}
		}
	}


	CButton* btn = (CButton*)GetDlgItem(IDC_CHECK_HEARTH1_DIRECTION);
	gb_AnnularCW[0] = (btn->GetCheck()==BST_CHECKED)?true:false;
	btn = (CButton*)GetDlgItem(IDC_CHECK_HEARTH2_DIRECTION);
	gb_AnnularCW[1] = (btn->GetCheck()==BST_CHECKED)?true:false;

	fn_StartHearthTest(iItem);
}

void COARotatorView::OnBnClickedBtnCancelHearthTest()
{
	// TODO: Add your control notification handler code here
	bool bTest;
	EnterCriticalSection(&gCriticalSection_HearthState);
	bTest = g_bTestHearthRotation;
	LeaveCriticalSection(&gCriticalSection_HearthState);

	if(bTest)
	{
		//if(IDYES == AfxMessageBox(TEXT("正在测试坩埚转动中,你真的要退出?"),MB_YESNO | MB_ICONQUESTION|MB_DEFBUTTON2))
		if(IDYES == AfxMessageBox(g_item3_String[_ITEM3_STR_HEARTH_TEST_EXIT_Q].IsEmpty()?TEXT("正在测试坩埚转动中,你真的要取消测试?\r\nIt's testing Hearth rotation, Are you sure to cancel?"):g_item3_String[_ITEM3_STR_HEARTH_TEST_EXIT_Q]
			,MB_YESNO | MB_ICONQUESTION|MB_DEFBUTTON2))
		{
			EnterCriticalSection(&gCriticalSection_HearthState);
			g_bTestHearthRotation = false;
			LeaveCriticalSection(&gCriticalSection_HearthState);

			if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[g_stHearthRotationState.iCurHearth].iHearthType)
			{//环形坩埚,停止转动. 模拟按一次
				//gb_AnnularCW
				//g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.bAnnularPositive);
				g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,gb_AnnularCW[g_stHearthRotationState.iCurHearth]);
			}
			fn_ShowWindow();
		}
	}
}


LRESULT COARotatorView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch(message)
	{
	case WM_ITEM3_HEARTH_ROTATE_DATA:
		fn_OnHearthMonitorThread(wParam,lParam);
		break;
	case WM_ITEM3_DOME_ROTATE_DATA:
		fn_OnDomeMonitorThread(wParam,lParam);
		break;
	}
	return CFormView::WindowProc(message, wParam, lParam);
}

void COARotatorView::fn_OnDomeMonitorThread(WPARAM wParam, LPARAM lParam)
{
	_stDomeThreadData* pst = (_stDomeThreadData*)lParam;
	bool bStopByUser = false;

	//enPlcNoAnswer,enAckRight,enErrNotRotation,enErrSpeed,enErrHighState,enErrStop,enErrDoorSwitchOff,enErrSwitchNotManual}enStatus;
	static int iPlcErrCounts = 0;
	iPlcErrCounts = (iPlcErrCounts++)%4;
	static _stReadDomeThreadData staticDomeThreadData;

	int iFigureResult = 0;

	static bool st_bWaitingForStop = false; //用于等待 监测线程发来的 STOP 消息,以启动下一次。
	//完成单项测试时，st_bWaitingForStop = true; 等待监测线程 STOP消息
	//enErrStart: st_bWaitingForStop = false;
	//enErrStop:  进行下一项，或完成时存盘退出。

#define _XYZ_DOME_MAX_ROTATION_TIME		20000 //单位ms, 20s. 没有圈数变化，最长等待并报错的时间。
	switch(pst->enStatus)
	{
	case _stDomeThreadData::enAckRight:
		if( st_bWaitingForStop )
			break;
		//数据异常情况分类
		//1,长时间 转数 不变化
		//2,转数之间差大于1（程序没有跟上plc记录的速度。）
		//3,转数变化间隔时间 超过预估值。（plc可能发生漏记）
//		m_staticexDomeThreadState.textColor(GetSysColor(COLOR_3DFACE));//消隐状态
#ifdef _DEBUG
//		TRACE2("dwTickTime = %d, wRdData = %d\r\n",pst->dwTickTime,pst->wRdData);
		if(g_stReadDomeThreadData.size()>0 && g_stReadDomeThreadData.back().wDomeRotationCount != pst->wRdData)
		{
			TRACE2("dwTimeSpace = %.02f, RotationCnt = %d\r\n",(double)(pst->dwTickTime-g_stReadDomeThreadData.back().dwTickTime)/1000,
				pst->wRdData - g_stReadDomeThreadData.back().wDomeRotationCount);	
		}
#endif

		/*
PLC DA->Dome控制器->马达
计算算法更改如下:
1, 扣除前30圈数据
2, 再记录　speed*10　圈，比如: speed = 50，则记录　500圈
3, 实际总记录圈数　=  30 + speed*10.
4, 数据分析:
   a, 基本预测时间 t0 = 60/speed.
   b, 对记录圈数的时间进行排序, 找出落在 t0 ,2*t0, 3*t0, 4*t0一定范围内的数据个数(n1,n2,n3,n4).(暂时未发现 4倍t0,也就不考虑 5倍t0. ) 
   c, 平均每圈时间 = 总时间 / ( n1 + 2*n2 + 3*n3 + 4*n4 )
   d, 输出10圈时间 = xx.xx秒, RPM
*/

		//记录数据开始
		if( g_stReadDomeThreadData.size() == 0 )
		{//第一次
			staticDomeThreadData.dwTickTime = pst->dwTickTime;
			staticDomeThreadData.wDomeRotationCount = pst->wRdData;
			g_stReadDomeThreadData.push_back(staticDomeThreadData);//添加数据至末尾.

			_stprintf(gszRotatorBuf,TEXT("%d"),g_stReadDomeThreadData.back().wDomeRotationCount);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_START_CNT,gszRotatorBuf);
			break;
		}
		else
		{
			//记录一下时间值
			_stprintf(gszRotatorBuf,TEXT("%d"),(pst->dwTickTime - g_stReadDomeThreadData.front().dwTickTime)/1000);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_WAITING_TIME,gszRotatorBuf);
			
			if(g_stReadDomeThreadData.back().wDomeRotationCount == pst->wRdData)
			{//圈数没有发生变化
				//超过规定时间，没有发生圈数变化，报警！
				if((DWORD)( pst->dwTickTime-g_stReadDomeThreadData.back().dwTickTime) > _XYZ_DOME_MAX_ROTATION_TIME )
				{
					m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
					m_staticexDomeThreadState.SetWindowText(TEXT("Error: cannot detect dome rotation!\r\n错误：程序检测不到圈数变化，请确认连接正常及PLC更新."));
				}
				break;
			}
			else
			{
				//记录
				staticDomeThreadData.dwTickTime = pst->dwTickTime;
				staticDomeThreadData.wDomeRotationCount = pst->wRdData;
				g_stReadDomeThreadData.push_back(staticDomeThreadData);//添加数据至末尾.
			}
		}
		//记录数据结束-----------------------------


		//显示并判断数据-----------------------------------
		if( g_stReadDomeThreadData.size() > _ROTATE_DOME_NOT_COUNT )
		{
			DWORD dwTickInterval;
			int iCounted = 0;
			if( 1 == (g_stReadDomeThreadData.back().wDomeRotationCount - g_stReadDomeThreadData[g_stReadDomeThreadData.size()-2].wDomeRotationCount) )
			{
				dwTickInterval = g_stReadDomeThreadData.back().dwTickTime - g_stReadDomeThreadData[g_stReadDomeThreadData.size()-2].dwTickTime;
				for(int i=0 ; i<_ROTATE_DOME_ABNOMAL_TIMES ; i++)
				{
					if( dwTickInterval >= g_stDomeRotationState.dwLimit[i][0] && dwTickInterval <= g_stDomeRotationState.dwLimit[i][1])
					{
						g_stDomeRotationState.n[i]++;
						g_stDomeRotationState.dwTotalCountTime += dwTickInterval;	
						iCounted = 1;
						break;
					}
				}
			}
			//异常数值
			if( 0 == iCounted )
			{
				g_stDomeRotationState.nAbnomalCount++;
			}
#ifdef _DEBUG
			_stprintf(gszRotatorBuf,TEXT("1:%d, 2:%d, 3:%d, 4:%d, Abn:%d\r\n"),g_stDomeRotationState.n[0],
				g_stDomeRotationState.n[1],g_stDomeRotationState.n[2],g_stDomeRotationState.n[3],
				g_stDomeRotationState.nAbnomalCount
				);
			TRACE(gszRotatorBuf);
#endif //_DEBUG

			int n=0;
			for(int i=0;i<_ROTATE_DOME_ABNOMAL_TIMES;i++)
			{
				n += (i+1)*g_stDomeRotationState.n[i];
			}

			if( n!=0 )
			{
				double dT0 =(double) g_stDomeRotationState.dwTotalCountTime/n; //估算值
				//dT0 /= 1000.0;

				//先显示当前估算结果
				if(g_stDomeRotationState.nAbnomalCount < 5)
				{
				_stprintf(gszRotatorBuf,TEXT("%d: AveSecPR= %.3f, EstimatedSecPR= %.3f"),g_stReadDomeThreadData.size(), 
						((double)((g_stReadDomeThreadData.back().dwTickTime - g_stReadDomeThreadData[2].dwTickTime)/(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData[2].wDomeRotationCount)) )/1000.0,
						dT0/1000.0
						);	
				m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
				m_staticexDomeThreadState.textColor( RGB(0,192,0));
				}
				else
				{//异常多发，可能有问题.
					_stprintf(gszRotatorBuf,TEXT("%d: AveSecPR= %.3f, EstimatedSecPR= %.3f,  Abnormal:%d"),g_stReadDomeThreadData.size(), 
						((double)((g_stReadDomeThreadData.back().dwTickTime - g_stReadDomeThreadData[2].dwTickTime)/(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData[2].wDomeRotationCount)) )/1000.0,
						dT0/1000.0,g_stDomeRotationState.nAbnomalCount
						);	
					m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
					m_staticexDomeThreadState.textColor( RGB(0,192,0));
					m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
				}

				int iRealCount = 0;
				for(int i=0 ; i<_ROTATE_DOME_ABNOMAL_TIMES ; i++)
				{//将异常的时间累计进来
					iRealCount += (i+1)*g_stDomeRotationState.n[i];
				}

				//已达到有效个数值.可以退出了！g_stReadDomeThreadData.size()-->iRealCount
				if( ( iRealCount - g_stDomeRotationState.nAbnomalCount) > (_ROTATE_DOME_NOT_COUNT + g_stDomeRotationState.iTargetSpeed*10 ) )
				{
					//i,先暂停
					EnterCriticalSection(&gCriticalSection_DomeState);
					g_bTestDomeRotation = false; ////等待监测线程发送 stop消息.
					LeaveCriticalSection(&gCriticalSection_DomeState);
					st_bWaitingForStop  = true;

					//ii,输出分析结果，填充当前行
					_stprintf(gszRotatorBuf,TEXT("%d"),g_stReadDomeThreadData.back().wDomeRotationCount);
					m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_END_CNT,gszRotatorBuf);
					_stprintf(gszRotatorBuf,TEXT("%d"),(GetTickCount() - g_stDomeRotationState.dwStartTime)/1000);
					m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_TEST_TIME,gszRotatorBuf);
					_stprintf(gszRotatorBuf,TEXT("%.3f"),dT0/1000);
					m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION,gszRotatorBuf);

					_stprintf(gszRotatorBuf,TEXT("%.1f"),60.0 / (dT0/1000) );
					m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE,gszRotatorBuf);

					m_staticexDomeThreadState.bkColor(RGB(0,64,0)); //完成时改写背景颜色
				}
				/*
				//有效个数没达到，但也超过100了. 看最后连续10次的平均值，是否稳定在 0.0005之间
				else if((g_stReadDomeThreadData.size() - g_stDomeRotationState.nAbnomalCount)>(_ROTATE_DOME_NOT_COUNT+100))
				{

				}
				*/
				break;
			}

		} //if( g_stReadDomeThreadData.size() > _ROTATE_DOME_NOT_COUNT )
		
		else if(g_stReadDomeThreadData.size() > 3 )
		{
			_stprintf(gszRotatorBuf,TEXT("%d: AveSecPR= %.3f"),g_stReadDomeThreadData.size(), 
				((double)((g_stReadDomeThreadData.back().dwTickTime - g_stReadDomeThreadData[2].dwTickTime)/(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData[2].wDomeRotationCount)) )/1000.0);	
			m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
			m_staticexDomeThreadState.textColor( RGB(0,128,0));
		}
		
		else
		{
			_stprintf(gszRotatorBuf,TEXT("%d: Waiting..."),g_stReadDomeThreadData.size());
			m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
			m_staticexDomeThreadState.textColor( RGB(0,128,0));
			m_staticexDomeThreadState.bkColor(GetSysColor(COLOR_3DFACE));//恢复正常背景颜色
		}
		//显示并判断数据结束-----------------------------------/

		/*

		if( g_stReadDomeThreadData.size() > (_ROTATE_DOME_NOT_COUNT + g_stDomeRotationState.iTargetSpeed * 10) )
		{//超过预定次数，则停止转动，并输出结果
			//i,先暂停
			EnterCriticalSection(&gCriticalSection_DomeState);
				g_bTestDomeRotation = false; ////等待监测线程发送 stop消息.
			LeaveCriticalSection(&gCriticalSection_DomeState);
			st_bWaitingForStop  = true;
			
			
			double dT0 = 0.001; ////平均每圈的时间，单位 s
			int n1,n2,n3,n4; //1倍,2倍,3倍,4倍基准时间的个数.
			fn_CalcDomeRotationData(dT0,n1,n2,n3,n4,(double)(60/g_stDomeRotationState.iTargetSpeed));

			//ii,输出分析结果，填充当前行
			_stprintf(gszRotatorBuf,TEXT("%d"),g_stReadDomeThreadData.back().wDomeRotationCount);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_END_CNT,gszRotatorBuf);
			_stprintf(gszRotatorBuf,TEXT("%d"),(GetTickCount() - g_stDomeRotationState.dwStartTime)/1000);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_TEST_TIME,gszRotatorBuf);
			_stprintf(gszRotatorBuf,TEXT("%.3f"),dT0/1000);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION,gszRotatorBuf);

			_stprintf(gszRotatorBuf,TEXT("%.1f"),60.0 / (dT0/1000) );
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE,gszRotatorBuf);
		}
		*/

		/*

		//每次重新分析
		//输出当前次数, 2 次以上记录阶梯次数间隔. 超过最大次数停止
		iFigureResult = fn_FigureDomeThreadData(  g_stDomeRotationState.dAveSecPR, g_stDomeRotationState.dFiguredSecPR) ;

		if(	100 ==  iFigureResult || 50==iFigureResult)
		{
			//i,先暂停			
			EnterCriticalSection(&gCriticalSection_DomeState);
				g_bTestDomeRotation = false; ////等待监测线程发送 stop消息.
			LeaveCriticalSection(&gCriticalSection_DomeState);

			//ii,输出分析结果，填充当前行
			_stprintf(gszRotatorBuf,TEXT("%d"),g_stReadDomeThreadData.back().wDomeRotationCount);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_END_CNT,gszRotatorBuf);
			_stprintf(gszRotatorBuf,TEXT("%d"),(GetTickCount() - g_stDomeRotationState.dwStartTime)/1000);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_TEST_TIME,gszRotatorBuf);
			_stprintf(gszRotatorBuf,TEXT("%.02f"),g_stDomeRotationState.dFiguredSecPR);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION,gszRotatorBuf);

			_stprintf(gszRotatorBuf,TEXT("%.01f"),60.0 / g_stDomeRotationState.dFiguredSecPR );
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE,gszRotatorBuf);
			//顺利结束本次测试.

			st_bWaitingForStop  = true;
		}
		else
		{
			//只有一个数据，填写开始计数器值
			if(g_stReadDomeThreadData.size()==1)
			{
				_stprintf(gszRotatorBuf,TEXT("%d"),g_stReadDomeThreadData.front().wDomeRotationCount);
			}

			_stprintf(gszRotatorBuf,TEXT("%d"),(g_stReadDomeThreadData.back().dwTickTime - g_stReadDomeThreadData.front().dwTickTime)/1000);
			m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_WAITING_TIME,gszRotatorBuf);			
			
			//0:没有开始预测
			//1:仅平均值可以用
			//2:平均值与预测值都可以用，但此时尚不满足停止条件
			
			if( 2 == iFigureResult)
			{
				_stprintf(gszRotatorBuf,TEXT("%03d: Average SecPR = %.3f , estimated SecPR = %.3f"),g_stReadDomeThreadData.size(), g_stDomeRotationState.dAveSecPR,g_stDomeRotationState.dFiguredSecPR);
				m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
				m_staticexDomeThreadState.textColor( RGB(0,128,0));	
				m_staticexDomeThreadState.bkColor(GetSysColor(COLOR_3DFACE));
			}
			else if( 1 == iFigureResult)
			{
				_stprintf(gszRotatorBuf,TEXT("%03d: Average SecPR = %.3f "),g_stReadDomeThreadData.size(), g_stDomeRotationState.dAveSecPR);
				m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
				m_staticexDomeThreadState.textColor( RGB(0,128,0));	
				m_staticexDomeThreadState.bkColor(GetSysColor(COLOR_3DFACE));
			}
			else //0==iFigureResult
			{
				_stprintf(gszRotatorBuf,TEXT("Waiting: %03d"),g_stReadDomeThreadData.size());
				m_staticexDomeThreadState.SetWindowText(gszRotatorBuf);
				m_staticexDomeThreadState.textColor( RGB(0,128,0));	
				m_staticexDomeThreadState.bkColor(GetSysColor(COLOR_3DFACE));
			}

		}
		*/
		break;
	case	_stDomeThreadData::enErrStart:
		TRACE1("START:Target Speed = %d\r\n",g_stDomeRotationState.iTargetSpeed);
		st_bWaitingForStop = false;
		g_stReadDomeThreadData.clear();//清空

		//初始化一次g_stDomeRotationState内容
		g_stDomeRotationState.dwEpsilon = 490; //490ms
		g_stDomeRotationState.dwTotalCountTime = 0;
		g_stDomeRotationState.nAbnomalCount = 0;
		g_stDomeRotationState.dwStandardValue =  60000/g_stDomeRotationState.iTargetSpeed;
		for(int i=0;i<_ROTATE_DOME_ABNOMAL_TIMES;i++)
		{
			g_stDomeRotationState.dwLimit[i][0] = (i+1)*g_stDomeRotationState.dwStandardValue - g_stDomeRotationState.dwEpsilon;
			g_stDomeRotationState.dwLimit[i][1] = (i+1)*g_stDomeRotationState.dwStandardValue + g_stDomeRotationState.dwEpsilon;
			g_stDomeRotationState.n[i] = 0;
		}

		m_staticexDomeThreadState.SetWindowText(TEXT("Starting..."));
		m_staticexDomeThreadState.bkColor(GetSysColor(COLOR_3DFACE));//恢复正常背景颜色

		break;
	case _stDomeThreadData::enErrStop:
		g_stReadDomeThreadData.clear();//清空
		EnterCriticalSection(&gCriticalSection_DomeState);
		bStopByUser  = g_bTestDomeStopByUser;
		LeaveCriticalSection(&gCriticalSection_DomeState);
		if(!bStopByUser)
		{
			//后面是否还有未完成行，有的话则开始新行
			if( g_stDomeRotationState.iCurListItem + 1 < m_ListctrlDome.GetItemCount() )
			{
				fn_StartDomeTest(g_stDomeRotationState.iCurListItem+1);
			}
			else
			{//全部完成，自动保存一次结果.
				fn_AutoSaveDomeList();
				fn_SaveRestoreDomeConfigure(false);//恢复Dome状态.
			}
		}
		fn_ShowDomeWindow();
		break;
	case _stDomeThreadData::enPlcNoAnswer:
		g_stReadDomeThreadData.clear();//清空
		m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
		//m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? GetSysColor(COLOR_3DHIGHLIGHT) :GetSysColor(COLOR_3DFACE));
		m_staticexDomeThreadState.SetWindowText(TEXT("Error: PLC Read Error!"));
		
		_stprintf(gszRotatorBuf,TEXT("%d"),(GetTickCount() - g_stDomeRotationState.dwStartTime)/1000);
		m_ListctrlDome.SetItemText(g_stDomeRotationState.iCurListItem,_ROTATOR_DOME_LIST_COL_WAITING_TIME,gszRotatorBuf);
		
		//m_staticexDomeThreadState.textColor(GetSysColor(COLOR_3DFACE));
		break;
	case _stDomeThreadData::enErrNotRotation:
		g_stReadDomeThreadData.clear();//清空
		m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
		m_staticexDomeThreadState.SetWindowText(TEXT("Error: Dome is not Rotating!"));
		break;
	case _stDomeThreadData::enErrSpeed:
		g_stReadDomeThreadData.clear();//清空
		m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
		m_staticexDomeThreadState.SetWindowText(TEXT("Error: Dome speed is outof range!"));
		break;
	case _stDomeThreadData::enErrHighState:
		g_stReadDomeThreadData.clear();//清空
		m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
		m_staticexDomeThreadState.SetWindowText(TEXT("Error: Dome state must be <High>!"));
		break;
	case _stDomeThreadData::enErrDoorSwitchOff: //not rotation
		g_stReadDomeThreadData.clear();//清空.
		m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
		m_staticexDomeThreadState.SetWindowText(TEXT("Error: Dome door switch is off!"));
		break;
	case _stDomeThreadData::enErrSwitchNotManual: //not rotation
		g_stReadDomeThreadData.clear();//清空
		m_staticexDomeThreadState.bkColor( iPlcErrCounts==3 ? RGB(255,255,0):RGB(255,0,0));
		m_staticexDomeThreadState.SetWindowText(TEXT("Error: Dome Switch is not <MANUAL>!"));
		break;
	}
}

//从监测线程发来的数据包，lParam包含测试到的数据
void COARotatorView::fn_OnHearthMonitorThread(WPARAM wParam, LPARAM lParam)
{
	_stHearthThreadData* pst = (_stHearthThreadData*)lParam;
	bool bTest;
	EnterCriticalSection(&gCriticalSection_HearthState);
	bTest = g_bTestHearthRotation;
	LeaveCriticalSection(&gCriticalSection_HearthState);

	int iTurnBit;//,iReversalBit;
	bool bReallyFinish;       //真的完成？ //点坩埚是测试到 停转并到达预期的编码器号(点坩埚号). 环形坩埚是仍在旋转但已到达预期的编码器号
	bool bRotateStopError;  //不在转动状态出错。
	bool bRotatePosError; //转动不到位出错

	static int iRotateErrorTimes = 0;
	_stDrawHearth* pDrawCrucible;
	CString str;

	switch(pst->enStatus)
	{
	case _stHearthThreadData::enAckRight:
		g_stHearthRotationState.iCurCoder = pst->wRdData[g_stHearthRotationState.iCurHearth==0?0:2]; //0:2请参见 plcrdwrthread.cpp中相关区域
		g_stHearthRotationState.iCurCrucible = pst->wRdData[g_stHearthRotationState.iCurHearth==0?1:3];

		//编码器状态, 2008.01.03
		if(g_stHearthRotationState.iLastCoder != g_stHearthRotationState.iCurCoder)
		{
			g_stHearthRotationState.iCoderNotChangeTime = 0; //可以不需要？直接记录当前时刻即可

			g_stHearthRotationState.dwCoderNotChangeStartTickTime = pst->dwTickTime; 
			
			if(g_stHearthRotationState.iCurCoder>g_stHearthRotationState.iLastCoder)
			{
				g_stHearthRotationState.iCoderChangeCounts++;
			}
			else
			{
				g_stHearthRotationState.iCoderChangeCounts--;
			}
			g_stHearthRotationState.bStop = false;
		}
		else
		{
			//g_stHearthRotationState.iCoderChangeCounts = 0;	
		}

		//点坩埚
		if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[g_stHearthRotationState.iCurHearth].iHearthType)
		{
			bReallyFinish = false;
			bRotateStopError = false;
			bRotatePosError = false;

			if(g_stHearthRotationState.iTargetPos == g_stHearthRotationState.iStartPos)
				bReallyFinish = true;

			//有无坩埚号变化，因为测试中几乎不可能会跳过坩埚号的，所以最终结果与转动坩埚最多相差2.(带反转功能)
			if(g_stHearthRotationState.iLastCrucible != g_stHearthRotationState.iCurCrucible)
			{
				g_stHearthRotationState.iCrucibleChangeCounts++;
			}
			
			//超过可判停时间 (_ROTATE_CRUCIBLE_STOP_JUDGE_TIME时间内编码器值没有变化). 2008.01.03
			if(GetTickCount() - g_stHearthRotationState.dwCoderNotChangeStartTickTime >= _ROTATE_CRUCIBLE_STOP_JUDGE_TIME)
			{
				if( abs(g_stHearthRotationState.iTargetCoder - g_stHearthRotationState.iCurCoder) < 3  && g_stHearthRotationState.iTargetPos==g_stHearthRotationState.iCurCrucible)
				{//编码器值相差在 3度 之内，并且目标坩埚号与指定坩埚号相同，可以判停了！
					bReallyFinish = true;
					g_stHearthRotationState.bStop = true;
				}
				else
				{//否则转动出错。 两次输出间会出错，所以临时取消！
					if(GetTickCount() - g_stHearthRotationState.dwCoderNotChangeStartTickTime >= _ROTATE_CRUCIBLE_STOP_JUDGE_TIME*4)
					{//延长转动报警时间,因为 有的20点坩埚启动很慢.可能造成误判!
						bRotateStopError = true;
					}
				}
			}

			/* 2008.01.03
			//判断是否停止转动
			//Hearth1 正转：Q10.14 = 1 ,反转: Q10.15=1;
			//Hearth2 正转：Q10.11 = 1 ,反转: Q10.12=1; //点坩埚有效，环形坩埚不清楚,  而且可能对不同类型的坩埚并不正确//张俊林语！ 
			iTurnBit = (g_stHearthRotationState.iCurHearth == 0 ? 14:11);
			iReversalBit = (g_stHearthRotationState.iCurHearth == 0 ? 15:12);
			if(  ((WORD) 1<<iTurnBit | (WORD)1<<iReversalBit ) & pst->wRdData[4] )
			{//转动中，不做判断
				g_stHearthRotationState.bStop = false;
				if( g_stHearthRotationState.iLastCoder == g_stHearthRotationState.iCurCoder)
				{//相等， 
					iRotateErrorTimes++;
					if( iRotateErrorTimes > 100 )
					{
						bRotateStopError = true;   //转动出问题了！ 长时间没有 编码器值变化 而plc指示正在转动！
						iRotateErrorTimes = 0; //重新清一次0
					}
				}
				else
				{
					iRotateErrorTimes = 0;
				}
			}
			else
			{//没有判断到转动
				iRotateErrorTimes = 0;
				if(g_stHearthRotationState.bStop)
				{
					//点坩埚，不在旋转状态，而且编码器相差小于 3度， 可以判停了
					//S561 一直没有判停,很搞
					if( abs(g_stHearthRotationState.iTargetCoder - g_stHearthRotationState.iCurCoder) < 3  && g_stHearthRotationState.iTargetPos==g_stHearthRotationState.iCurHearth )
					{
						bReallyFinish = true;
					}

					//停止状态超过 4 秒钟，可以判转动出错了？   转动不到位!
					if( pst->dwTickTime - g_stHearthRotationState.dwStopTime > 4000)
					{
						//仅仅作为Debugging,  暂时取消 这里 2007.12.29
						//S561,  如下代码引起错误,错误原因 估计是 不能正确探测到 是否处于旋转状态

					
						//停止状态超过4秒钟，而且距离 目标 编码器值 超过 2 度，则判断出错！
						//if( abs(g_stHearthRotationState.iTargetCoder - g_stHearthRotationState.iCurCoder) > 2 )
						{
						//	bRotatePosError = true;
						}
						

					}
				}
				else
				{//第一次进入此处
					g_stHearthRotationState.dwStopTime = pst->dwTickTime;
					//本应在此处根据是否 反转 来确定 已经停止。 但 是否反转已经涉及到 plc内部， 可能并不统一。所以不能统一判断。
				}
				g_stHearthRotationState.bStop = true;
			}
			*/

			if(bTest)
			{//测试状态
				if(bReallyFinish)
				{//已经停止，则记录当前所耗费时间， 并移动到下一行， 如果没有则全部停止.
					
					//填写起始坩埚位置
					str.Format(TEXT("%2d"),g_stHearthRotationState.iStartPos );
					m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_START,str);

					//填写终止坩埚位置
					str.Format(TEXT("%2d"),g_stHearthRotationState.iTargetPos );
					m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_END,str);

					//填写最终经过时间
					//g_stHearthRotationState.dwCoderNotChangeStartTickTime
					//借用iTurnBit
					iTurnBit = 0.5+ (double)(g_stHearthRotationState.dwCoderNotChangeStartTickTime - g_stHearthRotationState.dwStartTime)/1000;
					//str.Format(TEXT("%02d:%02d"),iTurnBit/60,iTurnBit%60);
					str.Format(TEXT("%3d s"),iTurnBit);
					m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME,str);

					/*
			if( g_stHearthRotationState.iTargetPos > g_stHearthRotationState.iStartPos)
			{
				if(g_stHearthRotationState.iTargetPos-g_stHearthRotationState.iStartPos
					>= m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
				{
					g_stHearthRotationState.bReversal = true;
				}
				else
				{
					g_stHearthRotationState.bReversal = false;
				}
			}
			else
			{
				if(g_stHearthRotationState.iTargetPos+m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber - g_stHearthRotationState.iStartPos
					> m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
				{
					g_stHearthRotationState.bReversal = true;
				}
				else
				{
					g_stHearthRotationState.bReversal = false;
				}
			}*/
					//g_stHearthRotationState.iCoderChangeCounts, + 则coder变大, - 则coder变小.
					//g_stHearthRotationState.iCrucibleChangeCounts, Crucible变化次数.
					//不可，因为OTFC只能正转，没有反转功能.内部记录吧


					if(g_stHearthRotationState.iCoderChangeCounts > 0)
					{
						iTurnBit = (g_stHearthRotationState.iTargetPos + m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber - g_stHearthRotationState.iStartPos)%m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber;
					}
					else if(g_stHearthRotationState.iCrucibleChangeCounts==0)
					{
						iTurnBit = 0;
					}
					else
					{
						iTurnBit = -(g_stHearthRotationState.iCrucibleChangeCounts-2);
						//abs(g_stHearthRotationState.iTargetPos - g_stHearthRotationState.iStartPos);
						// 1-->17 可以报出 -16,hehe
					}

					/*
					//填写平均转动1个坩埚位的时间.
					//计算实际转动间隔了几个Hearth, 借用iTurnBit。
					//循环最小间隔
					iTurnBit = abs(g_stHearthRotationState.iTargetPos - g_stHearthRotationState.iStartPos);
					if(iTurnBit > m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
					{
						iTurnBit = m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber - iTurnBit;
					}			

					if( g_stHearthRotationState.iCrucibleChangeCounts - iTurnBit == 2) 
					{//记录到的坩埚号变化数目比 循环最小间隔 多2 ，则有反转
						//存在 1个 bug， 假如没有反转功能的， 20点坩埚 记录到 11次，而最小间隔是9.于是 bug产生
						if(g_stHearthRotationState.iCoderChangeCounts < 0)
						{
							g_stHearthRotationState.enHaveReversalFunction = _stHearthRotationState::_enHearthReversalTrue;//有反转功能.
							iTurnBit = -iTurnBit;
						}
						else
						{
							iTurnBit = abs(g_stHearthRotationState.iTargetPos - g_stHearthRotationState.iStartPos);
						}
					}
					else
					{//本次没有反转，转动的实际间隔是 检查到的转动变化数目.
						iTurnBit = g_stHearthRotationState.iCrucibleChangeCounts;
						if(g_stHearthRotationState.iCrucibleChangeCounts > m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber >>1)
						{//记录到的转动数目超过坩埚总数1半，而且本次没有反转
							g_stHearthRotationState.enHaveReversalFunction = _stHearthRotationState::_enHearthReversalFalse;//没有反转功能.
						}
					}
					*/

					double dTemp;
					if(iTurnBit==0)
					{
						dTemp = 0;
					}
					else
					{
						dTemp = (double) (g_stHearthRotationState.dwCoderNotChangeStartTickTime - g_stHearthRotationState.dwStartTime)/abs(iTurnBit)/1000;
					}
					//str.Format(TEXT("%d, Ave= %02d:%02d"),iTurnBit,iTemp/60,iTemp%60);
					str.Format(TEXT("%d, Ave= %.1f s"),iTurnBit,(double)dTemp);
					m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_RESULT,str);

					/*
					iTurnBit = g_stHearthRotationState.iTargetPos - g_stHearthRotationState.iStartPos;
					if( g_stHearthRotationState.iTargetPos > g_stHearthRotationState.iStartPos)
					{
						if(g_stHearthRotationState.iTargetPos-g_stHearthRotationState.iStartPos
							>= m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
						{
							iTurnBit =  m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber-(g_stHearthRotationState.iTargetPos-g_stHearthRotationState.iStartPos);
						}
						else
						{
							iTurnBit = g_stHearthRotationState.iTargetPos-g_stHearthRotationState.iStartPos;
						}
					}
					else
					{ 

						if(g_stHearthRotationState.iStartPos-g_stHearthRotationState.iTargetPos
							>= m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
						{
							iTurnBit =  m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber-(g_stHearthRotationState.iStartPos-g_stHearthRotationState.iTargetPos);
						}
						else
						{
							iTurnBit = g_stHearthRotationState.iStartPos-g_stHearthRotationState.iTargetPos;
						}
					}					
					//debugging.
					str.Format(TEXT("%02d:%02d"),((g_stHearthRotationState.dwCoderNotChangeStartTickTime - g_stHearthRotationState.dwStartTime)/iTurnBit/1000)/60,
						((g_stHearthRotationState.dwCoderNotChangeStartTickTime - g_stHearthRotationState.dwStartTime)/iTurnBit/1000)%60);
					m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_RESULT,str);
					*/


					if( m_ListctrlCrucible.GetItemCount() > g_stHearthRotationState.iCurListItem + 1)
					{//还有下一行
						g_stHearthRotationState.iCurListItem++;
						fn_StartHearthTest(g_stHearthRotationState.iCurListItem);
						

						//Debugging Test 2008.01.03
						//str.Format(TEXT("Start: List item %d"),g_stHearthRotationState.iCurListItem+1);
						//SetDlgItemText( IDC_STATIC_DEBUG_TEXT,str);
						//输出
					}
					else
					{//已经是最后一行，则报告完成，并退出!
						EnterCriticalSection(&gCriticalSection_HearthState);
						g_bTestHearthRotation = false;
						LeaveCriticalSection(&gCriticalSection_HearthState);

						fn_AutoSaveHearthList();//Add, 2008.01.08
						//AfxMessageBox(TEXT("恭喜：坩埚转动测试完成！"));					
						AfxMessageBox(g_item3_String[_ITEM3_STR_HEARTH_TEST_FINISH].IsEmpty()?TEXT("恭喜：坩埚转动测试完成！\r\nCongradulation: Hearth rotation test finished! "):g_item3_String[_ITEM3_STR_HEARTH_TEST_FINISH]);
					}
				}
				else
				{
					if( bRotatePosError || bRotateStopError)
					{//停止测试状态. 并报告发生转动错误！
						EnterCriticalSection(&gCriticalSection_HearthState);
						g_bTestHearthRotation = false;
						LeaveCriticalSection(&gCriticalSection_HearthState);

						if(bRotatePosError)
						{
							//AfxMessageBox(TEXT("错误： 坩埚转动不到位"));
							AfxMessageBox(g_item3_String[_ITEM3_STR_HAERTH_ROTAT_POS_ERR].IsEmpty()?TEXT("错误： 坩埚转动不到位！\r\nError: Hearth rotation error."):g_item3_String[_ITEM3_STR_HAERTH_ROTAT_POS_ERR]);
						}
						else
						{
							//AfxMessageBox(TEXT("错误： 转动超时！"));
							AfxMessageBox(g_item3_String[_ITEM3_STR_HEARTH_ROTAT_OVER_TIME].IsEmpty()?TEXT("错误： 坩埚转动超时！！\r\nError: Hearth rotation overtime."):g_item3_String[_ITEM3_STR_HEARTH_ROTAT_OVER_TIME]);
						}
					}
					else
					{//测试中途，IDC_STATIC_TEST_TIME
						//借用iTurnBit;						
						iTurnBit = 0.5+ (double)(pst->dwTickTime - g_stHearthRotationState.dwStartTime)/1000;
						if( pst->dwTickTime < g_stHearthRotationState.dwStartTime)
							iTurnBit = 0;
						str.Format(TEXT("%02d:%02d"),iTurnBit/60, iTurnBit%60);
						SetDlgItemText(IDC_STATIC_TEST_TIME,str);
					}
				}
			}
		}//点坩埚
		else
		{//环形坩埚
			//ASSERT(0==g_stHearthRotationState.iCurHearth);  //暂不支持 Hearth2是环形坩埚！ 2007.12.27//取消,2008.07.15
			//此处在 cancel 后可能 会发生 bug报警,很奇特

			if(bTest)
			{ //环形坩埚，测试中
				bReallyFinish = false;
				bRotateStopError = false;
				bRotatePosError = false;
				
				//3884.08 Hearth1 ,//Hearth2,2008.07.15
				if( ((WORD) 1<<8)  & (g_stHearthRotationState.iCurHearth==0?pst->wRdData[_HEARTH_READ_PLC_ADD_ROTATE_1_ANNULAR]:pst->wRdData[_HEARTH_READ_PLC_ADD_ROTATE_2_ANNULAR] ))
				{//转动中
					g_stHearthRotationState.bStop = false;
					if( g_stHearthRotationState.iLastCoder == g_stHearthRotationState.iCurCoder)
					{//相等， 
						iRotateErrorTimes++;
						if( iRotateErrorTimes > 1000 ) //2009.12.14 100->1000 S702\S703三级变速，导致转动特别缓慢，约降低10倍.
						{
							bRotateStopError  = true;   //转动出问题了！ 长时间没有 编码器值变化 而plc指示正在转动！
							iRotateErrorTimes = 0;      //重新清一次0
						}
					}
					else
					{//不相等
						iRotateErrorTimes = 0;
					}
				}
				else
				{
					//环形坩埚，测试过程必须处于转动状态. 否则报错.  //此处混淆了 等待状态和非等待状态同属于测试状态.
					//如果正在等待下一次开始则例外!
					iRotateErrorTimes = 0;
					if(g_stHearthRotationState.bStop)
					{//正在 debugging . 2007.12.28
						//停止状态超过 100 秒钟，可以判转动出错了？ 报错.增大 时间判断, 避开等待状态的问题
						//2009.12.14， 1000秒钟
						if( pst->dwTickTime - g_stHearthRotationState.dwStopTime > 1000000)
						{
							bRotateStopError = true;
						}
					}
					else
					{//第一次进入此处
						g_stHearthRotationState.dwStopTime = pst->dwTickTime;
					}
					g_stHearthRotationState.bStop = true;
				}

				static bool bOnceDifferent  = false;
				if( g_stHearthRotationState.bAnnularBeginingWaiting )
				{
					if( GetTickCount()-g_stHearthRotationState.dwStartingWaitingTick > g_stHearthRotationState.iAnnularStartWaitTime)
					{
						if( 0 == iRotateErrorTimes && false == g_stHearthRotationState.bStop)
						{
							g_stHearthRotationState.bAnnularBeginingWaiting = false;
							g_stHearthRotationState.iStartCoder = g_stHearthRotationState.iCurCoder;
							
							//2008.01.03, S561正转时，实际编码器朝小的方向走，所以 取一次反
							//对测试180角度来说，结果一样. 暂时不可设定 测试角度.
							if(!g_stHearthRotationState.bAnnularPositive)
							{//正转
								g_stHearthRotationState.iTargetCoder = (g_stHearthRotationState.iCurCoder + g_stHearthRotationState.iAnnularTestAngle)%360;
							}
							else
							{//反转
								g_stHearthRotationState.iTargetCoder = ( 360 + g_stHearthRotationState.iCurCoder - g_stHearthRotationState.iAnnularTestAngle)%360;
							}
							//每次开始测试前, 置 一次相同为 false
							bOnceDifferent = false;

							
							//记录起始编码器值
							str.Format(TEXT("% 3d"),g_stHearthRotationState.iStartCoder);
							m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem, _ROTATOR_CRUCIBLE_LIST_COL_START,str);

							g_stHearthRotationState.dwStartTime = pst->dwTickTime ; //重新记录起始时间

						}
					}
				}
				else if( g_stHearthRotationState.bAnnularIntervalWaiting )
				{
					if( GetTickCount() - g_stHearthRotationState.dwStartingWaitingTick > g_stHearthRotationState.iAnnularIntervalWaitTime )
					{
						g_stHearthRotationState.bAnnularIntervalWaiting = false;
						fn_StartHearthTest(g_stHearthRotationState.iCurListItem);
					}
				}
				else
				{//正常 测试过程
					//360度测试,比较麻烦,因为有可能存在多次 iCurCoder 与  TargetCoder,所以必须有个变量记录 下 至少出现过一次不同后才能继续进行判断.
					if(bOnceDifferent)
					{//至少存在 1 次编码器值不同,才可以检测 是否 完成
						//g_stHearthRotationState.iCoderChangeCounts
						//if( g_stHearthRotationState.bAnnularPositive )
						if( g_stHearthRotationState.iCoderChangeCounts > 0) //
						{//编码器值递增的情况, 当前编码器值 大于 目标编码器值 3度以内, 立即停止.
							if( (g_stHearthRotationState.iCurCoder + 360 - g_stHearthRotationState.iTargetCoder) % 360 < 4 )
							{
								bReallyFinish = true;
							}

							//2008.04.02，添加.　对应速度低时，过长时间测试没有必要.
							if(  GetTickCount()-g_stHearthRotationState.dwStartTime > 10*60*1000 )
							{//超过１０分钟
								if( (g_stHearthRotationState.iCurCoder != g_stHearthRotationState.iLastCoder) && (g_stHearthRotationState.iCurCoder + 360 - g_stHearthRotationState.iStartCoder) % 360 >= 20 )
								{   ////当前发生编码器值与上次比发生变化. 而且编码器值与起始变化超过20度
									bReallyFinish = true;
								}
							}
						}
						else
						{// 编码器值递减的情况
							if( (g_stHearthRotationState.iTargetCoder + 360 - g_stHearthRotationState.iCurCoder) %360 < 4)
							{
								bReallyFinish = true;
							}

							//2008.04.02，添加.　对应速度低时，过长时间测试没有必要.
							if(  GetTickCount()-g_stHearthRotationState.dwStartTime > 10*60*1000 )
							{//超过１０分钟
								if( (g_stHearthRotationState.iCurCoder != g_stHearthRotationState.iLastCoder) && (g_stHearthRotationState.iStartCoder + 360 - g_stHearthRotationState.iCurCoder) %360 >= 20 )
								{   //编码器值变化超过20度
									bReallyFinish = true;
								}
							}

						}
					}
					else
					{
						if(g_stHearthRotationState.iCurCoder != g_stHearthRotationState.iLastCoder)
						{
							bOnceDifferent = true;
						}
					}

					if(bReallyFinish)
					{
						//完成1项后立即停止旋转!
						//2007.12.29
						//g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.bAnnularPositive);
						g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,gb_AnnularCW[g_stHearthRotationState.iCurHearth]);
						//报告停止位置
						str.Format(TEXT("% 3d"),g_stHearthRotationState.iCurCoder);
						m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem, _ROTATOR_CRUCIBLE_LIST_COL_END,str);

						//报告经过时间
						int iTemp = 0.5+(double)( pst->dwTickTime - g_stHearthRotationState.dwStartTime )/1000;
						str.Format(TEXT("%02d:%02d:%02d"),iTemp/3600,(iTemp/60)%60 ,iTemp%60);
						m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME,str);

						//g_stHearthRotationState.iCoderChangeCounts
						// 整圈估计 时间/1000 * 360 
						////记录实际转动的角度数
						if( g_stHearthRotationState.iCoderChangeCounts > 0)
						{//编码器递增
							iTemp = (360 + g_stHearthRotationState.iCurCoder - g_stHearthRotationState.iStartCoder)%360;
						}
						else
						{//编码器递减
							iTemp = (g_stHearthRotationState.iStartCoder + 360 -g_stHearthRotationState.iCurCoder)%360;
						}
						iTemp = 0.5 + (double)(pst->dwTickTime - g_stHearthRotationState.dwStartTime ) * 0.36  / iTemp; //转化成 1圈 360度 的时间 *360 /1000

						str.Format(TEXT("%02d:%02d:%02d"),iTemp/3600,(iTemp/60)%60, iTemp%60);
						m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_RESULT,str);


						if( m_ListctrlCrucible.GetItemCount() > g_stHearthRotationState.iCurListItem + 1)
						{//还有下一行
							g_stHearthRotationState.iCurListItem++;
							//  放在
							//进入中间间歇期
							g_stHearthRotationState.bAnnularIntervalWaiting = true;
							g_stHearthRotationState.dwStartingWaitingTick = GetTickCount();
						}
						else
						{//已经是最后一行，则报告完成，并退出!
							EnterCriticalSection(&gCriticalSection_HearthState);
							g_bTestHearthRotation = false;
							LeaveCriticalSection(&gCriticalSection_HearthState);
							fn_AutoSaveHearthList();//Add, 2008.01.08
							//AfxMessageBox(TEXT("恭喜：坩埚转动测试完成！"));	
							AfxMessageBox(g_item3_String[_ITEM3_STR_HEARTH_TEST_FINISH].IsEmpty()?TEXT("恭喜：坩埚转动测试完成！\r\nCongradulation: Hearth rotation test finished! "):g_item3_String[_ITEM3_STR_HEARTH_TEST_FINISH]);
						}
					}
					else
					{
						if( bRotatePosError || bRotateStopError)
						{//停止测试状态. 并报告发生转动错误！
							EnterCriticalSection(&gCriticalSection_HearthState);
							g_bTestHearthRotation = false;
							LeaveCriticalSection(&gCriticalSection_HearthState);

							if(bRotatePosError)
							{
								//AfxMessageBox(TEXT("错误： 坩埚转动不到位"));
								AfxMessageBox(g_item3_String[_ITEM3_STR_HAERTH_ROTAT_POS_ERR].IsEmpty()?TEXT("错误： 坩埚转动不到位！\r\nError: Hearth rotation error."):g_item3_String[_ITEM3_STR_HAERTH_ROTAT_POS_ERR]);
							}
							else
							{
								//AfxMessageBox(TEXT("错误： 转动超时！"));
								AfxMessageBox(g_item3_String[_ITEM3_STR_HEARTH_ROTAT_OVER_TIME].IsEmpty()?TEXT("错误： 坩埚转动超时！！\r\nError: Hearth rotation overtime."):g_item3_String[_ITEM3_STR_HEARTH_ROTAT_OVER_TIME]);
							}

						}
						else
						{//测试中途，IDC_STATIC_TEST_TIME
							iTurnBit = 0.5+ (double)(pst->dwTickTime - g_stHearthRotationState.dwStartTime)/1000;
							if( pst->dwTickTime < g_stHearthRotationState.dwStartTime)
								iTurnBit = 0;
							str.Format(TEXT("%02d:%02d"),iTurnBit/60, iTurnBit%60);
							//str.Format(TEXT("%02d:%02d"),((pst->dwTickTime - g_stHearthRotationState.dwStartTime)/1000)/60, ((pst->dwTickTime - g_stHearthRotationState.dwStartTime)/1000)%60);
							SetDlgItemText(IDC_STATIC_TEST_TIME,str);
						}
					}
				}//正常测试过程
			}//环形坩埚测试中
		}//环形坩埚

		//发送绘图指令！ 独立于坩埚测试之外，仅供用户界面显示之用.
		if( m_OAHearth[0].iHearthType != _OA_HEARTH_TYPE_NONE )
		{
			pDrawCrucible = ::GetOneDrawCrucibleBuffer(0);
			m_OAHearth[0].iCoderVal = pst->wRdData[0];

			//pDrawCrucible->iCurCurcible = m_OAHearth[i].iCurCrucible;
			pDrawCrucible->iCrucibleMaxNumber = m_OAHearth[0].iCrucibleMaxNumber ;
			pDrawCrucible->iCoderVal = m_OAHearth[0].iCoderVal;


			if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[0].iHearthType)
			{
				pDrawCrucible->iSpeed = m_OAHearth[0].iSpeed;
			}
			else
			{
				m_OAHearth[0].iCurCrucible = pst->wRdData[1];
				pDrawCrucible->iCurCurcible = m_OAHearth[0].iCurCrucible;
			}
			g_pDrawCrucibleThread->PostThreadMessage(WM_ITEM3_DRAW_CRUCIBLE,0,LPARAM(pDrawCrucible));
		}

		if( m_OAHearth[1].iHearthType != _OA_HEARTH_TYPE_NONE )
		{
			pDrawCrucible = ::GetOneDrawCrucibleBuffer(1);
			m_OAHearth[1].iCoderVal = pst->wRdData[2];

			pDrawCrucible->iCrucibleMaxNumber = m_OAHearth[1].iCrucibleMaxNumber ;
			pDrawCrucible->iCoderVal = m_OAHearth[1].iCoderVal;

			if(_OA_HEARTH_TYPE_ANNULAR == m_OAHearth[1].iHearthType)
			{
				pDrawCrucible->iSpeed = m_OAHearth[1].iSpeed;
			}
			else
			{
				m_OAHearth[1].iCurCrucible = pst->wRdData[3];
				pDrawCrucible->iCurCurcible = m_OAHearth[1].iCurCrucible;
			}
			g_pDrawCrucibleThread->PostThreadMessage(WM_ITEM3_DRAW_CRUCIBLE,0,LPARAM(pDrawCrucible));
		}

		g_stHearthRotationState.iLastCoder = g_stHearthRotationState.iCurCoder;
		g_stHearthRotationState.iLastCrucible = g_stHearthRotationState.iCurCrucible;

		fn_ShowWindow();
		break;
	case _stHearthThreadData::enPlcNoAnswer:
		EnterCriticalSection(&gCriticalSection_HearthState);
		g_bTestHearthRotation = false;
		LeaveCriticalSection(&gCriticalSection_HearthState);
		fn_ShowWindow();
		//AfxMessageBox(_T("错误： PLC 没有正常响应.测试停止\r\ncase _stHearthThreadData::enPlcNoAnswer:"),MB_OK|MB_ICONSTOP);
		AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR].IsEmpty()?TEXT("错误：（坩埚状态未响应） PLC 没有正常响应.测试停止！\r\nError: plc communication failed."):g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);
		break;
	}
}

//iItem，起始 测试的行号
bool COARotatorView::fn_StartHearthTest(int iItem)
{
	if( iItem >= m_ListctrlCrucible.GetItemCount())
	{//防止出现意外 删除! 2007.12.28
		EnterCriticalSection(&gCriticalSection_HearthState);
			g_bTestHearthRotation = false;
		LeaveCriticalSection(&gCriticalSection_HearthState);
		fn_ShowWindow();
		return false;
	}

	g_stHearthRotationState.iCurListItem = iItem;
	//清除当前行已有内容 //以防是第二次测量.
	m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_START,TEXT(""));
	m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_END,TEXT(""));
	m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME,TEXT(""));
	m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_RESULT,TEXT(""));	

	//debugging 2007.12.18 LVIS_SELECTED
	//m_ListctrlCrucible.SetItem(iItem,0,LVIF_STATE,0,0,LVIS_SELECTED,LVIS_SELECTED,0);
	m_ListctrlCrucible.SetItemState(iItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	m_ListctrlCrucible.EnsureVisible(iItem,false);

	CString str = m_ListctrlCrucible.GetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_NO);
	
	g_stHearthRotationState.iCurHearth = _ttoi(str) - 1;  //从1,2转成 0,1
	ASSERT(g_stHearthRotationState.iCurHearth< 2 );
	ASSERT(m_OAHearth[g_stHearthRotationState.iCurHearth].iHearthType != _OA_HEARTH_TYPE_NONE);
	
	//目标值
	str = m_ListctrlCrucible.GetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE);
	g_stHearthRotationState.iTargetPos = _ttoi(str);

	g_stHearthRotationState.iRotationTime = 0;
	g_stHearthRotationState.bStop = false;

	if( m_OAHearth[g_stHearthRotationState.iCurHearth].iHearthType ==_OA_HEARTH_TYPE_ANNULAR)
	{//环形坩埚 iTargetSpeed
		g_stHearthRotationState.iTargetSpeed = g_stHearthRotationState.iTargetPos;
	}
	else
	{   //点坩埚，目标坩埚对应的 编码器读数
		double d = 360/m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber;
		g_stHearthRotationState.iTargetCoder  = (int)( d*(g_stHearthRotationState.iTargetPos-1) + 0.5);
	}
	
	// >> 获取当前Hearth编码器及坩埚位置
	bool bPlcOK = g_pRotateThread->fn_GetHearthCoderValue(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.iLastCoder);
	g_stHearthRotationState.iCurCoder = g_stHearthRotationState.iLastCoder;
	
	if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[g_stHearthRotationState.iCurHearth].iHearthType)	
	{//点坩埚
		bPlcOK &= g_pRotateThread->fn_GetHearthCrucibleNumber(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.iCurCrucible);
		//点坩埚，判断，是否逆转.  因为逆转存在 短暂停顿.  顺转则不存在停顿.
		if(bPlcOK)
		{
			g_stHearthRotationState.iStartPos             = g_stHearthRotationState.iCurCrucible;
			g_stHearthRotationState.iLastCrucible         = g_stHearthRotationState.iCurCrucible;
			g_stHearthRotationState.iCrucibleChangeCounts = 0;
			//G1268, 12点坩埚
			// 1-->7 反转
			// 7-->1 正转
			if( g_stHearthRotationState.iTargetPos > g_stHearthRotationState.iStartPos)
			{
				if(g_stHearthRotationState.iTargetPos-g_stHearthRotationState.iStartPos
					>= m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
				{
					g_stHearthRotationState.bReversal = true;
				}
				else
				{
					g_stHearthRotationState.bReversal = false;
				}
			}
			else
			{
				if(g_stHearthRotationState.iTargetPos+m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber - g_stHearthRotationState.iStartPos
					> m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber>>1)
				{
					g_stHearthRotationState.bReversal = true;
				}
				else
				{
					g_stHearthRotationState.bReversal = false;
				}
			}
		}
		else
		{
			//AfxMessageBox(TEXT("Error: plc连接有问题"));
			AfxMessageBox(g_item3_String[_ITEM3_STR_PLC_COMM_ERR].IsEmpty()?
				TEXT("出错：plc通信不正常！\r\n plc communication failed."):g_item3_String[_ITEM3_STR_PLC_COMM_ERR]);

			return false;
		}		
		// >>设置转动目标，并发送模拟触摸屏的转动命令.
		//debugging,仅针对 点坩埚.
		ASSERT(g_stHearthRotationState.iTargetPos<=m_OAHearth[g_stHearthRotationState.iCurHearth].iCrucibleMaxNumber);
		g_pRotateThread->fn_SetCruciblePosPT(g_stHearthRotationState.iCurHearth, g_stHearthRotationState.iTargetPos);
		Sleep(200);
		//if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[g_stHearthRotationState.iCurHearth].iHearthType)
		g_pRotateThread->fn_RunCruciblePT(g_stHearthRotationState.iCurHearth);

		//填写起始坩埚位置.08.01.05
		str.Format(TEXT("%2d"),g_stHearthRotationState.iStartPos );
		m_ListctrlCrucible.SetItemText(g_stHearthRotationState.iCurListItem , _ROTATOR_CRUCIBLE_LIST_COL_START,str);

	}
	else
	{//环形坩埚,2007.12.24
		//设置目标旋转速度
		g_pRotateThread->fn_SetHearthSpeed(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.iTargetSpeed);

		//添加目标速度, 2007.12.29
		m_OAHearth[g_stHearthRotationState.iCurHearth].iSpeed = g_stHearthRotationState.iTargetSpeed;

		//开始旋转. g_stHearthRotationState.bAnnularReversal 提供高级界面供用户设置
		//g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.bAnnularPositive);
		g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,gb_AnnularCW[g_stHearthRotationState.iCurHearth]);
		Sleep(400);
		//确保已处于运转状态
		bool bRunning = false;
		g_pRotateThread->fn_GetHearthRunState(g_stHearthRotationState.iCurHearth,bRunning,true);
		if(!bRunning)
		{//如果没有运转，则再发送一次开始旋转命令
			//g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,g_stHearthRotationState.bAnnularPositive);
			g_pRotateThread->fn_RunAnnularHearthPT(g_stHearthRotationState.iCurHearth,gb_AnnularCW[g_stHearthRotationState.iCurHearth]);
			Sleep(400);
			g_pRotateThread->fn_GetHearthRunState(g_stHearthRotationState.iCurHearth,bRunning,true);
			if(!bRunning)
			{//仍然处于停止状态，则报错并退出。
				//AfxMessageBox(TEXT("Error: 环形坩埚没有按预期开始旋转！"));
				AfxMessageBox(g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_NOT_ROTAT].IsEmpty()?
				TEXT("错误: 环形坩埚没有按预期开始旋转！\r\n Annualr hearth start rotation failed."):g_item3_String[_ITEM3_STR_ANNULAR_HEARTH_NOT_ROTAT]);
				return false;
			}
		}

		g_stHearthRotationState.bAnnularBeginingWaiting  = true; //进入初始等待状态
		g_stHearthRotationState.bAnnularIntervalWaiting  = false;
		g_stHearthRotationState.dwStartingWaitingTick = GetTickCount();
	}
	//开始计时
	g_stHearthRotationState.dwStartTime = GetTickCount();

	//编码器状态, 2008.01,03
	g_stHearthRotationState.iCoderChangeCounts = 0;	
	g_stHearthRotationState.iCoderNotChangeTime = 0;
	g_stHearthRotationState.dwCoderNotChangeStartTickTime = g_stHearthRotationState.dwStartTime; 
	g_stHearthRotationState.iCrucibleChangeCounts = 0;

	// >> 发送消息至线程，令其进入循环！
		// TODO: Add your control notification handler code here
	EnterCriticalSection(&gCriticalSection_HearthState);
		g_bTestHearthRotation = true;
	LeaveCriticalSection(&gCriticalSection_HearthState);
	g_pRotateThread->PostThreadMessage(WM_ITEM3_HEARTH_ROTATE_MONITOR,0,g_stHearthRotationState.iCurHearth);
	fn_ShowWindow();
	return true;
}

void COARotatorView::OnBnClickedBtnCreateHearth1()
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(_OA_HEARTH_TYPE_CRUCIBLE==m_OAHearth[0].iHearthType? IDC_COMBO_HEARTH_12:IDC_COMBO_HEARTH_1,str);
	//GetDlgItemText(IDC_COMBO_HEARTH_1,str);
	int iStartPos = abs(_ttoi(str));
	fn_CreateHearthTestOrder(0,iStartPos);
}

void COARotatorView::OnBnClickedBtnCreateHearth2()
{
	CString str;
	GetDlgItemText(_OA_HEARTH_TYPE_CRUCIBLE==m_OAHearth[1].iHearthType? IDC_COMBO_HEARTH_22:IDC_COMBO_HEARTH_2,str);
	int iStartPos = abs(_ttoi(str));
//	ASSERT(iStartPos>0 && iStartPos<100);
	fn_CreateHearthTestOrder(1,iStartPos);
}

void COARotatorView::fn_CreateHearthTestOrder(int iHearth, int iStartPos)
{
	CString str;
	ASSERT(m_bInit);
	ASSERT( _OA_HEARTH_TYPE_NONE != m_OAHearth[iHearth].iHearthType);
	int iItem = m_ListctrlCrucible.GetItemCount();
	if( _OA_HEARTH_TYPE_ANNULAR == m_OAHearth[iHearth].iHearthType )
	{
		if(_HEARTH_MONITOR_TYPE_OMRON==m_OAHearth[iHearth].iHearthMonitorType)
		{//2008.07.15
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("300"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("250"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("200"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("150"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("100"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("50"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("20"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("10"));
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("5"));
		}
		else
		{
			//900~100, 50. 环形坩埚， 速度 (0~999)
			for( int i = 900;i>0;i-=100)
			{
				str.Format(TEXT("%d"),i);
				m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
				m_ListctrlCrucible.SetItemText(iItem++,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,str);
			}
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,TEXT("50"));
		}
	}
	else if( _OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[iHearth].iHearthType )
	{//点坩埚，目标坩埚号

		//08.01.03
		int iCurPos = iStartPos;
		for(int i=0;i<m_OAHearth[iHearth].iCrucibleMaxNumber;i++)
		{
			iCurPos += i;
			iCurPos = iCurPos%m_OAHearth[iHearth].iCrucibleMaxNumber;
			str.Format(TEXT("%2d"),iCurPos!=0?iCurPos:m_OAHearth[iHearth].iCrucibleMaxNumber);			
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,str);
			iItem++;
		}
		/*
		int i;
		int iHalfNumber = m_OAHearth[iHearth].iCrucibleMaxNumber >> 1;  // /2
		
		for( i = 0; i<= iHalfNumber; i++)
		{
			iCurPos += i;
			iCurPos = iCurPos%m_OAHearth[iHearth].iCrucibleMaxNumber;
			str.Format(TEXT("%2d"),iCurPos!=0?iCurPos:m_OAHearth[iHearth].iCrucibleMaxNumber);			
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,str);
			iItem++;
		}

		for( i = iHalfNumber;i>0;i--)
		{
			iCurPos += m_OAHearth[iHearth].iCrucibleMaxNumber;
			iCurPos -= i;
			iCurPos = iCurPos%m_OAHearth[iHearth].iCrucibleMaxNumber;
			str.Format(TEXT("%2d"),iCurPos!=0?iCurPos:m_OAHearth[iHearth].iCrucibleMaxNumber);
			m_ListctrlCrucible.InsertItem(iItem,0==iHearth?TEXT("1"):TEXT("2"));
			m_ListctrlCrucible.SetItemText(iItem,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE,str);
			iItem++;
		}
		*/
	}
}

BOOL COARotatorView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message==WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_DELETE)
		{
			if(GetFocus()->GetSafeHwnd() == m_ListctrlCrucible.GetSafeHwnd())
			{
				if(!g_bTestHearthRotation)//测试过程中不允许删除.2008.01.05
					OnBnClickedBtnDeleteHearthRow();
				return false;
			}
		}
	}
	return CFormView::PreTranslateMessage(pMsg);
}



void COARotatorView::fn_SetIDsText(void)
{
	for(int i=0;i<gc_item3_Text_ID_CNT;i++)
	{
		if(!g_item3_ID_Text[i].IsEmpty())
			SetDlgItemText(g_item3_ID[i],g_item3_ID_Text[i]);
	}
}

int COARotatorView::fn_SaveHearthList(LPCTSTR pszFileName)
{
	TCHAR  lptchTmp[512];  //不可重入
	TCHAR cbuf[512];

	CFile fFile;
	if(!fFile.Open(pszFileName,CFile::modeCreate|CFile::modeWrite))
	{
		return 0;
	}

	CString str;

	fFile.SeekToBegin();
	//Save unicode file head
#ifdef UNICODE
	cbuf[0] = 0xFEFF; //FF在低 FE在高
	fFile.Write(cbuf,1*sizeof(TCHAR)); 
#endif

	TCHAR* p = TEXT("OPTORUN ASSITANT TEST FILE\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));

	p = TEXT("FILE TYPE: HEARTH ROTATION\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));


	int iLength;
	iLength = _stprintf(lptchTmp,TEXT("PROGRAM VERSION: %.3f\x0d\x0a"),g_dDataVersion);
	if(iLength)
		fFile.Write( lptchTmp,iLength * sizeof(TCHAR) );

	MultiByteToWideChar(CP_UTF8,0,__DATE__,strlen(__DATE__) + 1,cbuf,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
	iLength = _stprintf(lptchTmp,TEXT("PROGRAM DATE: %s\x0d\x0a"),cbuf);
	if(iLength)
		fFile.Write( lptchTmp,iLength * sizeof(TCHAR) );

	iLength = _stprintf(lptchTmp,TEXT("PROGRAM DATA VERSION: %.2f\x0d\x0a"),g_dProgramVersion);
	if(iLength)
		fFile.Write(lptchTmp, iLength * sizeof(TCHAR));

	//文件保存时刻
	DWORD bufsize = 256;
	::GetComputerName(cbuf,&bufsize);
	iLength = wsprintf(lptchTmp,TEXT("%s%s\x0d\x0a"),TEXT("Computer Name: "),cbuf);
	fFile.Write(lptchTmp,iLength*sizeof(TCHAR));

	SYSTEMTIME   st;   
	GetLocalTime(&st); 
	str.Format(TEXT("Save Time: %4d-%02d-%02d %02d:%02d:%02d\x0d\x0a"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fFile.Write(str,sizeof(TCHAR) * str.GetLength()); 

	//坩埚类型
	iLength = _stprintf(lptchTmp,TEXT("Hearth 1 TYPE: %s\x0d\x0a"),_OA_HEARTH_TYPE_NONE == m_OAHearth[0].iHearthType ? TEXT("No Hearth") :(_OA_HEARTH_TYPE_ANNULAR==m_OAHearth[0].iHearthType?TEXT("Annular"):TEXT("Cup")) );
	if(iLength)
		fFile.Write(lptchTmp, iLength * sizeof(TCHAR));
	//环形坩埚的旋转方向,2008.07.14
	if(_OA_HEARTH_TYPE_ANNULAR==m_OAHearth[0].iHearthType)
	{
		iLength = _stprintf(lptchTmp,TEXT("Hearth 1 Rotate Direction: %s\x0d\x0a"),gb_AnnularCW[0]==true?TEXT("CW"):TEXT("CCW") );
		if(iLength)
			fFile.Write(lptchTmp, iLength * sizeof(TCHAR));
	}

	iLength = _stprintf(lptchTmp,TEXT("Hearth 2 TYPE: %s\x0d\x0a"),_OA_HEARTH_TYPE_NONE == m_OAHearth[1].iHearthType ? TEXT("No Hearth") :(_OA_HEARTH_TYPE_ANNULAR==m_OAHearth[1].iHearthType?TEXT("Annular"):TEXT("Cup")) );
	if(iLength)
		fFile.Write(lptchTmp, iLength * sizeof(TCHAR));
	//环形坩埚的旋转方向,2008.07.14
	if(_OA_HEARTH_TYPE_ANNULAR==m_OAHearth[1].iHearthType)
	{
		iLength = _stprintf(lptchTmp,TEXT("Hearth 2 Rotate Direction: %s\x0d\x0a"),gb_AnnularCW[1]==true?TEXT("CW"):TEXT("CCW") );
		if(iLength)
			fFile.Write(lptchTmp, iLength * sizeof(TCHAR));
	}

	//数据起始标志
	p=TEXT("<DATA>\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));


	HDITEM hdi;
	CHeaderCtrl* pmyHeaderCtrl = m_ListctrlCrucible.GetHeaderCtrl();

	//列表头
	str.Empty();
	hdi.mask = HDI_TEXT;
	hdi.pszText = cbuf;
	hdi.cchTextMax = 256;
	for(int i = 0 ; i< pmyHeaderCtrl->GetItemCount();i++)
	{
		pmyHeaderCtrl->GetItem(i,&hdi);
		str += hdi.pszText;
		str += TEXT('\t');
	}
	str += TEXT("\x0d\x0a");
	fFile.Write(str, str.GetLength() * sizeof(TCHAR));

	//列表内容
	int iCnt = m_ListctrlCrucible.GetItemCount();
	for(int i=0;i<iCnt;i++)
	{
		iLength = _stprintf(lptchTmp,TEXT("%s\t%s\t%s\t%s\t%s\t%s\r\n"),m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_NO),
			m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_CRUCIBLE),
			m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_START),
			m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_END),
			m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME),
			m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_RESULT));
		fFile.Write(lptchTmp,iLength*sizeof(TCHAR));
	}

	p=TEXT("</DATA>\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));
	return 1;
}

int COARotatorView::fn_SaveDomeList(LPCTSTR pszFileName)
{
	TCHAR  lptchTmp[512];  //不可重入
	TCHAR cbuf[512];

	CFile fFile;
	if(!fFile.Open(pszFileName,CFile::modeCreate|CFile::modeWrite))
	{
		return 0;
	}

	CString str;

	fFile.SeekToBegin();
	//Save unicode file head
#ifdef UNICODE
	cbuf[0] = 0xFEFF; //FF在低 FE在高
	fFile.Write(cbuf,1*sizeof(TCHAR)); 
#endif

	TCHAR* p = TEXT("OPTORUN ASSITANT TEST FILE\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));

	p = TEXT("FILE TYPE: DOME ROTATION\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));

	int iLength;
	iLength = _stprintf(lptchTmp,TEXT("PROGRAM VERSION: %.3f\x0d\x0a"),g_dDataVersion);
	if(iLength)
		fFile.Write( lptchTmp,iLength * sizeof(TCHAR) );

	MultiByteToWideChar(CP_UTF8,0,__DATE__,strlen(__DATE__) + 1,cbuf,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
	iLength = _stprintf(lptchTmp,TEXT("PROGRAM DATE: %s\x0d\x0a"),cbuf);
	if(iLength)
		fFile.Write( lptchTmp,iLength * sizeof(TCHAR) );

	iLength = _stprintf(lptchTmp,TEXT("PROGRAM DATA VERSION: %.3f\x0d\x0a"),g_dProgramVersion);
	if(iLength)
		fFile.Write(lptchTmp, iLength * sizeof(TCHAR));

	//文件保存时刻
	DWORD bufsize = 256;
	::GetComputerName(cbuf,&bufsize);
	iLength = wsprintf(lptchTmp,TEXT("%s%s\x0d\x0a"),TEXT("Computer Name: "),cbuf);
	fFile.Write(lptchTmp,iLength*sizeof(TCHAR));

	SYSTEMTIME   st;   
	GetLocalTime(&st); 
	str.Format(TEXT("Save Time: %4d-%02d-%02d %02d:%02d:%02d\x0d\x0a"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fFile.Write(str,sizeof(TCHAR) * str.GetLength()); 

	//数据起始标志
	p=TEXT("<DATA>\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));

	HDITEM hdi;
	CHeaderCtrl* pmyHeaderCtrl = m_ListctrlDome.GetHeaderCtrl();

	//列表头
	str.Empty();
	hdi.mask = HDI_TEXT;
	hdi.pszText = cbuf;
	hdi.cchTextMax = 256;
	for(int i = 0 ; i< pmyHeaderCtrl->GetItemCount();i++)
	{
		pmyHeaderCtrl->GetItem(i,&hdi);
		str += hdi.pszText;
		str += TEXT('\t');
	}
	str += TEXT("\x0d\x0a");
	fFile.Write(str, str.GetLength() * sizeof(TCHAR));

	//列表内容
	int iCnt = m_ListctrlDome.GetItemCount();
	for(int i=0;i<iCnt;i++)
	{
		iLength = _stprintf(lptchTmp,TEXT("%s\t%s\t%s\t%s\t%s\t%s\t%s\r\n"),m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_SPEED),
			m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_START_CNT),
			m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_END_CNT),
			m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_WAITING_TIME),
			m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_TEST_TIME),
			m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION),
			m_ListctrlDome.GetItemText(i,_ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE));
		fFile.Write(lptchTmp,iLength*sizeof(TCHAR));
	}

	p=TEXT("</DATA>\x0d\x0a");
	fFile.Write(p,_tcslen(p)*sizeof(TCHAR));
	return 1;
}

// 自动生成文件名，并保存当前Hearth列表内容，供测量停止时自动保存
int COARotatorView::fn_AutoSaveHearthList(void)
{
	TCHAR  lptchTmp[512];  //不可重入
	TCHAR cbuf[512];
	DWORD bufsize = 512;
	
	//Path
	::GetModuleFileName(NULL,lptchTmp,512);
	CString str(lptchTmp);
	int ibackslash = str.ReverseFind(TEXT('\\'));
	lptchTmp[ibackslash + 1]= TEXT('\0');

	//fileName
	::GetComputerName(cbuf,&bufsize);
	SYSTEMTIME   st;   
	GetLocalTime(&st);   
	str.Format(TEXT("-OA-HEARTH-%4d%02d%02d %02d%02d%02d.log"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	_tcscat(cbuf,str); //computer name + time = filename

	_tcscat(lptchTmp,cbuf); // path + filename.

	return fn_SaveHearthList(lptchTmp);
}
int COARotatorView::fn_AutoSaveDomeList(void)
{
	TCHAR  lptchTmp[512];  //不可重入
	TCHAR cbuf[512];
	DWORD bufsize = 512;
	
	//Path
	::GetModuleFileName(NULL,lptchTmp,512);
	CString str(lptchTmp);
	int ibackslash = str.ReverseFind(TEXT('\\'));
	lptchTmp[ibackslash + 1]= TEXT('\0');

	//fileName
	::GetComputerName(cbuf,&bufsize);
	SYSTEMTIME   st;   
	GetLocalTime(&st);   
	str.Format(TEXT("-OA-Dome-%4d%02d%02d %02d%02d%02d.log"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	_tcscat(cbuf,str); //computer name + time = filename

	_tcscat(lptchTmp,cbuf); // path + filename.

	return fn_SaveDomeList(lptchTmp);
}

// 求线性回归方程：Y = a + bx
// data[rows*2]数组：X, Y；rows：数据行数；a, b：返回回归系数. 连续x,y方式存储的二维数据
// SquarePoor[4]：返回方差分析指标: 回归平方和，剩余平方和，回归平方差，剩余平方差
// 返回值：0求解成功，-1错误
int LinearRegression(double *data, int rows, double *a, double *b, double *SquarePoor=NULL)
{
    int m;
    double *p, Lxx = 0.0, Lxy = 0.0, xa = 0.0, ya = 0.0;
    if (data == 0 || a == 0 || b == 0 || rows < 1)
        return -1;
    for (p = data, m = 0; m < rows; m ++)
    {
        xa += *p ++;
        ya += *p ++;
    }
    xa /= rows;                                     // X平均值
    ya /= rows;                                     // Y平均值
    for (p = data, m = 0; m < rows; m ++, p += 2)
    {
        Lxx += ((*p - xa) * (*p - xa));             // Lxx = Sum((X - Xa)平方)
        Lxy += ((*p - xa) * (*(p + 1) - ya));       // Lxy = Sum((X - Xa)(Y - Ya))
    }
    *b = Lxy / Lxx;                                 // b = Lxy / Lxx
    *a = ya - *b * xa;                              // a = Ya - b*Xa
    if (SquarePoor == 0)
        return 0;
    // 方差分析
    SquarePoor[0] = SquarePoor[1] = 0.0;
    for (p = data, m = 0; m < rows; m ++, p ++)
    {
        Lxy = *a + *b * *p ++;
        SquarePoor[0] += ((Lxy - ya) * (Lxy - ya)); // U(回归平方和)
        SquarePoor[1] += ((*p - Lxy) * (*p - Lxy)); // Q(剩余平方和)
    }
    SquarePoor[2] = SquarePoor[0];                  // 回归方差
    SquarePoor[3] = SquarePoor[1] / (rows - 2);     // 剩余方差
    return 0;
}

/*

*/
void COARotatorView::OnBnClickedBtnStatisticsEvaluation()
{
	// TODO: Add your control notification handler code here
	int nCount = m_ListctrlCrucible.GetItemCount();
	if( !m_bInit || nCount==0 )
		return;

	//只有点坩埚数据才可以进行线形回归统计
	if(_OA_HEARTH_TYPE_CRUCIBLE != m_OAHearth[0].iHearthType && _OA_HEARTH_TYPE_CRUCIBLE != m_OAHearth[1].iHearthType)
		return;

	int i;
	int iTemp;
	double dTemp;
	CString str,strTemp;
	int iHearth1PositiveDataNumber = 0; //正转
	int iHearth1NegativeDataNumber = 0; //反转
	int iHearth2PositiveDataNumber = 0; //正转
	int iHearth2NegativeDataNumber = 0; //反转
	
	double *pH1PositiveData = NULL;
	double *pH1NegativeData = NULL;

	double *pH2PositiveData = NULL;
	double *pH2NegativeData = NULL;

	//多分配点空间
	if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[0].iHearthType)
	{
		pH1PositiveData = new double[nCount<<1];
		pH1NegativeData = new double[nCount<<1];

		ASSERT(pH1PositiveData && pH1NegativeData);
		if(!pH1PositiveData || !pH1NegativeData)
			goto label_Hearth_Statistics_Return;
	}

	if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[1].iHearthType)
	{
		pH2PositiveData = new double[nCount<<1];
		pH2NegativeData = new double[nCount<<1];

		ASSERT(pH2PositiveData && pH2NegativeData);
		if(!pH2PositiveData || !pH2NegativeData)
			goto label_Hearth_Statistics_Return;
	}

	//合计数据表格中可能的数据.
	//CString str;
	for( i=0; i<nCount; i++ )
	{
		iTemp = _ttoi(m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_NO));
		if( iTemp == 1 )
		{//Hearth1
			if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[0].iHearthType)
			{
				dTemp = _tstof(m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME));
				if(dTemp>0.1) 
				{//有测试时间(测试时间 通常在 1秒以上)，说明有数据
					iTemp = _ttoi(m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_RESULT));
					if(iTemp>0)
					{
						//正转经历的坩埚数
						pH1PositiveData[ (iHearth1PositiveDataNumber<<1) ] = iTemp;   //X 坩埚数
						pH1PositiveData[ (iHearth1PositiveDataNumber<<1) +1 ] = dTemp; //Y 时间
						iHearth1PositiveDataNumber++;
					}
					else if(iTemp<0)
					{   //反转经历的坩埚数，因为在填写数据的时候，反转的坩埚数用 负号表示
						pH1NegativeData[ (iHearth1NegativeDataNumber<<1) ] = -iTemp;   //X 坩埚数
						pH1NegativeData[ (iHearth1NegativeDataNumber<<1) +1 ] = dTemp; //Y 时间
						iHearth1NegativeDataNumber++;
					}
				}			
			}
		}
		else if( iTemp == 2 )
		{//Hearth2
			if(_OA_HEARTH_TYPE_CRUCIBLE == m_OAHearth[1].iHearthType)
			{
				dTemp = _tstof(m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_TEST_TIME));
				if(dTemp>0.1) 
				{//有测试时间(测试时间 通常在 1秒以上)，说明有数据
					iTemp = _ttoi(m_ListctrlCrucible.GetItemText(i,_ROTATOR_CRUCIBLE_LIST_COL_RESULT));
					if(iTemp>0)
					{
						//正转经历的坩埚数
						pH2PositiveData[ (iHearth2PositiveDataNumber<<1) ] = iTemp;   //X 坩埚数
						pH2PositiveData[ (iHearth2PositiveDataNumber<<1) +1 ] = dTemp; //Y 时间
						iHearth2PositiveDataNumber++;
					}
					else if(iTemp<0)
					{   //反转经历的坩埚数，因为在填写数据的时候，反转的坩埚数用 负号表示
						pH2NegativeData[ (iHearth2NegativeDataNumber<<1) ] = -iTemp;   //X 坩埚数
						pH2NegativeData[ (iHearth2NegativeDataNumber<<1) +1 ] = dTemp; //Y 时间
						iHearth2NegativeDataNumber++;
					}
				}			
			}
		}
	}

	double a[4];  //a 对应 1+ 1- 2+ 2-
	double b[4];  //b 对应 1+ 1- 2+ 2-
	int    c[4];  //对应的回归函数返回值
	memset( a, 0, sizeof(double) * 4 );
	memset( b, 0, sizeof(double) * 4 );
	memset( c, 0, sizeof(int) * 4 );

	if( iHearth1PositiveDataNumber > 2)
	{ //2点以上才进行统计分析
		c[0] = LinearRegression(pH1PositiveData,iHearth1PositiveDataNumber,&a[0],&b[0]);
	}

	if( iHearth1NegativeDataNumber > 2)
	{ //2点以上才进行统计分析
		c[1] = LinearRegression(pH1NegativeData,iHearth1NegativeDataNumber,&a[1],&b[1]);
	}

	if( iHearth2PositiveDataNumber > 2)
	{ //2点以上才进行统计分析
		c[2] = LinearRegression(pH2PositiveData,iHearth2PositiveDataNumber,&a[2],&b[2]);
	}

	if( iHearth2NegativeDataNumber > 2)
	{ //2点以上才进行统计分析
		c[3] = LinearRegression(pH2NegativeData,iHearth2NegativeDataNumber,&a[3],&b[3]);
	}

	//报告 Hearth1 分析结果
	if(iHearth1PositiveDataNumber>2 && 0==c[0])
	{//Hearth1 正转 数据分析正确
		strTemp.Format(TEXT("Hearth1 clockwise rotation\r\n\t<beginning and ending time>: %.1f seconds.\r\n\t<each cup>: %.1f seconds.\r\n"),a[0],b[0]);
		str += strTemp;
	}

	if(iHearth1NegativeDataNumber>2 && 0==c[1])
	{//Hearth1 反转 数据分析正确
		if( iHearth1PositiveDataNumber>2 && 0==c[0] )
		{ //Hearth1 正转 数据已有
			strTemp.Format(TEXT("Hearth1 counter clockwise rotation\r\n\t<beginning and ending time>: %.1f seconds.\r\n\t\tgreater than clockwise: %.1f.\r\n\t<each cup>: %.1f seconds.\r\n"),a[1],a[1]-a[0],b[1]);
		}
		else
		{//没有反转数据
			strTemp.Format(TEXT("Hearth1 counter clockwise rotation\r\n\t<beginning and ending time>: %.1f seconds.\r\n\t<each cup>: %.1f seconds.\r\n"),a[1],b[1]);
		}
		str += strTemp;
	}


	//报告 Hearth2 分析结果
	if(iHearth2PositiveDataNumber>2 && 0==c[2])
	{//Hearth2 正转 数据分析正确
		strTemp.Format(TEXT("Hearth2 clockwise rotation\r\n\t<beginning and ending time>: %.1f seconds.\r\n\t<each cup>: %.1f seconds.\r\n"),a[2],b[2]);
		str += strTemp;
	}
	if(iHearth2NegativeDataNumber>2 && 0==c[3])
	{//Hearth2 反转 数据分析正确
		if( iHearth2PositiveDataNumber>2 && 0==c[2] )
		{ //Hearth1 正转 数据已有
			strTemp.Format(TEXT("Hearth2 counter clockwise rotation\r\n\t<beginning and ending time>: %.1f seconds.\r\n\t\tgreater than clockwise: %.1f.\r\n\t<each cup>: %.1f seconds."),a[3],a[3]-a[2],b[3]);
		}
		else
		{//没有反转数据
			strTemp.Format(TEXT("Hearth2 counter clockwise rotation\r\n\t<beginning and ending time>: %.1f seconds.\r\n\t<each cup>: %.1f seconds."),a[3],b[3]);
		}
		str += strTemp;
	}


	//报告 分析 结果
	if(!str.IsEmpty())
		AfxMessageBox(str);

label_Hearth_Statistics_Return:
	//释放空间
	if(pH1PositiveData)
		delete [] pH1PositiveData;
	if(pH1NegativeData)
		delete [] pH1NegativeData;
	if(pH2PositiveData)
		delete [] pH1PositiveData;
	if(pH2NegativeData)
		delete [] pH1NegativeData;
}

void COARotatorView::OnBnClickedBtnAutocreateDomeSpeed()
{
	int iItem = m_ListctrlDome.GetItemCount();
	m_ListctrlDome.InsertItem(iItem++,TEXT("10"));
	m_ListctrlDome.InsertItem(iItem++,TEXT("15"));
	m_ListctrlDome.InsertItem(iItem++,TEXT("20"));
	m_ListctrlDome.InsertItem(iItem++,TEXT("25"));
	m_ListctrlDome.InsertItem(iItem++,TEXT("30"));
}

void COARotatorView::OnBnClickedBtnInsertDomeSpeed()
{
	CString str;	
	GetDlgItemText(IDC_COMBO_DOME_SPEED,str);
	int iSpeed = _ttoi(str);
	str.Format(TEXT("%d"),iSpeed);

	if( iSpeed>30 && iSpeed<=60 )
	{
		static bool bSure=false; //确认一次，第二次以后就开始默认可以.
		if(IDYES != AfxMessageBox(TEXT("Speed %s is greater than 30, Are you sure?"),MB_YESNO | MB_ICONQUESTION|(bSure?MB_DEFBUTTON1:MB_DEFBUTTON2)))
		{return;}
		bSure = true;
	}

	if( iSpeed >= 10 && iSpeed <= 60)
	{
		int iItems;
		POSITION pos = m_ListctrlDome.GetFirstSelectedItemPosition();

		if(pos)
		{//在当前选择项之前插入
			iItems = m_ListctrlDome.GetNextSelectedItem(pos);

			bool bTest = false;
			EnterCriticalSection(&gCriticalSection_DomeState);
				bTest = g_bTestDomeRotation;
			LeaveCriticalSection(&gCriticalSection_DomeState);
			if(bTest)
			{//测量过程中，不允许在前面插入测试项目.只能向后插入.
				if(iItems<=g_stDomeRotationState.iCurListItem)
					return;
			}

		}
		else
		{//没有选择项，则插入到最后一行
			iItems = m_ListctrlDome.GetItemCount();
		}
		m_ListctrlDome.InsertItem(iItems,str);
	}
	else if( iSpeed>60)
	{
		AfxMessageBox(TEXT("Speed should less than 60."));
	}
}

//删除当前选择的第一项
void COARotatorView::OnBnClickedBtnDeleteDomeRow()
{
		POSITION pos = m_ListctrlDome.GetFirstSelectedItemPosition();
		if(pos)
		{//在当前选择项之前插入
			int iItems = m_ListctrlDome.GetNextSelectedItem(pos);

			bool bTest = false;
			EnterCriticalSection(&gCriticalSection_DomeState);
			bTest = g_bTestDomeRotation;
			LeaveCriticalSection(&gCriticalSection_DomeState);
			if(bTest)
			{//测量过程中，不允许删除当前以前的 item行
				if(iItems<=g_stDomeRotationState.iCurListItem)
					return;
			}

			m_ListctrlDome.DeleteItem(iItems);
			if( iItems < m_ListctrlDome.GetItemCount())
			{
				m_ListctrlDome.SetItemState(iItems,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
				m_ListctrlDome.EnsureVisible(iItems,false);
			}
		}
}

void COARotatorView::OnBnClickedBtnSaveDomeList()
{
	TCHAR  lptchTmp[512];  //不可重入
	TCHAR cbuf[512];
	DWORD bufsize = 512;
	
	int iCnt = m_ListctrlDome.GetItemCount();
	if(iCnt==0)
	{	
		SYSTEMTIME   st;   
		CString str;
		GetLocalTime(&st);   
		str.Format(TEXT("Error: no data! \r\n%4d-%02d-%02d %02d:%02d:%02d\r\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		AfxMessageBox(str);
		return ; 
	}

	::GetModuleFileName(NULL,lptchTmp,512);
	CString str(lptchTmp);
	int ibackslash = str.ReverseFind(TEXT('\\'));
	lptchTmp[ibackslash + 1]= TEXT('\0');

	bufsize = 512-2;
	::GetComputerName(cbuf,&bufsize);

	SYSTEMTIME   st;   
	GetLocalTime(&st);   
	str.Format(TEXT("-OA-Dome-%4d%02d%02d %02d%02d%02d.log"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);

	_tcscat(cbuf,str);
	_tcscat(lptchTmp,cbuf);
		CFileDialog dlg(FALSE);
	dlg.m_ofn.lpstrTitle = _T("Save Dome Rotation Data into File");
	dlg.m_ofn.lpstrFilter = _T("Optorun Assitant log(*.log)\0*.log\0");
	dlg.m_ofn.lpstrInitialDir = lptchTmp; 
	dlg.m_ofn.lpstrFile = lptchTmp;

	if(dlg.DoModal()==IDOK)
	{
		str = dlg.GetPathName();
		if(-1==str.Find(_T(".log")))
		{str +=_T(".log");}

		fn_SaveDomeList(str);
	}
}

void COARotatorView::OnBnClickedBtnStartDomeTest()
{
	// >> 获取当前转动列表起始行. 以及待转动位置
	int iItem = 0;
	if( BST_CHECKED == ((CButton*)GetDlgItem(IDC_CHECK_START_FROM_SELECTION_DOME ))->GetCheck())
	{
		POSITION pos = m_ListctrlDome.GetFirstSelectedItemPosition();
		if(pos)
		{//在当前选择项之前插入
			iItem = m_ListctrlDome.GetNextSelectedItem(pos);
		}
	}

	//确认当前行小于总行数
	if(iItem >= m_ListctrlDome.GetItemCount() )
	{//
		return;
	}

	fn_SaveRestoreDomeConfigure(true);	//存储当前的参数.

	fn_StartDomeTest(iItem);
}

void COARotatorView::OnBnClickedBtnCancelDomeTest()
{
	bool bTest;
	EnterCriticalSection(&gCriticalSection_DomeState);
		bTest = g_bTestDomeRotation;
	LeaveCriticalSection(&gCriticalSection_DomeState);

	if(bTest)
	{
		if(IDYES == AfxMessageBox(g_item3_String[_ITEM3_STR_DOME_TEST_EXIT_Q].IsEmpty()?TEXT("正在测量Dome转速,你真的要停止测试?\r\nIt's testing Dome rotation, Are you sure to cancel?"):g_item3_String[_ITEM3_STR_DOME_TEST_EXIT_Q]
			,MB_YESNO | MB_ICONQUESTION|MB_DEFBUTTON2))
		{
			////需要能反映出的确被用户停止
			EnterCriticalSection(&gCriticalSection_DomeState);
			g_bTestDomeRotation = false; 
			g_bTestDomeStopByUser = true;
			LeaveCriticalSection(&gCriticalSection_DomeState);
			
			fn_SaveRestoreDomeConfigure(false);//恢复Dome状态.
			fn_ShowDomeWindow();
			return;

		}
	}
}

/****************************************************************
函数名称：fn_SaveRestoreDomeConfigure
函数说明：记录及恢复测试Dome前的状态。因为在测试中，将更改这些状态。
输入参数：bSave true: 将当前Dome状态保存进内存中
				false:恢复保存过的Dome状态
#define _ZZY_DOME_ROTATE_LOW	1
#define _ZZY_DOME_ROTATE_HIGH	0
返回值：暂不用.
测试中更改了高速状态下的Speed.
*****************************************************************/
bool COARotatorView::fn_SaveRestoreDomeConfigure(bool bSave)
{
	static bool bParaSaved = false;
	static int  iSpeed = 10;
	static int  iLow   = _ZZY_DOME_ROTATE_LOW;
	static int  iAUTO_OFF_MANUAL = _ZZY_DOME_SWITCH_OFF;
	static int  iHighSpeed = 30;//用于恢复 HighState 的速度值

	if(bSave)
	{
		g_pDomeRotateThread->fn_GetDomeHighSpeed(iHighSpeed);
		g_pDomeRotateThread->fn_GetDomeRotationState(iSpeed, iLow);
		g_pDomeRotateThread->fn_GetDomeRotationControl(iAUTO_OFF_MANUAL);
	}
	else
	{
		g_pDomeRotateThread->fn_SetDomeHighSpeed(iHighSpeed);
		g_pDomeRotateThread->fn_SetDomeRotationState(iSpeed, iLow);
		g_pDomeRotateThread->fn_ToggleDomeSwitch(iAUTO_OFF_MANUAL);
	}
	
	return true;
}


/*****************************************************************************
函数名称：fn_StartDomeTest
说明：从某个行号开始进行Dome检测
输出参数：int iItem，起始 测试的行号
******************************************************************************/
bool COARotatorView::fn_StartDomeTest(int iItem)
{
	
	if( iItem >= m_ListctrlDome.GetItemCount())
	{//防止出现意外 删除!
		EnterCriticalSection(&gCriticalSection_DomeState);
			g_bTestDomeRotation = false;
		LeaveCriticalSection(&gCriticalSection_DomeState);
		fn_ShowDomeWindow();
		return false;
	}

	g_stDomeRotationState.iCurListItem = iItem;
	//清除当前行已有内容 //以防是第二次测量.
	m_ListctrlDome.SetItemText(iItem,_ROTATOR_DOME_LIST_COL_START_CNT,TEXT(""));
	m_ListctrlDome.SetItemText(iItem,_ROTATOR_DOME_LIST_COL_END_CNT,TEXT(""));
	m_ListctrlDome.SetItemText(iItem,_ROTATOR_DOME_LIST_COL_WAITING_TIME,TEXT(""));
	m_ListctrlDome.SetItemText(iItem,_ROTATOR_DOME_LIST_COL_TEST_TIME,TEXT(""));	
	m_ListctrlDome.SetItemText(iItem,_ROTATOR_DOME_LIST_COL_SEC_PER_REVOLUTION,TEXT(""));
	m_ListctrlDome.SetItemText(iItem,_ROTATOR_DOME_LIST_COL_REVOLUTION_PER_MINUTE,TEXT(""));

	m_ListctrlDome.SetItemState(iItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	m_ListctrlDome.EnsureVisible(iItem,false);

	//获取当前栏的目标速度
	CString str = m_ListctrlDome.GetItemText(iItem,_ROTATOR_DOME_LIST_COL_SPEED);
	g_stDomeRotationState.iTargetSpeed = _ttoi(str);
	ASSERT(g_stDomeRotationState.iTargetSpeed< 51);

	g_stDomeRotationState.iRotationTime = 0;
	g_stDomeRotationState.bStop = false;

	//开始计时
	g_stDomeRotationState.dwStartTime = GetTickCount();

	// >> 发送消息至线程，令其进入循环！
	EnterCriticalSection(&gCriticalSection_DomeState);
		g_bTestDomeStopByUser = false;
		g_bTestDomeRotation = true;
	LeaveCriticalSection(&gCriticalSection_DomeState);
	g_pDomeRotateThread->PostThreadMessage(WM_ITEM3_DOME_ROTATE_MONITOR,0,(LPARAM)g_stDomeRotationState.iTargetSpeed);
	fn_ShowDomeWindow();
	return true;
}
void COARotatorView::fn_ShowDomeWindow(void)
{
	bool bTest = false;
	EnterCriticalSection(&gCriticalSection_DomeState);
	bTest = g_bTestDomeRotation;
	LeaveCriticalSection(&gCriticalSection_DomeState);

	GetDlgItem(IDC_BTN_AUTOCREATE_DOME_SPEED)->EnableWindow(!bTest);
	//GetDlgItem(IDC_COMBO_DOME_SPEED)->EnableWindow(bTest);	
	//GetDlgItem(IDC_BTN_INSERT_DOME_SPEED)->EnableWindow(bTest);
	//GetDlgItem(IDC_BTN_DELETE_DOME_ROW)->EnableWindow(!bTest);
	//GetDlgItem(IDC_BTN_SAVE_DOME_LIST)->EnableWindow(!bTest);
	GetDlgItem(IDC_BTN_START_DOME_TEST)->EnableWindow(!bTest);
	GetDlgItem(IDC_BTN_CANCEL_DOME_TEST)->EnableWindow(bTest);

}

/********************************************************************************
函数名称：fn_FigureDomeThreadData
函数说明：计算 序列g_stReadDomeThreadData平均间隔时间及推算出的 时间
输入参数：double& dAveSecPR，平均间隔时间
		  double& dFiguredSecPR,推算出的间隔时间
返回值：  int类型
			0:没有开始预测
			1:仅平均值可以用
			2:平均值与预测值都可以用，但此时尚不满足停止条件
			50:已经超过最大转动次数，强制停止.
			100:预测取得成功，已经满足了 预设条件，可以停止了。
********************************************************************************/
#define _XYZ_DOME_FIGURE_MIN_ROTATIONS	10   //开始分析的最小圈数
#define _XYZ_DOME_FIGURE_MAX_ROTATIONS	1000 //，最多这么多圈，即使仍不能成功分析出结果，也强制 返回 true;
#define _XYZ_DOME_ROTATION_SecPR_RESOLUTION 0.015
int COARotatorView::fn_FigureDomeThreadData( double& dAveSecPR, double& dFiguredSecPR)
{
	static double static_pdSecPR[_XYZ_DOME_FIGURE_MAX_ROTATIONS]; //单圈间隔时间。//记录每次间隔时间与圈数的比值, 数据量少一个. 
	static double static_pdSecPR_LineRegression[_XYZ_DOME_FIGURE_MAX_ROTATIONS];  //依次记录线性预测值
	
	//线性预测，省略 前5个数据，从第8个数据开始进行(至少3)。
	static double static_pd_b[_XYZ_DOME_FIGURE_MAX_ROTATIONS]; //用于存放线性预测结果 斜率 y = a + bx中的b.
	static double x_constant1[_XYZ_DOME_FIGURE_MAX_ROTATIONS]; //全1，用于线性预测
	static bool   x_b_constant1_Initialed = false;//是否初始化过
	if(!x_b_constant1_Initialed)
	{//只初始化一次
		for(int i=0;i<_XYZ_DOME_FIGURE_MAX_ROTATIONS;i++)
		{
			x_constant1[i] = i;
		}
		x_b_constant1_Initialed = true;
	}

	dAveSecPR = 0;
	dFiguredSecPR = 0;
	int iDomeDataSize = g_stReadDomeThreadData.size();
	if( iDomeDataSize <= 1 )
	{
		return 0;
	}
	else
	{
		const int ci_leaveoutcnt = 5;  //扣除前 x 组数据不参与计算。//5

		//数据保存时，已经满足每个记录之间的 转数不同.
		if( iDomeDataSize == 2)
		{
			dAveSecPR = (double) ((DWORD)(g_stReadDomeThreadData.back().dwTickTime-g_stReadDomeThreadData.front().dwTickTime))/((WORD)(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData.front().wDomeRotationCount));
			dAveSecPR *= 0.001; //ms->s
		}
		else if( iDomeDataSize < 10) 
		{////第一个数据间隔不能采用，因为起始位置不能保证
			dAveSecPR = (double) ((DWORD)(g_stReadDomeThreadData.back().dwTickTime-g_stReadDomeThreadData[1].dwTickTime))/((WORD)(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData[1].wDomeRotationCount));
			dAveSecPR *= 0.001; //ms->s
		}
		else //>10，从第5圈之后开始
		{
			dAveSecPR = (double) ((DWORD)(g_stReadDomeThreadData.back().dwTickTime-g_stReadDomeThreadData[ci_leaveoutcnt].dwTickTime))/((WORD)(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData[ci_leaveoutcnt].wDomeRotationCount));
			dAveSecPR *= 0.001; //ms->s
		}		

		//每次只需记录一个差值.
		static_pdSecPR[ iDomeDataSize - 1] =
			(double) ((DWORD)(g_stReadDomeThreadData.back().dwTickTime-g_stReadDomeThreadData[iDomeDataSize-2].dwTickTime))/((WORD)(g_stReadDomeThreadData.back().wDomeRotationCount-g_stReadDomeThreadData[iDomeDataSize-2].wDomeRotationCount));
		static_pdSecPR[ iDomeDataSize - 1] *= 0.001;//ms->s
		
		if(	iDomeDataSize > ci_leaveoutcnt+3)
		{
			COACnfgDoc::fn_LinearRegression(x_constant1[0],static_pd_b[iDomeDataSize-1],&x_constant1[ci_leaveoutcnt],&static_pdSecPR[ci_leaveoutcnt],iDomeDataSize-1-ci_leaveoutcnt);
			static_pdSecPR_LineRegression[iDomeDataSize-1] = x_constant1[0] + static_pd_b[iDomeDataSize-1];
			dFiguredSecPR = static_pdSecPR_LineRegression[ iDomeDataSize - 1];

			//超过规定圈数，再开始进行预估.
			if( iDomeDataSize>_XYZ_DOME_FIGURE_MIN_ROTATIONS && ((WORD)(g_stReadDomeThreadData.back().wDomeRotationCount - g_stReadDomeThreadData.front().wDomeRotationCount) > _XYZ_DOME_FIGURE_MIN_ROTATIONS) )
			{
				// 完成条件
				// 连续iEstimateDataNumber次线性预测之间相差不超过dEstimatedErr秒，并且 |斜率| < dEstimatedSlope ;
				const double dEstimatedErr = 0.005;
				const double dEstimatedSlope = 0.005;//直线斜率最大允差
				int iEstimateDataNumber = (iDomeDataSize > 40) ? iDomeDataSize/10 : 4; //自动以 1/10的数据进行估算.
				if(iEstimateDataNumber > 6)
					iEstimateDataNumber = 6;
				if( fn_Is_x_near( &static_pdSecPR_LineRegression[iDomeDataSize-iEstimateDataNumber-1] ,iEstimateDataNumber, dEstimatedErr) 
					&& fn_Is_x_near( &static_pd_b[iDomeDataSize-iEstimateDataNumber-1] ,iEstimateDataNumber, dEstimatedSlope) )
				{
					return 100;
				}
				else
				{
					//TRACE1("%3f\r\n",static_pd_b[iDomeDataSize-1]);
					return 2;
				}
			}

			if( iDomeDataSize > _XYZ_DOME_FIGURE_MAX_ROTATIONS)
			{
				dFiguredSecPR = dAveSecPR;
				return 50;
			}
			else 
				return 1;
		}
		else
		{
			return 1;
		}

	}
	return 0;
}
//判断连续 n 个x值是否绝对值相差在 d_epsilon之内
bool COARotatorView::fn_Is_x_near( double* x,int n,double d_epsilon)
{
	if(n<=2)
		return false;
	double dmax = x[0];
	double dmin = x[0];
	for(int i=1;i<n;i++)
	{
		if( x[i] > dmax)
		{
			dmax = x[i];
		}
		if( x[i] < dmin)
		{
			dmin = x[i];
		}
	}
	return ( ( dmax - dmin ) < d_epsilon ) ;
}

//排序后,取中间100项的平均值.
//估计值,偏差 

		
//			
//double dT0 = 0.001; ////平均每圈的时间，单位 s
//			int n1,n2,n3,n4; //1倍,2倍,3倍,4倍基准时间的个数.
/********************************************************************************************
函数名称：fn_CalcDomeRotationData
函数说明：计算Dome转动值
参数说明：double& dT0 , [output] Dome转动单圈平均时间，单位为 s
		  int& n1,      [output] 转动时间在平均时间附件的记录个数
		  int& n2,		[output] 转动时间在2倍平均时间附件的记录个数
		  int& n3,		[output] 转动时间在3倍平均时间附件的记录个数
		  double dStandValue,    平均时间的标准时间．　（即平均时间　~ 标准时间）
假定1:	　平均时间　约等于　给出的标准时间，二者绝对值应相差 < 0.1. 本测量目的是得到　平均时间的实测值.
假定2:    每次转动间隔时间　应分布在 标准时间的n倍　+/- 偏差值范围之内，这个偏差值同样是根据实际得出的估计值.
说明:	  扣除前　_ROTATE_DOME_NOT_COUNT　个数据不进行计算.
		  数据在　g_stReadDomeThreadData　之中			
********************************************************************************************/
int	COARotatorView::fn_CalcDomeRotationData(double& dT0, int& n1, int& n2,int& n3,int& n4, double dStandValue)
{
	const DWORD dwEpsilon = 500; //ms, 偏差值, 每次转动间隔　应分布在 标准时间的n倍　+/- 偏差值范围之内. 估计值
	n1=n2=n3=n4=0;
	int iDataSize = g_stReadDomeThreadData.size();
	DWORD dwStandValue = dStandValue * 1000; //转换成 DWORD值方便计算.
	DWORD  dwTotalTime = g_stReadDomeThreadData.back().dwTickTime - g_stReadDomeThreadData[_ROTATE_DOME_NOT_COUNT].dwTickTime;
	DWORD  dwTickInterval; //临时变量
	//限定范围
	DWORD  dw1Down = dwStandValue - dwEpsilon;
	DWORD  dw1Up   = dwStandValue + dwEpsilon;
	DWORD  dw2Down = 2*dwStandValue - dwEpsilon;
	DWORD  dw2Up   = 2*dwStandValue + dwEpsilon;
	DWORD  dw3Down = 3*dwStandValue - dwEpsilon;
	DWORD  dw3Up   = 3*dwStandValue + dwEpsilon;
	DWORD  dw4Down = 4*dwStandValue - dwEpsilon;
	DWORD  dw4Up   = 4*dwStandValue + dwEpsilon;

	for(int i=_ROTATE_DOME_NOT_COUNT; i<iDataSize; i++)
	{
		if( 1 == (g_stReadDomeThreadData[i].wDomeRotationCount - g_stReadDomeThreadData[i-1].wDomeRotationCount) )
		{
			dwTickInterval = g_stReadDomeThreadData[i].dwTickTime - g_stReadDomeThreadData[i-1].dwTickTime;
			if( dwTickInterval >= dw1Down && dwTickInterval <= dw1Up )
			{
				n1++;
			}
			else if( dwTickInterval >= dw2Down && dwTickInterval <= dw2Up )
			{
				n2++;
			}
			else if( dwTickInterval >= dw3Down && dwTickInterval <= dw3Up )
			{
				n3++;
			}
			else if( dwTickInterval >= dw4Down && dwTickInterval <= dw4Up )
			{
				n4++;
			}
			else
			{
				//扣除时间间隔不落在限定范围之内的时间
				dwTotalTime -= g_stReadDomeThreadData[i].dwTickTime - g_stReadDomeThreadData[i-1].dwTickTime;
			}

		}
		else
		{//扣除圈数不是间隔1的时间
			dwTotalTime -= g_stReadDomeThreadData[i].dwTickTime - g_stReadDomeThreadData[i-1].dwTickTime;
		}
	}

	dT0 =(double) dwTotalTime /(n1+2*n2+3*n3+4*n4);

	return 0;
}