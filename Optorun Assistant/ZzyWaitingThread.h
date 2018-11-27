#pragma once
// "ZzyWaitingThread.h"

// 用于较长时间等待.  时间并不精确, 准确度通常为秒级.
// 等待方法
// 主界面程序发送等待消息(置位bWaiting)， 本线程响应消息函数中等待指定时间，清除该变量(bWaiting)后退出。

// 主界面程序发送的消息应包为如下结构 _stZzyWaiting.

// 主界面程序 负责该结构对象的维护，包括初始化 CRITICAL_SECTION cs_ZzyWaiting。

// 主界面程序 应定时或在某个循环中等待该变量的清除

#define WM_ZZY_WAITING_THREAD_WORK (WM_USER + 400)

struct _stZzyWaiting{
	bool bWaitingStart;             //仅供主界面线程使用, 无需同步保护. 初始化 为false. 
	                                //需要等待时先 置成 true.

	bool bWaiting;                  //主界面线程设置 true, 本线程在等待时间达到后将其设置成 false. 需要同步保护.
	                                //主界面线程在设置后, 需有某种机制进行确认(循环/定时/消息响应等)
	
	bool bCancelWaiting;            //主界面线程发送消息时将其设置成 false, 中途如果需要 取消,则将其 置 true.
	                                //本线程只 负责读取, 需要进行同步保护
	
	CRITICAL_SECTION cs_ZzyWaiting; //同步保护对象。
	
	DWORD dwWaitingMilliSeconds;    //等待时间, 单位为ms. 可以不同步保护.

	DWORD dwStartWaitingTickCount;  //仅供主界面线程使用，记录发送消息时的 系统TickCount,可用于进行最长时间判断.
	                            
	                                //其实主线程可以仅以 dwStartWaitingTickCount，dwWaitingMilliSeconds,以及bWaiting即可进行时间判断.无需这么麻烦. 初始时bWaiting = false;
};

// CZzyWaitingThread
class CZzyWaitingThread : public CWinThread
{
	DECLARE_DYNCREATE(CZzyWaitingThread)

protected:
	CZzyWaitingThread();           // protected constructor used by dynamic creation
	virtual ~CZzyWaitingThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	afx_msg void OnZzyWaiting(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


