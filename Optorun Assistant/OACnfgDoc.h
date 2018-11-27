#pragma once
#include "afx.h"

#include "..\XMLMarkup\Markup.h"
#include "..\SyncCommSource\SysmacPLC.h"
#include "IonGauge.h"
#include "APCGauge.h"
#include "VacuumProcessThread.h"
#include "gascontroller.h"
#include "odmcom.h"
#include "VGC500.h"
#include "IBGLCom.h"
#include "ZYGraphPlot.h"
//仅保留为调试使用，正式release版中应取消该定义
#define _ZHANGZIYE_DEBUG
//#undef  _ZHANGZIYE_DEBUG

extern const double g_dProgramVersion;
extern const double g_dDataVersion;
extern bool g_bOffLineDebug;


enum   enumOptorunCoaterType{_en_Coater_UNKNOWN_TYPE,_en_Coater_Gener,_en_Coater_OTFC,_en_Coater_AR
};
extern enum enumOptorunCoaterType g_enOptorunCoaterType;

//已经定义在MainFrm中了，必须避让开
//#define WM_INIT_OTHER_FORM  (WM_USER + 401)
//#define WM_MENU_SELECT_LNG_CMD    (WM_USER + 410)  //预留100个空间，下一个必须定义成 511

#define MAIN_FRM_START_WM	(WM_USER + 511) 
#define ITEM1_START_WM		(WM_USER + 600)
#define ITEM2_START_WM		(WM_USER + 800)
#define ITEM2_START_VACUUM_THREAD (WM_USER+850)
#define OA_APP_START_WM     (WM_USER + 900)
#define WM_ITEM3_ROTATOR_START  (WM_USER+920) 

		//   设定参数
		
#define _VACUUM_EXHAOSUT_PRESSURE_OK_TYPE 0  //0: 真空度达到(1.0E-4), 稳定时间  
#define _VACUUM_EXHAOSUT_END_TIME_OK_TYPE 1  //1: 停止时间达到

#define _MACHINE_TYPE_GENER      0
#define _MACHINE_TYPE_SUPERAR    1
#define _MACHINE_TYPE_AR     2
#define _MACHINE_TYPE_1800   3
#define _MACHINE_TYPE_1550   4
#define _MACHINE_TYPE_1300   5
#define _MACHINE_TYPE_1100   6
#define _MACHINE_TYPE_900    7
extern int g_iMachineType;


#define  _VACUUM_PRESSURE_MAX (2*24*3600)
extern int    g_iVacuumPressurePos;     //当前可用于存储真空度的位置.
extern double g_dVacuumVGCCh3[];   //存储 VGC023 ch3 (penning)真空度（已经转化成Pascal）
extern double g_dVacuumIonGauge[]; //存储 Ion Gauge (penning)真空度（已经转化成Pascal）
extern double g_dVacuumTime[];     //2008.04.08

extern int    g_iVacuumTestCnt;     //每次测试开始后数据到来的次数. //同秒数 
extern int    g_iVacuumStartPos;    //每次测试的数据位于 存储空间的起始位置. 

#define OA_APP_DISPLAY_VAC_VALUE	(OA_APP_START_WM+0)
#define OA_APP_VACUUM_THREAD_ERR_OCCUR (OA_APP_START_WM+1)
#define WM_PLC_RD_WR_MSG    (OA_APP_START_WM+2)


//从item x 发送的消息转发给item y，在mainframe中转
//从源 x 在 HIWORD(WPARAM), y在LOWORD(WPARAM)中, LPARAM为待转发的消息，转发的消息没有参数.
#define MAIN_FRM_ITEMx_2_ITEMy     (MAIN_FRM_START_WM+0)
#define MAIN_FRM_TEST	           (MAIN_FRM_START_WM+1)
#define MAIN_FRM_DISPLAY_VAC_VALUE (MAIN_FRM_START_WM+2)

//从 vacuum thread发送来的异常发生消息
#define MAIN_FRM_VACUUM_THREAD_ERR_OCCUR     (MAIN_FRM_START_WM+3) 

#define WM_ITEM1_DISPLAY_PLC_STATE (ITEM1_START_WM + 0)
#define WM_ITEM1_ADD_RUN_HISTORY   (ITEM1_START_WM + 1)
#define WM_ITEM1_GET_NAMES         (ITEM1_START_WM + 2)

#define WM_ITEM2_DISPLAY_VAC_STATE (ITEM2_START_WM + 0)
#define WM_ITEM2_DISPLAY_VAC_VALUE (ITEM2_START_WM + 1)//显示测量到的真空度值

#define WM_ITEM2_VACUUM_THREAD ( ITEM2_START_WM + 2) //从 Vacuum Thread发送来的消息
#define WM_ITEM2_TEST (ITEM2_START_WM+3)

#define WM_ITEM2_VACUUM_THREAD_QA (ITEM2_START_WM+4) //从Item2 发送给Vacuum Thread的消息．用于查询起始条件

#define WM_VACUUM_THREAD_TO_ITEM2 (ITEM2_START_VACUUM_THREAD+0)
#define WM_VACUUM_THREAD_TIMER_EX (ITEM2_START_VACUUM_THREAD+1)
#define WM_VACUUM_THREAD_ITEM2_QA (ITEM2_START_VACUUM_THREAD+2) //从Vacuum Thread 发送给Item2的消息．用于响应起始条件


#define WM_ITEM3_DRAW_CRUCIBLE   (WM_ITEM3_ROTATOR_START+0)
#define WM_ITEM3_HEARTH_ROTATE_MONITOR   (WM_ITEM3_ROTATOR_START+1) //from view to thread
#define WM_ITEM3_HEARTH_ROTATE_DATA   (WM_ITEM3_ROTATOR_START+2) //from thread to view

#define WM_ITEM3_DOME_ROTATE_MONITOR  (WM_ITEM3_ROTATOR_START+3) //from view to thread
#define WM_ITEM3_DOME_ROTATE_DATA   (WM_ITEM3_ROTATOR_START+4) //from thread to view

//颜色常量
extern const COLORREF gc_clrBlack;
extern const COLORREF gc_clrTrying;
extern const COLORREF gc_clrOK;
extern const COLORREF gc_clrAlert;
extern const COLORREF gc_clrWhite;
extern const COLORREF gc_clrRunning;

extern double   gc_dMaxPenningPa;
extern double   gc_dMinPenningPa;
extern double   gc_dMaxIonGaugePa;
extern double   gc_dMinIonGaugePa;

//DM地址常量
extern const WORD   gc_DM_machinetype;
extern const WORD   gc_DM_sequencer;
extern const WORD   gc_DM_touchpanel;
extern const WORD   gc_DM_IonGaugeType;
extern const WORD   gc_DM_APCGaugeType;
extern const WORD   gc_DM_IonGaugeValue;
extern const WORD   gc_DM_PenningValue;

//以下四个地址数组参数在联机后进行初始化.
extern const WORD   gc_DM_AddressHeaterTempSVPV;
extern const WORD   gc_DM_AddressDome2TempSVPV; //侧壁温度现在值与当前值的2个地址
#define _TEMP_MONITOR_HEATER_PV 0
#define _TEMP_DOME_HEATER_PV	1
#define _TEMP_MONITOR_HEATER_SV 2
#define _TEMP_DOME_HEATER_SV	3
#define _TEMP_DOME2_HEATER_PV   4
#define _TEMP_DOME2_HEATER_SV   5
#define _TEMP_HEATER_NUMBER     6
extern WORD gcp_DM_HeaterTemp[];

extern const int gc_plc_str_length;

//语言选择字符串表
#define _OADOC_LNG_MENU_NAME	0
#define _OADOC_LNG_FILE_NAME	1

//软件系统用字符串
#define _OA_STR_SYSTEM_NUMBER				11 //总数
#define _OA_STR_SYSTEM_ERROR_TEST			0 //第0个,正在测试，报错用
#define _OA_STR_SYSTEM_COMM_OBJECT_TIMEOUT  1
#define _OA_STR_SYSTEM_PLC_COMM_ERROR       2
#define _OA_STR_SYSTEM_PLC_COMM_API_ERROR   3
#define _OA_STR_SYSTEM_PLC_COMM_ENDCODE_ERR 4
#define _OA_STR_SYSTEM_CHANGE_LANGUAGE_NOTE 5
#define _OA_STR_SYSTEM_IS_TESTING_VAC_ERROR 6
#define _OA_STR_SYSTEM_VAC_DATA_NOT_SAVE_ERR 7
#define _OA_STR_SYSTEM_HEARTH_DATA_NOT_SAVE  8
#define _OA_STR_SYSTEM_LNG_FILE_NOT_EXIST    9
#define _OA_STR_SYSTEM_IS_TESTING_DOME_ERROR 10
//Item 
#define _ITEM1_NEED_TEXT_ID_CNT   11//需要从xml读取字符串的ID数，此处的ID只对应一种文本
#define _ITEM1_NEED_HELP_TEXT	  6 //需要从xml读取的帮助字符串数，本帮助专门出现在帮助栏内

extern const int gc_item2_Text_ID_CNT;
//#define _ITEM2_NEED_TEXT_ID_CNT 10

//Formview　
#define _OA_FORM_VIEW_NUMBER	3
#define _OA_FORM_VIEW_SYSTEM	0
#define _OA_FORM_VIEW_VACUUM	1
#define _OA_FORM_VIEW_ROTATOR	2

//PLC connection state
#define _ITEM1_COM_PLC_STATE_CNT  6 //com与plc端口连接状态总数，显示在连接状态处。
#define _COM_PLC_STATE_NOT_CONNECT 	0
#define _COM_PLC_STATE_TRY_COMM 	1
#define _COM_PLC_STATE_OPEN_COM_ERROR 	2
#define _COM_PLC_STATE_COMM_NORMAL 	3
#define _COM_PLC_STATE_COMM_ERROR 	4//CANCEL
#define _COM_PLC_STATE_COMM_NORESPOND 4
#define _COM_PLC_STATE_COMM_CONTENT_ERROR 5

//item2 VAC state
#define _ITEM2_VAC_STATE_CNT        3
#define _ITEM2_VAC_STATE_NOT_CONNECT 0
#define _ITEM2_VAC_STATE_PLC_OK      1
#define _ITEM2_VAC_STATE_TESTING     2

//item2 额外需要使用到的字符串
#define _ITEM2_OTHER_STR_NUMBER    18 //总数
#define _ITEM2_USING_ION_GAUGE     0 //第0个
#define _ITEM2_CANCEL_WARNING_TEXT 1
#define _ITEM2_STR_LEAKRATE_HELP   2
#define _ITEM2_STR_RECORDER_HELP   3
#define _ITEM2_STR_EXHUAST_HELP   4
#define _ITEM2_STR_MANUAL_EXHUAST_MODE_ERR 5
#define _ITEM2_STR_MAIN_VALVE_CLOSE_ERR 6
#define _ITEM2_STR_START_PRESSURE_ERR   7
#define _ITEM2_STR_WAITTING_TIME_OVER 8
#define _ITEM2_STR_PLC_COMM_ERR       9
#define _ITEM2_STR_PLC_COMM_ERR_STOP 10
#define _ITEM2_STR_TESTTIME_INVALID_ERR  11
#define _ITEM2_STR_INTERVAL_TIME_ERR  12
#define _ITEM2_STR_START_PRESSURE_INVALID 13
#define _ITEM2_STR_WAITINT_TIME_MIN_ERR 14
#define _ITEM2_STR_PRESSURE_STABLE_TIME_MIN_ERR 15
#define _ITEM2_STR_NOT_DP_MACHINE_ERR   16
#define _ITEM2_STR_IONGAUGE_NOT_REMOTE_ERR 17

//item3 额外需要使用到的字符串
#define _ITEM3_OTHER_STR_NUMBER        9 //总数
#define _ITEM3_STR_HEARTH_TEST_EXIT_Q  0 //测试中途退出时提问
#define _ITEM3_STR_HEARTH_TEST_FINISH  1 //正常结束，恭喜
#define _ITEM3_STR_HAERTH_ROTAT_POS_ERR 2  //转达不到位，错误
#define _ITEM3_STR_HEARTH_ROTAT_OVER_TIME 3 //转动超时
#define _ITEM3_STR_PLC_COMM_ERR  4
#define _ITEM3_STR_ANNULAR_HEARTH_NOT_ROTAT 5 //环形坩埚没有按预期开始转动
#define _ITEM3_STR_ANNULAR_HEARTH_CONTROL_WARNING 6 //环形坩埚控制模式不是ROTATE,报警.
#define _ITEM3_STR_ANNULAR_HEARTH_MONITOR_REMIND  7 //提醒, 在MONITOR TYPE=2(OMRON)并且环形坩埚时，xx.x 表示, 30.0代表300.
#define _ITEM3_STR_DOME_TEST_EXIT_Q	 8 //测试中途用户点击退出时提问.

//将 .cpp文件中声明的对象 extern出来，供外部包含本.h的类调用。
//否则，如果在 其他的类 函数前 应用 extern ，则const变量会出现 不能解析的链接错误。
extern CString g_strSystem[];
extern const TCHAR* g_strProductVersion;
extern TCHAR g_strBuffer[];
extern CString g_item1_ID_Text[];
extern const int    g_item1_ID[];
extern CString g_item1_Help_Text[];
extern CString g_item1_plc_state[];
//extern const TCHAR* g_item1_ID_Name[];

extern CString g_item2_Vac_state[];
extern CString g_item2_String[];
extern CString g_item2_ID_Text[];
extern const int    g_item2_ID[];

extern const int gc_item3_Text_ID_CNT;
extern CString g_item3_ID_Text[];
extern const int    g_item3_ID[];
extern CString g_item3_String[];

extern int giHelpItems;
extern CODMCom g_COM2_ODM;
extern CGasController g_COM4_GasController;
extern CVGC500  g_COM5_VGC500;
extern CIBGLCom g_COM3_IBGL;
extern CSysmacPLC g_plc;
extern CIonGauge  g_IonGauge; 
extern CAPCGauge  g_APCGauge;
extern CVacuumProcessThread* g_pVacProThread;
extern CPlcRdWrThread*       g_pPlcRdWrThread;
extern HANDLE g_hEvent_plc;
extern CRITICAL_SECTION gCriticalSection_State;
extern CRITICAL_SECTION gCriSec_Vacuum;
extern CRITICAL_SECTION gCriSec_UI; 

#define _VACUUM_THREAD_STATE_ZERO				0
#define _VACUUM_THREAD_STATE_WAITING_REPLY		1
#define _VACUUM_THREAD_STATE_REPLIED_ABORT		2
#define _VACUUM_THREAD_STATE_REPLIED_RETRY 	    3
//#define _VACUUM_THREAD_STATE_REPLIED_CONTI 	    4

struct stVacuumTestState
{
	bool bTest;
	int  iThreadState;
		//#define _VACUUM_THREAD_STATE_ZERO				    0
		//#define _VACUUM_THREAD_STATE_WAITING_REPLY		1
		//#define _VACUUM_THREAD_STATE_REPLIED_ABORT		2
		//#define _VACUUM_THREAD_STATE_REPLIED_RETRY 	    3
	int  iMainRouting;
	int  iSubRouting;
	int  iTryTimes;
};

#define _VACUUM_TESTTYPE_EXHAUST  1
#define _VACUUM_TESTTYPE_LEAKRATE 2
#define _VACUUM_TESTTYPE_RECORDER 3

//分清哪些只需 主线程管理，哪些是 主线程 与 子线程都有读写的。
// 公共读写：bTesting

//某一次只能测量 抽速或漏率
//抽速，自动控制抽气状态
//手动测量
struct stVacuumTestEx{
	//只有 bTesting 和 iCurTestType 在线程之间切换使用。需要同步保护。对其他项的操作则全在主线程内，无需同步保护。
	//运行检查参数
	bool bTesting;          //当前是否处于测试中。测试开始时主界面设置true，辅助线程判断。
	                        //结束时，由线程
	// 当前测试状态类型. 设定参数. 开始测试后是不会改变的, 所以不需要同步操作..
	// 1,测试抽速; 2,测试漏率; 3:记录仪
	int  iCurTestType;   //主线程写，辅助线程读。在 bTesting == true的情况下，不可能会改变。

	struct stExhaust
	{
		//   设定参数
		//0: 真空度达到(1.0E-4), 稳定时间  
		//1: 停止时间达到
		int  iOKType; //抽速完成要达成的条件 //用户设置. 由主线程保管。

		//设置参数, 自动停止记录条件之真空度达到并保持一段时间
		double dOKPressure;      //希望达到的停止真空度, 默认 1.0E-4
		DWORD  dwOKPressureTime; //达到希望的停止真空度后稳定时间(稳定)。

		//设置参数, 自动停止记录条件之时间到达
		//记录12小时 的真空度.(添加记录12小时后的真空度)
		bool   bLastRecordPressure; //是否需要添加一个记录 
		DWORD  dwLastRecordTime;    //需要记录的时刻. (时刻到达后,如果此时达到 dOKPressure则记录停止并存盘. 如果没有达到则继续直到达到...)

		//运行时记录参数
		double dLastPressure;

		//程序记录参数
		//int  iFinishType; //实际停止时记录到的类型
			//抽速测量完成的条件
			//a, 达到设定的真空度， 再稳定 时间.  <= 1.0E-4  ( 连续稳定 1 分钟，内部控制 ) 就 停止记录
			//b, 不管，直接记录到(例如，12个小时)时间到达后再停止。
			//c, 用户取消（停止）
		    //d, 不正常停止

		//程序 监听 但不监控！必须由用户手工操作。

	//运行检查参数.
		int    iCurPos;        //当前待完成的位置.	
	}sExhaust;

	struct stLeakRate
	{		
		//运行检查参数
	    //int  iOKType;         //漏率测量完了的条件  //0:时间 达到 20分钟后停止
		
		//设置参数,开始类型.
		//int  iWorkType;   //工作类型 0:程序自动控制，前提，必须在抽气状态，必须处于高真空(ch3已经能读取)。
		                  //适用于抽气开始后,自动等待测量.
		                  //1:手动开始。相当于记录仪 + 计算器

		//程序自动控制下的设置参数
		//此处的自动控制，是相当于 触摸屏界面处于  抽气控制 页面， 手动后，再关闭 MV.
		double dStartPressure;   //程序自动控制时的起始真空度。		                         
		int    iWaitTimeSeconds; //起始最长等待时间

		//int    iW;   // 测量结束时, 是否打开主阀门,继续抽气. //暂时为保持.
		/*
		//保留，暂时不关心 PolyCold
		bool   bStopPFC;         //自动控制时是否关闭 PolyCold.  true: 关闭PolyCold， false:不关心 PolyCold
		bool   bPFCOnStartState; //记录 PFC 在 开始时的状态.
		bool   bPFCOnStopState;  //停止记录时  PFC 的状态.
		*/

		//用于计算漏率，不需要共享到 测量线程中的
		double dChamberVolume;           //真空室尺寸. //设置参数, 以下是运行过程中记录的参数
		double dRecordedStartPressure;   //漏率检测开始时记录的真空度
		double dRecordedEndPressure;     //停止检测时记录的真空度
		double dRecordedStartIonGauge;   //漏率检测开始时记录的Ion gauge真空度( 如果Ion Gauge 有的话 )
		double dRecordedEndIonGauge;     //停止检测时记录的Ion gauge真空度 ( 如果Ion Gauge 有的话 )
		int    iTimeElapseSeconds;       //从开始到停止时实际经过的时间。 
		int    iPressureKeepTime;   //达到真空度后稳定一段时间．
		double fnLeakRate(void)
		{
			//m_dVacuumRateIonGauge = (m_dStartIonGauge-m_dIonGaugePascal)*m_dChamberVolume / (double)m_nMeasureTime ;
			return ( (dRecordedEndPressure -dRecordedStartPressure)*dChamberVolume /(double) iTimeElapseSeconds );
		};
		double fnIonGaugeLeakRate(void)
		{
			return ( (dRecordedEndIonGauge -dRecordedStartIonGauge)*dChamberVolume /(double) iTimeElapseSeconds );
		};		
	}sLeakRate;

	//设置参数
		int  iRecordTime;     //希望记录时间长度
		int  iRecordInterval; //记录的时间间隔
};


//#define _VACUUM_CONDITION_RV 1
/********************************2018-8-29 Zhou Yi************************************************************/
//从 Item2 向　VacuumThread发送消息时　LPARAM所使用的结构.
//struct stItem2VacuumThreadQA
//{
//	int iQAType; //询问消息的种类. 
//	             //0, 查询结果(读)
//	             //1, 执行动作(写), 并等待结果
//
//	int   iCnt;         //当前读取/写入的个数
//	WORD  wAddress[20]; //读取/写入地址
//	WORD  wBit[20];     //写入的地址位
//	bool  bDec[20];     //是否用　10进制　读数,　true:10进制． false:16进制
//    
//	WORD  wMask[20];    //正确的mask
//
//};


/*
抽气记录动作,未列出的位不关心. 需要先用MASK滤除,然后进行相等运算即可. 
数据来源请参见　触摸屏 < screen 0003 >
SRV-OP	3876	00=1	14=1	15=0
		3877	02=0	04=0	05=1	06=0	08=0	09=0

RV-OP	3876	00=1	14=1	15=0
		3877	02=0	04=1	05=1	06=0	08=0	09=0

MBP-ON	3876	00=1	14=1	15=1
		3877	02=0	04=1	05=1	06=0	08=0	09=0

SRV-CL	3876	00=1	14=1	15=1
		3877	02=0	04=0	05=0	06=0	08=0	09=0

MV-OP	3876	00=1	14=1	15=1
		3877	02=1	04=0	05=0	06=1	08=0	09=0
*/
//本Data仅限于抽速测量时的数据记录
struct stVacuumExhaustData
{
	static WORD wTP_1_Mask;    //TP1的Mask, 每次的数据与 Mask "与 "后再判定
	static WORD wTP_2_Mask;    //TP2的Mask, 

	double dPressure;
	bool   bPressureExist; //有真空度项， ListCtrl 填空用


    //早先 VGC023， ch2只能测低真空， ch3只能测高真空. 有的机型配额外电离规(GI-M2/M832等)作校正用.
	//     泵的开关真空条件由 ch2决定。
	//目前 VGC403， ch2, ch3都能测量 低真空到高真空. ch2的高真空相当于原额外的电离规. plc中在同一地址
	//     泵的开关据说 可设定由 ch2或ch3真空度决定。通常 ch3 ?
	//_enumPressureVGC: 由真空度，程序内部只记录 VGC ch3
	//_enumPressureBoth: 程序同时记录 Ion Gauge 和 VGC ch3
	//_enumAction:   由动作，程序内部记录动作时刻, 显示的真空度是预先填写的.//
	                       //记录初始列表既有动作又有真空度时，若存在VGC403，则动作发生时刻，在电离规栏内填写此时刻的(ch2)真空度.
	//_enumAction_CH2: 记录此动作时刻，并将此时刻的ch2记录下来， 2008.03.26添加. //
	                       //记录初始列表只有动作，没有真空度时， 若存在VGC403，则在动作发生时刻，在真空度栏内添加( "1.0E+00(ch2) ")字样。 
	//_enumNotRecord_CH2: 程序不记录, 若是VGC403则记录 ch2 到达指定真空度的时刻. 2008.03.26由_enumNotRecord更改
	                       //记录开始列表中只有真空度，却没有动作状态.用 ch2的数据(只适合VGC403)
	//所有情况，用户双击Extra time 列 均可记录当前时刻
	enum   {_enumPressureVGC,_enumPressureBoth,_enumAction,_enumAction_CH2,_enumNotRecord_CH2} enRecordType;
	//记录动作的如果存在VGC403，则记录真空度



	//下面三行可忽略，因为实际并无用处
	DWORD  dwTimeVGC;      //自动记录到 的 ch3 真空度对应时刻
	DWORD  dwTimeIonGauge; //自动记录到 的 IonGauge 真空度对应时刻
	DWORD  dwTimeAction;   //用户双击 Extra time行时记录。

	// _enumAction, 在需要记录第一次出现的状态处，进行比较．初始化完成即可.
	WORD   wTP1_OK_Value;  
	WORD   wTP2_OK_Value;

	bool   bRecord; // _enumAction 下，状态是否已经记录． 每次开始测试时需清成　false;
	                // _enumPressureXXX 下 VGC　ch3 是否记录

	bool   bIonGaugeRecord; //_enumPressureBoth下Ion Gauge项是否已经记录完成
	
	CString sActionName;  //_enumAction下需填写的 Action name.
};
extern int g_iVacuumExhaustDataItems;
extern int g_iVacuumExhaustDataHighItems;
extern struct stVacuumExhaustData* g_pstVacuumExhaustData;
//在 OACnfgDoc.cpp中定义
extern WORD   g_wVacuumRdAddress[];  //需要初始化
extern bool   g_bVacuumRdDecimal[];
extern int    g_iVacuumRdDM_IO[];

extern struct stVacuumTestEx    g_stVacuumTestEx;
extern struct stVacuumTestState gi_stVacuumTestState;

//extern bool  volatile gb_UsingIonGauge;
extern bool  g_bRecordVacuum;  //是否记录当前的真空度，开始时默认记录（连接上plc后自动）。
extern bool  gb_ProgrammerDebugNormal;

extern bool   g_bUseIongauge; //是否在抽速测试过程中使用 IonGauge.
extern CRITICAL_SECTION gCriticalSection_HearthState;
extern bool   g_bTestHearthDataSaved;
extern bool   g_bTestHearthRotation ; //是否正在进行测试坩埚旋转

extern CRITICAL_SECTION gCriticalSection_DomeState;
extern bool   g_bTestDomeRotation; //是否正在进行Dome旋转测试
extern bool   g_bTestDomeStopByUser; //Dome停止测试的原因是被用户中止。

/*********************2018-8-29 Zhou Yi********************************/
//typedef struct OA_VACUUM_ACTION_STATE_CIO_BIT
//{
//	WORD wWRAddress; //cmd address
//	WORD wWRBit;
//
//	WORD wRDAddress; //state address
//	WORD wRDBit;
//}_OA_VACUUM_ACTION_STATE_CIO_BIT;

//开一个绘图线程，接收主线程数据
struct _stDrawHearth
{
	CRect rect; //绘图区域
	HWND  hwnd; //待绘图窗口(用于获取DC)
	int   iCrucibleMaxNumber;//有多少　Crucible
	int	  iCoderVal;//当前编码器读数 ( 0~359)

	//2007.12.21
	int   iHearth;
	union
	{
		int   iSpeed;
		int   iCurCurcible;
	};
};


typedef struct sVacuumPressure
{
	double dVacuumPenningPa;
	double dVacuumIonGaugePa;
}_sVacuumPressure;
extern _sVacuumPressure gs_VacuumPressure;


typedef struct sPlcCommError
{
	bool   bObjectErr;       //是未等待到  single object.从而软件内部不许可进行通信
	DWORD  dwPlcLastError;   //plc通信的Error code
	DWORD  dwPlcLastErrorEx; //plc通信Error code的补助说明.
	                         //当为 API 函数出错时，为 m_dwLastSysApiError
	                         //当为 通信内容出错例如 EndCode Error时，为 m_iLastEndCode
}_sPlcCommError;
extern _sPlcCommError gs_plcCommError;

/************************************2018-8-29 Zhou Yi*************************************************/
//typedef struct sPlcRdWr
//{
//	DWORD idThread; // thread identifier
//	UINT Msg;       // message
//	WPARAM wParam;  // first message parameter
//	LPARAM lParam;   // second message parameter
//}_sPlcRdWr;

class COACnfgDoc : public CObject
{
public:
	COACnfgDoc(void);
public:
	~COACnfgDoc(void);
public:
	CMarkup     xml;

	// ini 文件相关内容
	int m_iCurLng; //当前选择的语言序号
	int m_iLngCnt; //总可选择语言种类数量

public:
	enum en_TestingVacuum
	{
		_en_NoTest = 0,_en_ExhaustRate,_en_LeakRate
	}m_enTestingVacuum;

public:
	CString     m_slngFile;		 //最终选定的语言文件名称．
	CStringList m_slistlngMenu;  //供选择的Menu显示字符串List

public:
	//页面名称
	CString		m_sTabLabel[_OA_FORM_VIEW_NUMBER];
	CString     m_sTabTooltip[_OA_FORM_VIEW_NUMBER];

public:
	//plc连接状态 0: 串口处于关闭状态. 1: 串口打开错误. 2:串口已经打开，正在进行连接测试. 3:串口打开，连接plc正常. 4:串口打开，连接plc不正常	
	int m_iPlcState; 

	//函数
public:
	BOOL fn_IsLngFileExist(void);         //确认 .lng文件可读取，但尚不确认内容
	BOOL fn_ReleaseInnerLng(void); //供fn_IsLngFileExist调用
public:
	BOOL fn_InitText(void);
	bool fn_InitLngMenuText(void);
public:
	bool fn_WriteCurLngInt(int iCurLng);
public:
	LPCTSTR fn_GetLocalTimeStr(void);
public:
	static double fn_TransPenningVoltPa(double dVolt);
public:
	void fn_CreateCommError(_sPlcCommError& sCommErr);
public:
	
public:
	static int fn_LinearRegression(double& a, double& b, double* x, double* y, int n);
};
