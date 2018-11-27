#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DomeRotateThread.h"

#include "OACnfgDoc.h"
// CDomeRotateThread

//只在　本　cpp中用到的变量.
#define _OA_DOME_THREAD_DATA_BUFFER_SIZE 20
_stDomeThreadData* GetOneDomeThreadBuffer( )
{
	static struct _stDomeThreadData g_stDomeThreadDataBuf[_OA_DOME_THREAD_DATA_BUFFER_SIZE];
	static int iBufPos = 0;
	iBufPos++;
	if(iBufPos >= _OA_DOME_THREAD_DATA_BUFFER_SIZE)
	{
		iBufPos= 0;
	}
	return &g_stDomeThreadDataBuf[iBufPos];
}

IMPLEMENT_DYNCREATE(CDomeRotateThread, CWinThread)

CDomeRotateThread::CDomeRotateThread()
{
	m_hWnd = NULL;
}

CDomeRotateThread::~CDomeRotateThread()
{
}

BOOL CDomeRotateThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CDomeRotateThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CDomeRotateThread, CWinThread)
	ON_THREAD_MESSAGE(WM_ITEM3_DOME_ROTATE_MONITOR,  OnDomeRotateMonitor)
END_MESSAGE_MAP()

// CDomeRotateThread message handlers
/******************************************************************
AUTO, OFF, MANUAL: 转动
函数名：fn_ToggleDomeSwitch
输入参数：int iAUTO_OFF_MANUAL
			#define _ZZY_DOME_SWITCH_AUTO   0
			#define _ZZY_DOME_SWITCH_OFF    1
			#define _ZZY_DOME_SWITCH_MANUAL 2
返回值：读取plc是否正确
说明：设定当前 Toggle Switch -> Dome Rotation的状态
触摸屏：
		AUTO	OFF	MANUAL
write:	3820.00	.01	.02
display:3873.00	.01	.02
******************************************************************/
bool CDomeRotateThread::fn_ToggleDomeSwitch(int iAUTO_OFF_MANUAL)
{
	ASSERT(iAUTO_OFF_MANUAL< 3 );
	if( WritePLCCIOEvent(3820, 1<<iAUTO_OFF_MANUAL ))
	{
		return true;
	}
	return false;
}

//获取当前Dome Toggle Switch状态
bool CDomeRotateThread::fn_GetDomeRotationControl(int &iAUTO_OFF_MANUAL)
{
	WORD wWord;
	if( ReadPLCCIOEvent(3873,wWord) )
	{
		iAUTO_OFF_MANUAL = -1;

		if( wWord & 1<<_ZZY_DOME_SWITCH_AUTO)
			iAUTO_OFF_MANUAL = _ZZY_DOME_SWITCH_AUTO;
		else if(wWord & 1<<_ZZY_DOME_SWITCH_OFF)
			iAUTO_OFF_MANUAL = _ZZY_DOME_SWITCH_OFF;
		else if(wWord & 1<<_ZZY_DOME_SWITCH_MANUAL)
			iAUTO_OFF_MANUAL = _ZZY_DOME_SWITCH_MANUAL;

		ASSERT(iAUTO_OFF_MANUAL!=-1);		
		return true;
	}
	return false;
}

/******************************************************************
函数名：fn_GetDomeRotationState
输入参数：	int& iLow,
			int& iSpeed, 当前状态对应的数值
返回值：读取plc是否正确
说明：获取当前活动的转动情况 Low or high以及当前的转速.
触摸屏：
Speed   High	Low	
SERIALA:DM03636	 SERIALA:DM03639  //BCD2
state	High	 Low
write:	3810.00	.01	 //SERIALA:03810.01
display:3877.13	.14

#define _ZZY_DOME_ROTATE_LOW	1
#define _ZZY_DOME_ROTATE_HIGH	0
******************************************************************/
bool CDomeRotateThread::fn_GetDomeRotationState(int& iSpeed, int& iLow)
{
	WORD wWord;
	if( ReadPLCCIOEvent(3877,wWord) )
	{	
		iLow = -1;
		if(wWord & 1<< 13)
		{
			iLow = _ZZY_DOME_ROTATE_HIGH;
		}
		else if(wWord & 1<<14)
		{
			iLow = _ZZY_DOME_ROTATE_LOW;
		}
		ASSERT(iLow!=-1);

		if( ReadPLCDMEvent( iLow==_ZZY_DOME_ROTATE_LOW ? 3639 : 3636, wWord, true) )
		{	
			iSpeed = (int)wWord;
			return true;
		}
	}
	return false;
}
bool CDomeRotateThread::fn_GetDomeHighSpeed(int& iSpeed)
{
	WORD wWord;
	if( ReadPLCDMEvent(3636,wWord, true) )
	{	
		iSpeed = (int)wWord;
		return true;
	}
	return false;
}
bool CDomeRotateThread::fn_SetDomeHighSpeed(int iSpeed)
{
	//10进制 --> 16进制
	WORD wValue = ((iSpeed/100)<<8) + (((iSpeed/10)%10)<<4) +iSpeed%10;
	if( WritePLCDMEvent(3636, wValue))
	{
		return true;
	}
	return false;
}

/*
//设置当前LOW\HIGH转动情况及转速
//Low: 10-15    High:10-30, 也有的OTFC 10-50
触摸屏：
Speed   High	Low	
SERIALA:DM03636	 SERIALA:DM03639  //BCD2
state	High	 Low
write:	3810.00	.01	 //SERIALA
display:3877.13	.14
*/
bool CDomeRotateThread::fn_SetDomeRotationState(int iSpeed, int iLow)
{
	ASSERT(iLow == _ZZY_DOME_ROTATE_HIGH || iLow ==_ZZY_DOME_ROTATE_LOW);
	ASSERT(iSpeed>=10 && iSpeed<=50 );

	if( WritePLCCIOEvent( 3810, iLow==_ZZY_DOME_ROTATE_LOW ? 1<<01 : 1<<00 ) )
	{
		//10进制 --> 16进制
		WORD wValue = ((iSpeed/100)<<8) + (((iSpeed/10)%10)<<4) +iSpeed%10;
		if( WritePLCDMEvent( iLow==_ZZY_DOME_ROTATE_LOW ? 3639 : 3636, wValue))
		{
			return true;
		}
	}
	return false;
}



//当前是否处于转动中，方法为读取触摸屏上的显示地址
//SERIALA:03876.04
bool CDomeRotateThread::fn_IsDomeRotation(bool& bRunning)
{
	WORD wWord;
	if( ReadPLCCIOEvent(3876,wWord) )
	{
		bRunning = ( 0 != (wWord & ((WORD)1<<4)));
		return true;
	}
	return false;
}

//门上的限位开关是否关闭， High下必须满足，需要人为锁上。 加提示
//SERIALA:03876.00
bool CDomeRotateThread::fn_IsDoorSwitchClosed(bool &bClosed)
{
	WORD wWord;
	if( ReadPLCCIOEvent(3876,wWord) )
	{
		bClosed = ( 0 != (wWord & (1<<0)));
		return true;
	}
	return false;
}

bool CDomeRotateThread::fn_ReadDomeRotationCounts(WORD& wRotationCnt)
{
	return ReadPLCDMEvent(9999,wRotationCnt);
}

/****************************************************************
//程序中每开始一个新的item，就发一次本消息.由OnDomeRotateMonitor响应
1，检查Door Switch，必须 On.	否则发送消息.最终退出必须 由外部将g_bTestDomeRotation设置成false.
2，设定当前的速度,设定High
3，设定Manual
4，进入循环 
	{
		Sleep(400); //等待400ms.
		1>, 检查g_bTestDomeRotation状态, true继续2>，false发送消息
		2>, 检查Rotation(run)状态
			a, running,继续3>
			b, not running
			{
				检查 Door Switch,
				检查 Manual.
				发送消息并退出
			}
		3>, 读取当前的 读数，并发送消息
	}
*****************************************************************/
void CDomeRotateThread::OnDomeRotateMonitor(WPARAM wParam, LPARAM lParam)
{
	int  iSpeed = (int)lParam;

	bool bTest = true; //是否处于测试中
	bool bPlc  = true; //通信是否正常
	_stDomeThreadData* pst=NULL;
	bool bRunning = false;//是否在转动
	bool bDoorClosed  = false;//门开关是否合上
	int  iAuto_off_manual = _ZZY_DOME_SWITCH_AUTO;
	
	int  iCurSpeed   = 5;//变量，留待循环中用。
	int  iCurHighLow = _ZZY_DOME_ROTATE_LOW;
	

	//发送线程响应消息
	pst = GetOneDomeThreadBuffer();
	pst->dwTickTime = GetTickCount();
	pst->enStatus   = _stDomeThreadData::enErrStart; //门未关紧
	PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_START,(LPARAM)pst);


	//第一次进入时进行前期设定与检查.
	while(bTest)
	{
		Sleep(300);
		EnterCriticalSection(&gCriticalSection_DomeState);
			bTest = g_bTestDomeRotation;
		LeaveCriticalSection(&gCriticalSection_DomeState);
		if(!bTest)
		{	
			pst = GetOneDomeThreadBuffer();
			pst->dwTickTime = GetTickCount();
			pst->enStatus   = _stDomeThreadData::enErrStop; 
			PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_START,(LPARAM)pst);
			break;				
		}

		bPlc = true; //每次循环开始，重新假定通信正常.

		
				
		//1,检查Door Switch，必须 On.
		bDoorClosed = false;
		bPlc &= fn_IsDoorSwitchClosed(bDoorClosed);
		if(bPlc && !bDoorClosed)
		{
			pst = GetOneDomeThreadBuffer();
			pst->dwTickTime = GetTickCount();
			pst->enStatus   = _stDomeThreadData::enErrDoorSwitchOff; //门未关紧
			PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_START,(LPARAM)pst);
			continue;
		}

		//2,设定当前的速度，并设置成High.
		if(iSpeed<5 || iSpeed>50)
		{//如果iSpeed超出范围，则不进行设定！
			pst = GetOneDomeThreadBuffer();
			pst->dwTickTime = GetTickCount();
			pst->enStatus   = _stDomeThreadData::enErrSpeed; //Speed超出范围
			PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_START,(LPARAM)pst);
		}
		else
			bPlc &= fn_SetDomeRotationState(iSpeed,	_ZZY_DOME_ROTATE_HIGH);

		//3,设定Dome switch为Manual模式
		bPlc &= fn_ToggleDomeSwitch(_ZZY_DOME_SWITCH_MANUAL);
		
		if(!bPlc)
		{
			pst = GetOneDomeThreadBuffer();
			pst->enStatus = _stDomeThreadData::enPlcNoAnswer;
			pst->dwTickTime = GetTickCount();
			PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_START,(LPARAM)pst);
			continue;
		}
		else
		{
			break;
		}
	}
	
	while(bTest)
	{
		Sleep(200);
		EnterCriticalSection(&gCriticalSection_DomeState);
			bTest = g_bTestDomeRotation;
		LeaveCriticalSection(&gCriticalSection_DomeState);

		if(!bTest)
		{	
			pst = GetOneDomeThreadBuffer();
			pst->dwTickTime = GetTickCount();
			pst->enStatus   = _stDomeThreadData::enErrStop; 
			PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_START,(LPARAM)pst);
			break;				
		}
		
		bPlc = true; //每次循环开始，重新假定通信正常.

		//确认处在转动状态.
		bRunning = false;
		bPlc &= fn_IsDomeRotation(bRunning);
		if(bPlc && !bRunning)
		{//不是转动状态
			pst = GetOneDomeThreadBuffer();
			pst->dwTickTime = GetTickCount();
			pst->enStatus   = _stDomeThreadData::enErrNotRotation; //不在转动中
			PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_READ,(LPARAM)pst);
		
			bDoorClosed = false;
			bPlc &= fn_IsDoorSwitchClosed(bDoorClosed);
			if(bPlc && !bDoorClosed)
			{
				pst = GetOneDomeThreadBuffer();
				pst->dwTickTime = GetTickCount();
				pst->enStatus   = _stDomeThreadData::enErrDoorSwitchOff; 
				PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_READ,(LPARAM)pst);
			}
			else if(bPlc && bDoorClosed)
			{
				bPlc &= fn_GetDomeRotationControl(iAuto_off_manual);
				pst = GetOneDomeThreadBuffer();
				pst->dwTickTime = GetTickCount();
				pst->enStatus   = _stDomeThreadData::enErrSwitchNotManual;
				PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_READ,(LPARAM)pst);
			}
			continue; //跳过后续，进行下一轮检查!
		}

		//确认当前的速度与设置相同，并已处于高速状态
		bPlc &= this->fn_GetDomeRotationState(iCurSpeed,iCurHighLow);
		if(bPlc)
		{
			if(iCurSpeed!=iSpeed)
			{
				pst = GetOneDomeThreadBuffer();
				pst->dwTickTime = GetTickCount();
				pst->enStatus   = _stDomeThreadData::enErrSpeed; 
				PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_READ,(LPARAM)pst);
				continue;
			}
			else
			{
				if( iCurHighLow != _ZZY_DOME_ROTATE_HIGH)
				{
					pst = GetOneDomeThreadBuffer();
					pst->dwTickTime = GetTickCount();
					pst->enStatus   = _stDomeThreadData::enErrHighState; 
					PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_READ,(LPARAM)pst);
					continue;
				}
			}
			
		}

		pst = GetOneDomeThreadBuffer();
		pst->dwTickTime = GetTickCount();
		GetLocalTime(&pst->st);
		bPlc &= fn_ReadDomeRotationCounts(pst->wRdData); //读取计数器读数
		if(bPlc)
		{
			pst->enStatus = _stDomeThreadData::enAckRight;
		}
		else
		{//通信不良
			pst->enStatus = _stDomeThreadData::enPlcNoAnswer;
			break;
		}

		PostMessage(m_hWnd,WM_ITEM3_DOME_ROTATE_DATA,_ZZY_DOME_ROTATE_DATA_WPARAM_READ,(LPARAM)pst);
	}
	return;
}