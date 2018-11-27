#pragma once
#include "plcrdwrthread.h"

class CRotatorThread :public CPlcRdWrThread
{
	DECLARE_DYNCREATE(CRotatorThread)
public:
	CRotatorThread(void);
public:
	virtual ~CRotatorThread(void);

public:
	HWND  m_hWnd; //待发送消息的窗口, 在线程启动时初始化
protected:
	afx_msg void OnHearthRotateMonitor(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	bool fn_GetHearthInfo(int iHearth, bool& bHearthExist, int& iCrucibleNumber);
public:
	bool fn_GetHearthCoderValue(int iHearth, int& iCoderVal);
public:
	bool fn_GetHearthCrucibleNumber(int iHearth, int& iCrucibleNumber);
public:
	// 初始化 Hearth 转动方式到 触摸屏，实际是清 0 某DM区域
	bool fn_InitHearthToPT(int iHearth);
public:
	bool fn_RunCruciblePT(int iHearth);
	bool fn_RunAnnularHearthPT(int iHearth,bool bPositiveRotation);
public:
	// 设定点坩埚位置（触摸屏上显示的位置）
	bool fn_SetCruciblePosPT(int iHearth, int iPos);

public:
	bool fn_GetHearthSpeed(int iHearth, int& iSpeed);
public:
	bool fn_SetHearthSpeed(int iHearth, int iSpeed);
public:
	bool fn_GetHearthRunState(int iHearth, bool& bRunning, bool bAnnular);
public:
	bool fn_SwitchAnnularHearthPT(int iHearth);
public:
	// 读取当前环形坩埚的控制状态,FIX or  ROTATE
	bool fn_GetAnnularHearthCntrl(int iHearth, int& iControlState);
public:
	bool fn_GetHearthMonitorType(int iHearth, int& iType);
};