#include "StdAfx.h"
#include "RotatorThread.h"
#include "OACnfgDoc.h"


//只在　本　cpp中用到的变量.
#define _OA_HEARTH_THREAD_DATA_BUFFER_SIZE 20
_stHearthThreadData* GetOneHearthThreadBuffer( )
{
	static struct _stHearthThreadData g_stHearthThreadDataBuf[_OA_HEARTH_THREAD_DATA_BUFFER_SIZE];
	static int iBufPos = 0;
	iBufPos++;
	if(iBufPos >= _OA_HEARTH_THREAD_DATA_BUFFER_SIZE)
	{
		iBufPos= 0;
	}
	return &g_stHearthThreadDataBuf[iBufPos];
}


IMPLEMENT_DYNCREATE(CRotatorThread, CPlcRdWrThread)
CRotatorThread::CRotatorThread(void)
{
	m_hWnd = NULL;
}

CRotatorThread::~CRotatorThread(void)
{
}


BEGIN_MESSAGE_MAP(CRotatorThread, CWinThread)
	ON_THREAD_MESSAGE(WM_ITEM3_HEARTH_ROTATE_MONITOR,  OnHearthRotateMonitor)
END_MESSAGE_MAP()

/*************************************************************************************
函数名称：fn_GetHearthInfo
输入参数：[in]  int iHearth ，查询的Hearth号 ,必须为　0或1
          [out] bool& bHearthExist, 是否存在
		  [out] int&  iCrucibleNumber, 点坩埚的数目
返回值：　函数是否正确执行．
说明：　　查询　Hearth 的基本信息
*************************************************************************************/
bool CRotatorThread::fn_GetHearthInfo(int iHearth, bool& bHearthExist, int& iCrucibleNumber)
{
	bHearthExist = false;
	iCrucibleNumber = 0;

	ASSERT( iHearth < 2 ); 

	WORD wValue;
	if( ReadPLCDMEvent( 9056, wValue, false) )
	{	
		//正好 9056.00 .01 对应 Hearth1 Hearth2　
		bHearthExist = ( wValue &  (1<<iHearth))?true:false;
		if(bHearthExist)
		{
			//9057: Hearth1 cup数, 9058: Hearth2 cup数
			if(ReadPLCDMEvent( 9057 + iHearth, wValue, true) )
			{
				iCrucibleNumber = wValue;
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

/*************************************************************************************
函数名称：fn_GetHearthCoderValue
输入参数：[in]  int iHearth ，查询的Hearth号 ,必须为　0或1
		  [out] int&  iCoderVal 
返回值：　函数(通信)是否正确执行．
说明：　　查询 Hearth 编码器读数
*************************************************************************************/
bool CRotatorThread::fn_GetHearthCoderValue(int iHearth, int& iCoderVal)
{
	ASSERT( iHearth < 2 ); 
	WORD wValue;
	if( ReadPLCDMEvent( ( iHearth==0 ? 3886 : 3906) , wValue, true) )
	{
		iCoderVal = (int)wValue;
		return true;
	}
	return false;
}

/*************************************************************************************
函数名称：fn_GetHearthCrucibleNumber
输入参数：[in]  int iHearth ，查询的Hearth号 ,必须为　0或1
		  [out] int&  iCrucibleNumber
返回值：　函数是否正确执行．
说明：　　查询 Hearth 当前坩埚号
*************************************************************************************/
bool CRotatorThread::fn_GetHearthCrucibleNumber(int iHearth,int& iCrucibleNumber)
{
	ASSERT( iHearth < 2 ); 
	WORD wValue;
	if( ReadPLCDMEvent( ( iHearth==0 ? 3923 : 3922) , wValue, true) )
	{	
		iCrucibleNumber = (int)wValue;
		return true;
	}
	return false;
}

/*************************************************************************************
函数名称：fn_GetHearthMonitorType
输入参数：[in]  int iHearth ，查询的Hearth号 ,必须为　0或1
		  [out] int&  iType.  1:oriental, 2: omron.
返回值：　函数是否正确执行．
说明：　　查询 Hearth Monitor的种类
*************************************************************************************/
bool CRotatorThread::fn_GetHearthMonitorType(int iHearth, int& iType)
{
	ASSERT( iHearth < 2 ); 
	WORD wValue;
	if( ReadPLCDMEvent( ( iHearth==0 ? 9089 : 9091) , wValue, true) )
	{	
		iType = (int)wValue;
		return true;
	}
	return false;
}


// 初始化 Hearth 转动方式到 触摸屏，实际是清 0 某DM区域 //Set ACS pos = 0
bool CRotatorThread::fn_InitHearthToPT(int iHearth)
{
	//hearth1 = 6249 (iHearth = 0)
	//hearth2 = 6248 (iHearth = 1)
	ASSERT(iHearth < 2 );
	if( WritePLCDMEvent( iHearth==0 ? 6249 : 6248 , 0 ) )
	{	
		return true;
	}
	return false;
}



//转动点坩埚 (模拟手动点击触摸屏)到指定位置
bool CRotatorThread::fn_RunCruciblePT(int iHearth)
{
	ASSERT(iHearth<2 && iHearth>=0);

	//<手动控制>Hearth 1,2分别为 3813.01, 3814.01
	//1,<手动控制> 上升沿 ( 1 即可 ，plc程序会自动清 ),CIO
	if(!WritePLCCIOEvent( 3813+iHearth, 1<<1 ) )
	{
		return false;
	}

	//停顿 200ms，确保PLC已经记录到该上升沿
	Sleep(200);

	//<移动> Hearth 1,2 分别为 3813.04, 3814.05. 此时<手动控制>需同时为1
	//<移动> Hearth 1,2 分别为 3813.05, 3814.05,//2008.03.20更正
	//2,<手动控制>与<移动>同时为 1
	//if(!WritePLCCIOEvent( 3813+iHearth, 1<<1 | 1<<(iHearth+4) ) ) 
	if(!WritePLCCIOEvent( 3813+iHearth, 1<<1 | 1<<5 ) )
	{
		return false;
	}

	////再来一次? 08.01.04 debugging
	Sleep(200);
	if(!WritePLCCIOEvent( 3813+iHearth, 0 ) )
	{
		return false;
	}

	return true;
}

//转动环形坩埚，正/反
//调用前请确认坩埚处于停止状态.
bool CRotatorThread::fn_RunAnnularHearthPT(int iHearth,bool bPositiveRotation=true)
{
	ASSERT(iHearth<2 && iHearth>=0);

//	ASSERT(iHearth==0); //2007.12.25,因为目前不清楚 Hearth2环形坩埚的位置
	//先设定转动方向
	//Hearth1 转动 3808.04
	//Hearth1 正转 3808.02, 反转 3808.03
	//Hearth2 正转 3809.02, 反转 3809.03 //2008.07.17
	if(!WritePLCCIOEvent(0==iHearth?3808:3809, bPositiveRotation? 1<<2 : 1<<3 ) )
	{
		return false;
	}

	//停顿 300ms，确保PLC已经响应
	Sleep(300);

	//再 <打开>
	if(!WritePLCCIOEvent( 0==iHearth?3808:3809, 1<<4) )
	{
		return false;
	}

	return true;
}

bool CRotatorThread::fn_SwitchAnnularHearthPT(int iHearth)
{
	if(!WritePLCCIOEvent( 0==iHearth?3808:0000, 1<<4) )
	{
		return false;
	}
	return true;
}

// 设定点坩埚位置（触摸屏上显示的位置）
bool CRotatorThread::fn_SetCruciblePosPT(int iHearth, int iPos)
{
	ASSERT(iHearth<2 && iHearth>=0);
	//因为 3940内部是 BCD编码形式，所以需要将 iPos伪装成 那种 例如 iPos = 10，则 iPos2 = 16.实际对应 "0010"
	//int iPos2 = 16*(iPos/10) + iPos%10
	if(iHearth==1)
	{//Hearth2，目前尚不知 Hearth1的设定位置地址！ 2007.12.18//2008.03.20添加Hearth1
		if( WritePLCDMEvent( 3940, 16*(iPos/10) + iPos%10 ) )
		{	
			return true;
		}
		return false;
	}
	else if(iHearth==0)
	{
		if( WritePLCDMEvent( 3995, 16*(iPos/10) + iPos%10 ) )//2008.03.20添加Hearth1
		{	
			return true;
		}
		return false;
	}
	return false;
}


/*
struct _stHearthThreadData
{
	SYSTEMTIME   st;
	enum _enHearthThreadDataState{enPlcNoAnswer,enAckRight}enStatus;
	//int    iStatus;     //状态，供判断结果使用
	DWORD  dwTickTime;  //经过的时间 ms. 调试用
	WORD wRdData[_HEARTH_READ_PLC_ADDRESS_CNT];
};
*/
void CRotatorThread::OnHearthRotateMonitor(WPARAM wParam, LPARAM lParam)
{
	/*
	#define _HEARTH_READ_PLC_ADD_1_CODER      0
#define _HEARTH_READ_PLC_ADD_1_CRUCIBLE   1 //cup
#define _HEARTH_READ_PLC_ADD_2_CODER      2
#define _HEARTH_READ_PLC_ADD_2_CRUCIBLE   3
#define _HEARTH_READ_PLC_ADD_ROTATE_CRUCIBLE   4
#define _HEARTH_READ_PLC_ADD_ROTATE_1_ANNULAR   5
	//2007.12.27新增 3884 ,用于环形坩埚1的 3884.08, 确定是否正在旋转
	*/
	static enum _enumSysmacRdType enRdType[_HEARTH_READ_PLC_ADDRESS_CNT] = {enReadDM,enReadDM,enReadDM,enReadDM,enReadCIO,enReadCIO,enReadCIO};
	//依次是 Hearth1 的 编码器与当前位置(点坩埚), Hearth2 的 编码器与当前位置(点坩埚). 以及点坩埚 是否正在运转的标志位
	static WORD wHearthRdAddress[_HEARTH_READ_PLC_ADDRESS_CNT] = {3886,3923,3906,3922,10,3884,3885};
	//可以将其设置成公共变量,已方便动态更改. 2007.12.29 

	int iHearth = (int)lParam; 
	bool bTest = true;
	bool bPlc  = true;
	_stHearthThreadData* pst;// = GetOneHearthThreadBuffer();
//	PostMessage(m_hWnd,WM_ITEM3_HEARTH_ROTATE_DATA,0,(LPARAM)pst);
	while(bTest)
	{
		Sleep(400);
		EnterCriticalSection(&gCriticalSection_HearthState);
			bTest = g_bTestHearthRotation;
		LeaveCriticalSection(&gCriticalSection_HearthState);
		if(!bTest)
		{
			//AfxMessageBox(TEXT("DEBUG:g_bTestHearthRotation = false. Stop"));
			break;
		}

		bPlc = true;
		pst = GetOneHearthThreadBuffer();
		pst->dwTickTime = GetTickCount();
		GetLocalTime(&pst->st);
		for(int i=0;i<_HEARTH_READ_PLC_ADDRESS_CNT;i++)
		{
			if(enRdType[i] == enReadDM)
			{
				bPlc &= ReadPLCDMEvent(wHearthRdAddress[i],pst->wRdData[i],true);//全是 10进制，省得再开空间.
			}
			else
			{//enReadCIO
				bPlc &= ReadPLCCIOEvent(wHearthRdAddress[i],pst->wRdData[i]);
			}
		}

		if(bPlc)
		{
			pst->enStatus = _stHearthThreadData::enAckRight;
		}
		else
		{//通信不良
			pst->enStatus = _stHearthThreadData::enPlcNoAnswer;
			break;
		}

		PostMessage(m_hWnd,WM_ITEM3_HEARTH_ROTATE_DATA,0,(LPARAM)pst);
	}
	return;
}

//环形坩埚
bool CRotatorThread::fn_GetHearthSpeed(int iHearth, int& iSpeed)
{
	ASSERT( iHearth < 2 ); 
	WORD wValue;
	if( ReadPLCDMEvent( ( iHearth==0 ? 6238 : 6239) , wValue, true) )
	{	
		iSpeed = (int)wValue;
		return true;
	}
	return false;
}

//环形坩埚
bool CRotatorThread::fn_SetHearthSpeed(int iHearth, int iSpeed)
{
	ASSERT( iHearth < 2 ); 
	ASSERT(iSpeed>=0 && iSpeed<1000);
	//将 3 位 10进制数转成 16进制 以蒙过 WritePLCDMEvent.
	//WORD wValue = 16*16*(iSpeed/100) + 16*((iSpeed/10)%10)+iSpeed%10;
	WORD wValue = ((iSpeed/100)<<8) + (((iSpeed/10)%10)<<4) +iSpeed%10;
	if( WritePLCDMEvent ( iHearth==0 ? 6238 : 6239, wValue) )
	{	
		iSpeed = (int)wValue;
		return true;
	}
	return false;
}

//测试环形坩埚是否处于转动中.
bool CRotatorThread::fn_GetHearthRunState(int iHearth, bool& bRunning, bool bAnnular = true)
{
	//环形坩埚 1，是否正在转动中 SERIAL:3884.08
	//环形坩埚 2 的3885.08 ? //2008.07.14
	WORD wVal;
	if( bAnnular)
	{//环形坩埚
		if( ReadPLCCIOEvent(0==iHearth?3884:3885,wVal) )
		{
			if( wVal & (1<<8))
			{
				bRunning = true;
			}
			else
			{
				bRunning = false;
			}
			return true;
		}
	}
	else
	{//点坩埚
		bRunning = false;
		return true;
	}
	return false;
}

/****************************************************************************
函数名称：fn_GetAnnularHearthCntrl
说明:  读取环形坩埚的控制状态,FIX or ROTATE
       本程序模仿触摸屏ROTATE模式下动作,需要状态确认.
输入参数: int iHearth, Hearth号, 0为Hearth1, 1:Hearth2.
          int& iControlState, 返回读取到的控制状态, 0:Fix, 1:Rotate
返回值:   读取成功?
Hearth1	FIX		ROTATE
	写	3808.00	3808.01
	读	3884.14	3884,15
Hearth2	FIX		ROTATE
	写	3809.00	3809.01
	读	3885.14	3885,15
写代表设置,读代表读取状态
****************************************************************************/
bool CRotatorThread::fn_GetAnnularHearthCntrl(int iHearth, int& iControlState)
{
	ASSERT(iHearth==0||iHearth==1);
	WORD wVal;
	if( ReadPLCCIOEvent(0==iHearth?3884:3885,wVal) )
	{
		if( wVal & (1<<14))
		{
			iControlState = 0;
		}
		else
		{
			iControlState = 1;
		}
		return true;
	}
	return false;
}


