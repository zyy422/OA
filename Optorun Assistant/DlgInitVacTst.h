#pragma once

#define _DLG_INIT_VAC_TST_RT_OVERTIME   0 //等候超时出错
#define _DLG_INIT_VAC_TST_RT_TIMERERR   1 //Timer　启动出错
#define _DLG_INIT_VAC_TST_RT_CANCEL     2 //用户取消
#define _DLG_INIT_VAC_TST_RT_COMM_ERR   3 //通信出错
#define _DLG_INIT_VAC_TST_RT_PRESS_ERR  4 //起始真空度没有达到，而且主阀没有打开．
#define _DLG_INIT_VAC_TST_RT_MANUAL_ERR 5 //10秒内仍未能进入 <手动排气>　模式
#define _DLG_INIT_VAC_TST_RT_MV_ERR     6 //真空度达到，程序关闭主阀，但主阀在１０秒钟内并没有关闭．

#define _DLG_INIT_VAC_TST_RT_OK         10

// CDlgInitVacTst dialog

class CDlgInitVacTst : public CDialog
{
	DECLARE_DYNAMIC(CDlgInitVacTst)

public:
	CDlgInitVacTst(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInitVacTst();

// Dialog Data
	enum { IDD = IDD_DIALOG_INIT_VACUUM_TEST };



	UINT m_unCurrentTimeEvent;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);

public:
	DWORD m_dwMaxWaitingTime; //最长等待时间（传递进来的值）,单位为 秒
	DWORD m_dwInitTick;       //初始化时的TickCount
	DWORD m_dwPressureKeepTime;  //达到起始真空度后的稳定时间，默然　５ 秒钟
	
	double  m_dStartPressure; //起始真空度值，低于即可．
	int     m_nRetValue;      //退出值．含义见 _DLG_INIT_VAC_TST_RT_OK等

	bool    m_bFirstTimer;
	
public:
//	afx_msg void OnClose();
};
