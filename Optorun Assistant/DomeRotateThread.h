#pragma once

#include "plcrdwrthread.h"

#define _ZZY_DOME_ROTATE_LOW	1
#define _ZZY_DOME_ROTATE_HIGH	0

#define _ZZY_DOME_SWITCH_AUTO   0
#define _ZZY_DOME_SWITCH_OFF    1
#define _ZZY_DOME_SWITCH_MANUAL 2

#define _ZZY_DOME_ROTATE_DATA_WPARAM_START		0 //启动阶段
#define _ZZY_DOME_ROTATE_DATA_WPARAM_READ		1 //数据读取阶段
// CDomeRotateThread //#include "DomeRotateThread"
/********************************************************
对Dome旋转进行计时而需要的线程。
本线程实际仅负责定时采集数据，发送给调度线程。
Low: 10-15    High:10-30, 也有的OTFC 10-50
High 下必须满足的两个条件：1,真空门关闭; 2,水晶冷却水打开。
AUTO 状态为成膜专用
OFF  状态下可通过按住门上的控制盒按钮来转动Dome
MANUAL 状态下转动. //High必须满足两个条件.
*********************************************************/

class CDomeRotateThread : public CPlcRdWrThread
{
	DECLARE_DYNCREATE(CDomeRotateThread)

protected:
	CDomeRotateThread();           // protected constructor used by dynamic creation
	virtual ~CDomeRotateThread();
public:
	HWND  m_hWnd; //待发送消息的窗口, 在线程启动时初始化
bool fn_ToggleDomeSwitch(int iAUTO_OFF_MANUAL);		   //AUTO, OFF, MANUAL: 转动
bool fn_GetDomeRotationControl(int &iAUTO_OFF_MANUAL); //获取当前Dome Toggle Switch状态
bool fn_SetDomeRotationState(int iSpeed, int iLow);    //设置当前LOW\HIGH转动情况及转速
bool fn_GetDomeRotationState(int& iSpeed, int& iLow);  //获取当前活动的转动情况 Low or high以及当前的转速.
bool fn_GetDomeHighSpeed(int& iSpeed);
bool fn_SetDomeHighSpeed(int iSpeed);
bool fn_IsDomeRotation(bool& bRunning);                //当前是否处于转动中，判断触摸屏上的
bool fn_IsDoorSwitchClosed(bool &bClosed);			   //门上的限位开关是否关闭， High下必须满足，需要人为锁上。 加提示
bool fn_ReadDomeRotationCounts(WORD& wRotationCnt);    //读取当前转动次数.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	afx_msg void OnDomeRotateMonitor(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


