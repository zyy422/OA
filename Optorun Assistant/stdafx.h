// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions


#include <afxdisp.h>        // MFC Automation classes



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//2007.09.12, by zhang ziye
#ifndef MAX_PAHT
#define MAX_PAHT 512
#endif

//抽速/漏率测量时的plc读取端口
#define _VACUUM_READ_PLC_ADDRESS_CNT            4
#define _VACUUM_READ_PLC_ADDRESS_SN_VGC         0
#define _VACUUM_READ_PLC_ADDRESS_SN_IONGAUGE    1
#define _VACUUM_READ_PLC_ADDRESS_SN_TP_1        2
#define _VACUUM_READ_PLC_ADDRESS_SN_TP_2        3

//iStatus种类
#define _VACUUM_THREAD_START 0  //线程开始采集   //判断
#define _VACUUM_THREAD_END   1  //线程结束采集
#define _VACUUM_NO_ANSWER    2  //出错：VGC023 3次通信均没有应答。 报告错误，并结束采集
#define _VACUUM_ACK_RIGHT    3  //采集成功

struct _stVacuumThreadData
{
	//static int iTest;
	SYSTEMTIME   st;
	int    iStatus;     //状态，供判断结果使用 （四种状态）
	DWORD  dwTickTime;  //经过的时间 ms. 调试用
	WORD   wRdData[_VACUUM_READ_PLC_ADDRESS_CNT];
};

//从Hearth Thread发送到主界面的消息 LPARAM结构
#define _HEARTH_READ_PLC_ADDRESS_CNT 7//6->7, 2008.07.15
#define _HEARTH_READ_PLC_ADD_1_CODER      0
#define _HEARTH_READ_PLC_ADD_1_CRUCIBLE   1 //cup
#define _HEARTH_READ_PLC_ADD_2_CODER      2
#define _HEARTH_READ_PLC_ADD_2_CRUCIBLE   3
#define _HEARTH_READ_PLC_ADD_ROTATE_CRUCIBLE   4
#define _HEARTH_READ_PLC_ADD_ROTATE_1_ANNULAR   5
#define _HEARTH_READ_PLC_ADD_ROTATE_2_ANNULAR   6

//直接测试 6 个区域，省得对 坩埚进行测试
//依次是 Hearth1 的 编码器与当前位置(点坩埚), Hearth2 的 编码器与当前位置(点坩埚). 以及点坩埚 是否正在运转的标志位
//编码器位置，现在位置(不管是否环形，如果是环形，则响应处可以略过)以及转动状态

struct _stHearthThreadData
{
	SYSTEMTIME   st;
	int iHearth;
	enum _enHearthThreadDataState{enPlcNoAnswer,enAckRight}enStatus;
	//int    iStatus;     //状态，供判断结果使用
	DWORD  dwTickTime;  //经过的时间 ms. 调试用
	WORD wRdData[_HEARTH_READ_PLC_ADDRESS_CNT];
};

//2008.08.14
struct _stDomeThreadData
{
	SYSTEMTIME   st;
	enum _enDomeThreadDataState{enPlcNoAnswer,enAckRight,enErrNotRotation,enErrSpeed,enErrHighState,enErrStart,enErrStop,enErrDoorSwitchOff,enErrSwitchNotManual}enStatus;
	//除了enAckRight，其它均为错误状态。
	//enPlcNoAnswer, plc通信故障。
	//enAckRight,正确读取数据.
	//enErrStart,第一次进循环时发送此消息。供主线程初始化。
	//enStop,由用户点击界面上的stop或检测完了时主线程关闭检测, 线程在检测到 bstop后，退出前发送此消息
	//enErrNotRotation,没有判断到在转动
	//enErrDoorSwitchOff,门限位需要合上才能High状态转动，但没有检测到。
	//enErrSwitchNotManual,Auto-off-manual开关不在Manual上。必须放置在Manual上才能转动。
	//enErrSpeed, 速度与目标设定不一致，或超出范围
	//enErrHighState, 当前不在高速状态下
	DWORD  dwTickTime;  //GetTickCount()
	WORD wRdData;		//读取到的转动计数
};


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


