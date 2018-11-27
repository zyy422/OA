#pragma once

#include "PlcRdWrThread.h"

extern char  vt_pszSendBuf[512]; //vacuum thread buffer
extern char  vt_pszRecvBuf[512];

//没有错误
#define _VACUUM_PROCESS_LAST_ERR_NONE         0 
//在规定时间内未等到 SingleObject
#define _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT  1 
//plc 通信出错或内容出错，需进一步检查
#define _VACUUM_PROCESS_LAST_ERR_COMM         2 

//等待object的时间, ms
#define _VACUUM_PROCESS_WAIT_OBJECT_TIME      1000

/***********************************2018-8-29 Zhou Yi(线程函数进行了多次的声明，暂时认为没有必要)********************************************************/
VOID CALLBACK gfn_VacuumTimerProc(
	HWND hwnd,     // handle of window for timer messages
	UINT uMsg,     // WM_TIMER message
	UINT idEvent,  // timer identifier
	DWORD dwTime   // current system time
);

// CVacuumProcessThread
class CVacuumProcessThread : public CPlcRdWrThread
{
	DECLARE_DYNCREATE(CVacuumProcessThread)

protected:
	CVacuumProcessThread();           // protected constructor used by dynamic creation
	virtual ~CVacuumProcessThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

public:
	HWND  m_hWnd; //待发送消息的窗口, 在线程启动时初始化
protected:
	afx_msg void OnVacuumThreadTimerEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnItem2VacuumThreadQA(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
/*
public:
	//写plc CIO 某一位 为1， 对应触摸屏动作
	bool SetPLCCIOBitEvent(WORD wAddress, WORD wBit);

	//写plc CIO
	bool WritePLCCIOEvent(WORD wAddress, WORD wValue);
	
	bool ReadPLCCIOEvent(WORD wAddress,WORD &wReadValue);
	bool ReadPLCDMEvent(WORD wAddress,WORD &wReadValue,bool bDec = true);
	
public:
	DWORD m_dwLastError;
	*/
public:
	// 取得 VGC023的真空度
	bool fn_GetVGCch3Pressure(double& dPressure);
public:
	// 检查主阀是否关闭
	bool fn_CheckMainValveStatus(bool& bOpen);
public:
	bool fn_SwitchMainValve(void);
public:
	bool fn_CheckExhaustMode(bool& bExhaustModeManual);
public:
	bool fn_SetExhaustMode(bool bManual);
public:
	// 查询是否扩散泵
	bool fn_IsDiffusionPump(bool& bDiffusionPump);
public:
	bool fn_IsIonGaugeRemoteControl(bool& bRemoteControl);
};



