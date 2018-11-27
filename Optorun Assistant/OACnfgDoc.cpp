#include "StdAfx.h"
#include "OACnfgDoc.h"
#include "resource.h"       // main symbols
#include <math.h>

//软件版本，显示在基本情况内
const TCHAR* g_strProductVersion = TEXT("1.05.2010.09.15");//TEXT("1.04.2010.06.01");//TEXT("2008.07.15");//TEXT("2008.04.02");//TEXT("2008.01.02");
//version : 1.04.2010.06.01", 增加 PEG100对应. 
//version : 1.05.2010.09.15, 增加 Ulvac 电离真空计 BMR2\SC1 (APC用)对应.
const double g_dProgramVersion = 1.05; //1.04->PEG100//1.02->2009.12.14 对应S702、S703环形坩埚慢转动
//const TCHAR*  g_strLastProgramDate = TEXT("PROGRAM DATA: 2007.12.10"); //
const double g_dDataVersion = 1.01;

//脱离硬件进行调试使用。 在界面上有明显提示。
//软件相同，需要脱机调试时，每次更改该设置。
bool  g_bOffLineDebug = false;

enum enumOptorunCoaterType g_enOptorunCoaterType = _en_Coater_UNKNOWN_TYPE;

//#define _OA_STR_SYSTEM_NUMBER
CString g_strSystem[_OA_STR_SYSTEM_NUMBER];//退出时出错报警。由xml报警

CODMCom g_COM2_ODM;
CGasController g_COM4_GasController;
CVGC500  g_COM5_VGC500;
CIBGLCom g_COM3_IBGL;
CSysmacPLC g_plc;
CVacuumProcessThread* g_pVacProThread = NULL ;
CPlcRdWrThread*       g_pPlcRdWrThread = NULL;
CIonGauge  g_IonGauge; 
CAPCGauge  g_APCGauge;
HANDLE g_hEvent_plc; //事件，通过串口访问plc，必须取得控制权

CRITICAL_SECTION gCriticalSection_State; //临界区，用于程序状态控制，在构造函数中初始化。
CRITICAL_SECTION gCriSec_Vacuum; //临界区，用于真空相关状态控制，在构造函数中初始化。 主线程子线程都需要进行

CRITICAL_SECTION gCriSec_UI;     //临界区，用于(用户接口)界面设置变量时的控制，在构造函数中初始化。

int    g_iVacuumExhaustDataCurrent = 0;
int    g_iVacuumExhaustDataItems = 16;    //抽速测试的真空数据总数		20160704增加OTFC2350 MTFC900
int    g_iVacuumExhaustDataHighItems = 7; //抽速测试数据中，低真空时的数据量（此时，程序不能测量到真空度）。用户自填写或程序辨识开关状态.
struct stVacuumExhaustData* g_pstVacuumExhaustData = NULL;

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
#define _3876_00_CHAMBER_DOOR 0
#define _3876_11_EXHAUST 11 //2007.12.20,未用
#define _3876_14_RP_ON	 14
#define _3876_15_MBP_ON  15


#define _3877_02_FV1     2
#define _3877_04_RV      4
#define _3877_05_SRV     5
#define _3877_06_MV      6
#define _3877_08_LV      8
#define _3877_09_SLV     9
WORD stVacuumExhaustData::wTP_1_Mask = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON)|(1<<_3876_15_MBP_ON);
WORD stVacuumExhaustData::wTP_2_Mask = (1<<_3877_02_FV1)|(1<<_3877_04_RV)|(1<<_3877_05_SRV)|(1<<_3877_06_MV)|(1<<_3877_08_LV)|(1<<_3877_09_SLV);


struct stVacuumTestEx    g_stVacuumTestEx;
//0:没有等待关系
//1:测试线程正在等待主线程响应
//2:主线程已经响应

int g_iMachineType = _MACHINE_TYPE_GENER; //镀膜机类型，默认为Gener. 在查找机器名称时进行初始化。目前仅用来初始化镀膜机体积选择时的容积。

struct stVacuumTestState gi_stVacuumTestState; //在 COACnfgDoc 中初始化

//CRITICAL_SECTION gCriticalSection_Hearth; 
//bool  volatile gb_UsingIonGauge = true; //测试过程是否使用IonGauge，默认使用。是否有IonGauge请查找 g_IonGauge type.
bool  gb_VacuumPercent  = false; //是否是百分比形式存储真空度数据

const TCHAR* g_strLngPath = TEXT("\\language");
TCHAR g_strBuffer[1024]; //只能在主线程当缓冲区用
TCHAR g_strTimeBuffer[256];//专门用于时间
const TCHAR* g_strFileData = _T("OA_FILE_DATA");
const TCHAR* g_strEnglishName  = _T("OA_English.lng");
const TCHAR* g_strChineseName  = _T("OA_Chinese.lng");
const TCHAR* g_strLngNameTplt = TEXT("LNG_%d_NAME"); //name template
const TCHAR* g_strxmlRoot = TEXT("OPTORUN");

#define PLCDM_ADDRESS
//有些地址值直接写在函数中.
PLCDM_ADDRESS const WORD   gc_DM_machinetype   = 9000;
PLCDM_ADDRESS const WORD   gc_DM_sequencer     = 9010;
PLCDM_ADDRESS const WORD   gc_DM_touchpanel    = 9020;

PLCDM_ADDRESS const WORD   gc_DM_VacuumType    = 9066; // 0001 是 %, 0002 是 Pa. 应该与电压值没关系.本软件单位为Pa
PLCDM_ADDRESS const WORD   gc_DM_IonGaugeType  = 9085; //电离真空计类别, 0=没有,1=GI-M2,2=IOZ_M832,3=IOZ_BPG400,4=GI-D7, 5=IOZ_BPG402+VGC403?
													   //对应单体的电离真空计，或5，此时占用原ch2
PLCDM_ADDRESS const WORD   gc_DM_APCGaugeType  = 9088; //APC用真空计类别，2008.05.12添加. 对应 G1275 使用 VGC403，但ch2仍使用 皮拉尼规PSG500(不能测量高真空).
                                                       //对应ch3
PLCDM_ADDRESS const WORD   gc_DM_IonGaugeValue = 6587;
PLCDM_ADDRESS const WORD   gc_DM_PenningValue  = 3400; //ch3, (3400) = (3421)*4


//从9069开始连续四个地址，分别为 monitor Heater温度现在值地址，DomeHeater现在值地址
//                               monitor Heater目标值，DomeHeater目标值
PLCDM_ADDRESS const WORD   gc_DM_AddressHeaterTempSVPV = 9069;
PLCDM_ADDRESS const WORD   gc_DM_AddressDome2TempSVPV = 9079;  //侧壁
//以下四个地址参数在联机后进行初始化.
PLCDM_ADDRESS WORD gcp_DM_HeaterTemp[_TEMP_HEATER_NUMBER]={4210,4128,4215,4132,4180,4182};
//多出的2个是可能存在的侧壁地址.

PLCDM_ADDRESS WORD gc_DM_VacPump = 9053; //高真空pump种类 (.00==1: 扩散泵) (.01=1:dry pump).本软件只识别 扩散泵?
PLCDM_ADDRESS WORD gc_DM_HAS_DP2 = 9097; //有无DP2 ? 0=无，1=有. 


const int gc_plc_str_length = 10;
const COLORREF gc_clrBlack  = RGB(0,0,0);
const COLORREF gc_clrTrying = RGB(128,128,0);
const COLORREF gc_clrOK     = RGB(0,128,0);
const COLORREF gc_clrAlert  = RGB(255,0,0);
const COLORREF gc_clrWhite  = RGB(255,255,255);
const COLORREF gc_clrRunning = RGB(255,0,0);

//4199 and 176参考自 plc,  内部记录及显示时作基准，超出范围的用 ********Pa表示
double   gc_dMaxPenningPa = 1.0E+6;// = COACnfgDoc::fn_TransPenningVoltPa(4199-1);
double   gc_dMinPenningPa = 1.0E-9;//COACnfgDoc::fn_TransPenningVoltPa(176+1);
//
//Ion gauge 上下限在 CIonGauge中声明，并可改变，外部不可对此进行改变
//extern const double gc_dMaxIonGaugePa;
//extern const double gc_dMinIonGaugePa;


//自动测试过程中，由于突然plc联系失败，在停止下来之前，自动最多重试次数
const int      gc_iMaxPLCtryTimes = 3; 
int            g_icurPLCtryTimes = 0; //当前重试次数

//item1(就是COASysInfoView)需要从 xml 文件中读取文本的 窗口ID 数量//本数量必须随时手工添加
//#define      _ITEM1_NEED_TEXT_ID_CNT 2 //定义在"OACnfgDoc.h"中，免得各 FormView同时更新
//种类1固定需要某种文本的窗口
const int    g_item1_ID[_ITEM1_NEED_TEXT_ID_CNT] 
				= { IDC_BUTTON_CONNECT_PLC,IDC_BUTTON_CLOSE_CONNECTION,IDC_STATIC_CUR_STATE_TITLE,IDC_ITEM1_SYS_GROUP,IDC_STATIC_PRODUCT_VERSION,IDC_STATIC_PC_NAME,
					IDC_STATIC_MACHINE_TYPE,IDC_STATIC_SEQUENCER,IDC_STATIC_TOUCHPANEL_VER,IDC_STATIC_HELP_TITLE,IDC_STATIC_HISTORY}; //ID号序列 //需要在 view中 extern
//在 xml 文件中的子项名称(本子项在 item1 element 之下). 必须一致
const TCHAR* g_item1_ID_Name[_ITEM1_NEED_TEXT_ID_CNT] = {TEXT("connectPLC"),TEXT("disconnectPLC"),TEXT("curPLCstate"),TEXT("sysGroup"),TEXT("productVer"),TEXT("pcName"),TEXT("machineType"),TEXT("sequencerVer"),TEXT("touchpanelVer"),TEXT("helpTitle"),TEXT("runHistory")}; //
CString g_item1_ID_Text[_ITEM1_NEED_TEXT_ID_CNT]; //需要在 view中 extern
//item1 种类2某几窗口需要多个文本切换

//item1 种类3
int giHelpItems = _ITEM1_NEED_HELP_TEXT;
CString g_item1_Help_Text[_ITEM1_NEED_HELP_TEXT];
CString g_item1_plc_state[_ITEM1_COM_PLC_STATE_CNT];


//item2 g_item2_ID与g_item2_ID_Name 数组内容必须保持同样尺寸,同步更新. //在 doc构造函数中添加了ASSERT确认.IDC_STATIC_EXHAUST_NEED_TIME_TEXT
const int    g_item2_ID[]
				= {IDC_STATIC_PENNING_VALUE_NAME,IDC_BTN_START_VACUUM_TEST,IDC_BTN_CANCEL_VACUUM_TEST,
					IDC_STATIC_COMMENT_TEXT,IDC_RADIO_LEAK_RATE,IDC_RADIO_EXHAUST,IDC_RADIO_RECORDER,
					IDC_STATIC_COATER_TEXT,IDC_STATIC_INTERVAL_TEXT,IDC_STATIC_RECORD_TIME_TEXT,IDC_STATIC_ELAPSED_TIME_TEXT,
					IDC_STATIC_LEAKRATE_START_PRESSURE_TEXT,IDC_STATIC_LEAKRATE_WAIT_TIME_TEXT,IDC_STATIC_LEAKRATE_KEEP_TIME_TEXT,IDC_EDIT_VACUUM_HELP,
				    IDC_BTN_ADD_VACUUM_FILE,IDC_BTN_CURVE_PROPERTY,IDC_BTN_DELETE_CURVE	};
//在 xml 文件中的子项名称(本子项在 item1 element 之下). 必须一致
const TCHAR* g_item2_ID_Name[]
				= {TEXT("penningName"),TEXT("startTest"),TEXT("cancelTest"),TEXT("Comment"),TEXT("radioLeak"),TEXT("radioExhaust"),
					TEXT("radioRecorder"),TEXT("coaterVolume"),TEXT("recordInterval"),TEXT("recordTime"),
					TEXT("elapsedTime"),TEXT("expectPressure"),TEXT("maxWaitTime"),TEXT("pressurekeepTime"),TEXT("helpInittext"),
				    TEXT("addFiles"),TEXT("curvePro"),TEXT("delCurve")}; //
const int  gc_item2_Text_ID_CNT = sizeof(g_item2_ID)/sizeof(int);//ITEM2 需要从 language中读取的id数量。_ITEM2_NEED_TEXT_ID_CNT;

CString g_item2_ID_Text[gc_item2_Text_ID_CNT]; //需要在 view中 extern
CString g_item2_Vac_state[_ITEM2_VAC_STATE_CNT];
CString g_item2_String[_ITEM2_OTHER_STR_NUMBER];//item2中需要使用到的额外字符串

//CString g_dlgVacuumThreadError_Str[_DLGVACTHRERROR_STR_NUMBER];

bool  g_bRecordVacuum = true; //是否记录当前的真空度，开始时默认记录（连接上plc后自动）。在真空度页面内用 check button设置

//item3
const int g_item3_ID[] = {IDC_BTN_INIT_HEARTH,IDC_CHECK_START_FROM_SELECTION,IDC_BTN_CREATE_HEARTH1,IDC_BTN_INSERT_HEARTH1,
			IDC_BTN_CREATE_HEARTH2,IDC_BTN_INSERT_HEARTH2,IDC_BTN_DELETE_HEARTH_ROW,IDC_BTN_SAVE_HEARTH_LIST,IDC_BTN_START_HEARTH_TEST,
			IDC_BTN_CANCEL_HEARTH_TEST,IDC_CHECK_HEARTH1_DIRECTION,IDC_CHECK_HEARTH2_DIRECTION,IDC_STATIC_MONITOR_ALERT,
			IDC_BTN_INSERT_DOME_SPEED,IDC_BTN_AUTOCREATE_DOME_SPEED,IDC_CHECK_START_FROM_SELECTION_DOME,IDC_BTN_DELETE_DOME_ROW,IDC_BTN_SAVE_DOME_LIST,
			IDC_BTN_START_DOME_TEST,IDC_BTN_CANCEL_DOME_TEST};
const TCHAR* g_item3_ID_Name[] = {TEXT("initHearth"),TEXT("startFromSelRow"),TEXT("autoCreateHearth1"),TEXT("insertHearth1"),
			TEXT("autoCreateHearth2"),TEXT("insertHearth2"),TEXT("delListRow"),TEXT("saveList"),TEXT("startHearthTest"),
			TEXT("cancelHearthTest"),TEXT("h1DrtCheck"),TEXT("h2DrtCheck"),TEXT("monitorAlert"),
			TEXT("insertDomeSpeed"),TEXT("autoCreateDomeSpeed"),TEXT("startDomeFromSelRow"),TEXT("delDomeTestRow"),TEXT("saveDomeList"),
			TEXT("startDomeTest"),TEXT("cancelDomeTest")};
const int gc_item3_Text_ID_CNT = sizeof(g_item3_ID)/sizeof(int);
CString g_item3_ID_Text[gc_item3_Text_ID_CNT]; //需要在 view中 extern		
CString g_item3_String[_ITEM3_OTHER_STR_NUMBER];//item2中需要使用到的额外字符串

#ifdef _ZHANGZIYE_DEBUG
//debug
CArray <int,int&> g_item1_ID_Text2;
#endif

_sVacuumPressure gs_VacuumPressure;
_sPlcCommError gs_plcCommError;

//真空度空间大小
//#define  _VACUUM_PRESSURE_MAX (2*24*3600)
int    g_iVacuumTestCnt     = 0;     //每次测试开始后数据到来的次数. //同秒数 
int    g_iVacuumStartPos    = 0;     //每次测试的数据位于 存储空间的起始位置. 

int    g_iVacuumPressurePos = 0;     //当前可用于存储真空度的位置.
double g_dVacuumVGCCh3[_VACUUM_PRESSURE_MAX];   //存储 VGC023 ch3 (penning)真空度（已经转化成Pascal）
double g_dVacuumIonGauge[_VACUUM_PRESSURE_MAX]; //存储 Ion Gauge (penning)真空度（已经转化成Pascal）
                                                //在VGC403之后，g_dVacuumIonGauge变成了存储 VGC403的ch2.
double g_dVacuumTime[_VACUUM_PRESSURE_MAX];     //存储记录时刻，用于显示。此处是指距离起始位置的时间差，单位为秒。


WORD   g_wVacuumRdAddress[_VACUUM_READ_PLC_ADDRESS_CNT];
bool   g_bVacuumRdDecimal[_VACUUM_READ_PLC_ADDRESS_CNT]; //是否采取 10 进制读取
int    g_iVacuumRdDM_IO[_VACUUM_READ_PLC_ADDRESS_CNT];   //0代表 DM，否则为　I/O

bool   g_bUseIongauge = true; //如果有IonGauge,是否在测试抽速的过程中使用　Iongauge，默认使用.
CZYGraphPlot g_CZYGraphPlot; //2008.04.08添加，真空度数据显示用

//界面 开始，停止
//主程序消息响应里开始，停止
//监测程序里在  循环内检测该变量，当为 false时 退出。
CRITICAL_SECTION gCriticalSection_HearthState;
bool   g_bTestHearthRotation = false; //是否正在进行测试坩埚旋转
bool   g_bTestHearthDataSaved = true;

CRITICAL_SECTION gCriticalSection_DomeState;
bool   g_bTestDomeRotation = false; //是否正在进行Dome旋转测试
bool   g_bTestDomeStopByUser = false;//Dome停止测试的原因是用户停止.


//开发人员 debug 一些通常的参数，软件通常不需要对使用人员开放这些功能。
//开发人员可以通过一定的方式使得 gb_xxx变为 true.
bool gb_ProgrammerDebugNormal = true; //将来如觉得有必要，可在编译时就设定这些量为true.

COACnfgDoc::COACnfgDoc(void):m_iPlcState(_COM_PLC_STATE_NOT_CONNECT)
{
	//确保g_item2_ID与g_item2_ID_Name 数组内容保持同样尺寸
	ASSERT( gc_item2_Text_ID_CNT == sizeof(g_item2_ID_Name)/sizeof(TCHAR*) );
	ASSERT( gc_item3_Text_ID_CNT == sizeof(g_item3_ID_Name)/sizeof(TCHAR*) );
	
	gs_VacuumPressure.dVacuumIonGaugePa = 1.0E+5;
	gs_VacuumPressure.dVacuumPenningPa  = 1.0E+5;
	m_enTestingVacuum = _en_NoTest;


	//启动plc读取线程
//	g_pPlcRdWrThread = (CPlcRdWrThread*)AfxBeginThread(RUNTIME_CLASS(CPlcRdWrThread), THREAD_PRIORITY_NORMAL,0,0);


	//debugging，放到 Vacuum View中启动
	//开启真空度相关监测线程
	//g_pVacProThread = (CVacuumProcessThread*)AfxBeginThread(RUNTIME_CLASS(CVacuumProcessThread), THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	//g_pVacProThread  = (CVacuumProcessThread*)AfxBeginThread(RUNTIME_CLASS(CVacuumProcessThread), THREAD_PRIORITY_NORMAL,0,0);


	//创建 plc访问事件，用于控制一个时间内只有一个线程操作 串口
	g_hEvent_plc = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetEvent(g_hEvent_plc);

	//初始化 状态控制用 临界区，凡是要访问 状态变量 bVacuumTest等，必须先取得临界区控制.
	/*
	EnterCriticalSection(&gCriticalSection);
	//do something
	LeaveCriticalSection(&gCriticalSection);
	*/
	InitializeCriticalSection(&gCriticalSection_State);
	InitializeCriticalSection(&gCriSec_Vacuum);
	InitializeCriticalSection(&gCriSec_UI);
	InitializeCriticalSection(&gCriticalSection_HearthState);
	InitializeCriticalSection(&gCriticalSection_DomeState);
	

	//初始化gi_stVacuumTestState
	gi_stVacuumTestState.bTest = false;
	gi_stVacuumTestState.iMainRouting = 0;
	gi_stVacuumTestState.iSubRouting = 0;
	gi_stVacuumTestState.iThreadState = 0;
	gi_stVacuumTestState.iTryTimes = 0;

	m_iCurLng = -1;
	m_iLngCnt = -1;
	//m_iPlcState = _COM_PLC_STATE_NOT_CONNECT;
		
#ifdef _ZHANGZIYE_DEBUG
	try
	{
		int x  = g_item1_ID_Text2.GetSize();
	}
	catch(...)
	{
		int x = 3;
	}
#endif

	g_stVacuumTestEx.bTesting = false;
	g_stVacuumTestEx.iCurTestType = _VACUUM_TESTTYPE_RECORDER;     //默认真空度测试类型为记录仪
	g_stVacuumTestEx.iRecordInterval = 60; //60 秒钟
	g_stVacuumTestEx.iRecordTime = 3600;   //60 分钟
	//g_stVacuumTestEx.sExhaust.

	//供Vacuum测试线程使用.
	g_wVacuumRdAddress[_VACUUM_READ_PLC_ADDRESS_SN_VGC] = gc_DM_PenningValue; //3400
	g_wVacuumRdAddress[_VACUUM_READ_PLC_ADDRESS_SN_IONGAUGE] = gc_DM_IonGaugeValue;//6587;
	g_wVacuumRdAddress[_VACUUM_READ_PLC_ADDRESS_SN_TP_1] = 3876;
	g_wVacuumRdAddress[_VACUUM_READ_PLC_ADDRESS_SN_TP_2] = 3877;

	g_bVacuumRdDecimal[_VACUUM_READ_PLC_ADDRESS_SN_VGC] = true;  //10进制
	g_bVacuumRdDecimal[_VACUUM_READ_PLC_ADDRESS_SN_IONGAUGE] = true;
	g_bVacuumRdDecimal[_VACUUM_READ_PLC_ADDRESS_SN_TP_1] = false; //16进制
	g_bVacuumRdDecimal[_VACUUM_READ_PLC_ADDRESS_SN_TP_2] = false;

	g_iVacuumRdDM_IO[_VACUUM_READ_PLC_ADDRESS_SN_VGC] = 0; //DM
	g_iVacuumRdDM_IO[_VACUUM_READ_PLC_ADDRESS_SN_IONGAUGE] = 0; //DM
	g_iVacuumRdDM_IO[_VACUUM_READ_PLC_ADDRESS_SN_TP_1] = 1; // I/O
	g_iVacuumRdDM_IO[_VACUUM_READ_PLC_ADDRESS_SN_TP_2] = 1; // I/O


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
	g_pstVacuumExhaustData = new stVacuumExhaustData[g_iVacuumExhaustDataItems];
	g_pstVacuumExhaustData[0].bPressureExist = true;
	g_pstVacuumExhaustData[0].dPressure = 1.0E+05;
	g_pstVacuumExhaustData[0].enRecordType = stVacuumExhaustData::_enumAction;
	g_pstVacuumExhaustData[0].sActionName = TEXT("SRV OP");
	g_pstVacuumExhaustData[0].wTP1_OK_Value = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON); // 记录１的位
	g_pstVacuumExhaustData[0].wTP2_OK_Value = (1<<_3877_05_SRV);

	g_pstVacuumExhaustData[1].bPressureExist = false;
	g_pstVacuumExhaustData[1].enRecordType = stVacuumExhaustData::_enumAction_CH2;
	g_pstVacuumExhaustData[1].sActionName = TEXT("RV OP");
	g_pstVacuumExhaustData[1].wTP1_OK_Value = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON); // 记录１的位
	g_pstVacuumExhaustData[1].wTP2_OK_Value = (1<<_3877_04_RV) | (1<<_3877_05_SRV) ;

	g_pstVacuumExhaustData[2].bPressureExist = true;
	g_pstVacuumExhaustData[2].dPressure = 1.0E+04;
	g_pstVacuumExhaustData[2].enRecordType = stVacuumExhaustData::_enumAction;																//2018-9-14 Zhouyi
	g_pstVacuumExhaustData[2].sActionName = TEXT("MBP ON");																					//2018-9-14 Zhouyi
	g_pstVacuumExhaustData[2].wTP1_OK_Value = (1 << _3876_00_CHAMBER_DOOR) | (1 << _3876_14_RP_ON) | (1 << _3876_15_MBP_ON); // 记录１的位	//2018-9-14 Zhouyi
	g_pstVacuumExhaustData[2].wTP2_OK_Value = (1<<_3877_04_RV) | (1<<_3877_05_SRV) ;														//2018-9-14 Zhouyi

	//g_pstVacuumExhaustData[2].enRecordType = stVacuumExhaustData::_enumNotRecord_CH2;													//2018-9-14 Zhouyi
	//2007.12.07, MBP有变频器的情况下　从 1.3E+3开始启动，否则从 1.0E+4启动																//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[2].sActionName = TEXT("MBP ON");																			//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[2].wTP1_OK_Value = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON) | (1<<_3876_15_MBP_ON); // 记录１的位	//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[2].wTP2_OK_Value = (1<<_3877_04_RV) | (1<<_3877_05_SRV) ;													//2018-9-14 Zhouyi

	g_pstVacuumExhaustData[3].bPressureExist = true;
	g_pstVacuumExhaustData[3].dPressure = 1.3E+03;
	g_pstVacuumExhaustData[3].enRecordType = stVacuumExhaustData::_enumNotRecord_CH2;													//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[3].enRecordType = stVacuumExhaustData::_enumAction;														//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[3].sActionName = TEXT("MBP ON");																			//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[3].wTP1_OK_Value = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON) | (1<<_3876_15_MBP_ON); // 记录１的位	//2018-9-14 Zhouyi
	//g_pstVacuumExhaustData[3].wTP2_OK_Value = (1<<_3877_04_RV) | (1<<_3877_05_SRV);													//2018-9-14 Zhouyi


	g_pstVacuumExhaustData[4].bPressureExist = true;
	g_pstVacuumExhaustData[4].dPressure = 1.0E+01;
	g_pstVacuumExhaustData[4].enRecordType = stVacuumExhaustData::_enumNotRecord_CH2;

	g_pstVacuumExhaustData[5].bPressureExist = true;
	g_pstVacuumExhaustData[5].dPressure = 6.6E+0;
	g_pstVacuumExhaustData[5].enRecordType = stVacuumExhaustData::_enumAction;
	g_pstVacuumExhaustData[5].sActionName = TEXT("SRV RV CL");
	g_pstVacuumExhaustData[5].wTP1_OK_Value = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON) | (1<<_3876_15_MBP_ON); // 记录１的位
	g_pstVacuumExhaustData[5].wTP2_OK_Value = 0 ;

	g_pstVacuumExhaustData[6].bPressureExist = false;
	g_pstVacuumExhaustData[6].enRecordType = stVacuumExhaustData::_enumAction_CH2;
	g_pstVacuumExhaustData[6].sActionName = TEXT("MV OP");
	g_pstVacuumExhaustData[6].wTP1_OK_Value = (1<<_3876_00_CHAMBER_DOOR)|(1<<_3876_14_RP_ON) | (1<<_3876_15_MBP_ON); // 记录１的位
	g_pstVacuumExhaustData[6].wTP2_OK_Value = (1<<_3877_02_FV1) | (1<< _3877_06_MV) ;

	g_pstVacuumExhaustData[7].bPressureExist = true;
	g_pstVacuumExhaustData[7].dPressure = 3.0E-3;
	g_pstVacuumExhaustData[7].enRecordType = stVacuumExhaustData::_enumPressureVGC;

	g_pstVacuumExhaustData[8].bPressureExist = true;
	g_pstVacuumExhaustData[8].dPressure = 2.0E-3;
	g_pstVacuumExhaustData[8].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[9].bPressureExist = true;
	g_pstVacuumExhaustData[9].dPressure = 1.3E-3;
	g_pstVacuumExhaustData[9].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[10].bPressureExist = true;
	g_pstVacuumExhaustData[10].dPressure = 1.0E-3;
	g_pstVacuumExhaustData[10].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[11].bPressureExist = true;
	g_pstVacuumExhaustData[11].dPressure = 5.0E-4;
	g_pstVacuumExhaustData[11].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[12].bPressureExist = true;
	g_pstVacuumExhaustData[12].dPressure = 1.5E-4;
	g_pstVacuumExhaustData[12].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[13].bPressureExist = true;
	g_pstVacuumExhaustData[13].dPressure = 1.0E-4;
	g_pstVacuumExhaustData[13].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[14].bPressureExist = true;
	g_pstVacuumExhaustData[14].dPressure = 1.0E-4;
	g_pstVacuumExhaustData[14].enRecordType = stVacuumExhaustData::_enumPressureBoth;

	g_pstVacuumExhaustData[15].bPressureExist = true;
	g_pstVacuumExhaustData[15].dPressure = 5.0E-5;
	g_pstVacuumExhaustData[15].enRecordType = stVacuumExhaustData::_enumPressureBoth;
//stVacuumExhaustData* g_pstVacuumExhaustData
}

COACnfgDoc::~COACnfgDoc(void)
{
	DeleteCriticalSection(&gCriticalSection_State);
	CloseHandle(g_hEvent_plc);

	delete [] g_pstVacuumExhaustData;
}


BOOL COACnfgDoc::fn_IsLngFileExist(void)
{
	//fn_ReleaseInnerLng(); //debug
	CWinApp* pApp = AfxGetApp();
	if(NULL==pApp)
	{
		return false;
	}

	m_iCurLng = pApp->GetProfileInt(TEXT("LANGUAGE"),TEXT("LNG_CURRENT"),1);
	m_iLngCnt = pApp->GetProfileInt(TEXT("LANGUAGE"),TEXT("LNG_COUNT"),0);
	if( m_iLngCnt< 0 )
	{
		return fn_ReleaseInnerLng();
	}

	if(m_iCurLng > m_iLngCnt)
	{
		m_iCurLng = 1;
	}

	//确认语言文件存在
	for(int i = m_iCurLng; i <= m_iLngCnt; i++)
	{
		_stprintf(g_strBuffer,TEXT("LNG_%d_FILE"),i);
		CString strFile = pApp->GetProfileString(TEXT("LANGUAGE"),g_strBuffer,NULL);
		if(strFile.GetLength() != 0)
		{
			CString str = pApp->m_pszProfileName;
			int nPos = str.ReverseFind(TEXT('\\'));
			str = str.Left(nPos);
			m_slngFile = str.Left(nPos) + TEXT("\\language\\") + strFile;

			HANDLE hFile = CreateFile(m_slngFile,0,FILE_SHARE_READ,0,OPEN_EXISTING,
				FILE_ATTRIBUTE_READONLY,NULL);
			if(GetLastError()==0)
			{//如果打开文件成功
				if( i != m_iCurLng )
				{//如果当前打开的文件序号与 配置不符合，则重新写 ini文件
					m_iCurLng = i;
					pApp->WriteProfileInt(TEXT("LANGUAGE"),TEXT("LNG_CURRENT"),m_iCurLng);
					AfxMessageBox(g_strSystem[_OA_STR_SYSTEM_LNG_FILE_NOT_EXIST].IsEmpty()?
						TEXT("选定的语言文件没有找到，请检查配置文件(OptorunAssistant.ini).\r\nlanguage file not exist. Please check \"OptorunAssistant.ini\" file"):g_strSystem[_OA_STR_SYSTEM_LNG_FILE_NOT_EXIST]);
					//AfxMessageBox(TEXT("Preselected language file not exist. Please check \"OptorunAssistant.ini\" file"));
				}

				CloseHandle(hFile);
				return true;
			}
		}
	}
	//上述循环内没有找到，则从文件里释放资源
	return fn_ReleaseInnerLng();
}

BOOL COACnfgDoc::fn_ReleaseInnerLng(void)
{
		CWinApp* pApp = AfxGetApp();
	
		//1, 判断路径是否存在
		CString strPath = pApp->m_pszProfileName;
		int nPos = strPath.ReverseFind(TEXT('\\'));
		strPath  = strPath.Left(nPos);
		strPath += TEXT("\\language\\");

		BOOL b = CreateDirectory(strPath,NULL);		
		if( !b )
		{
			DWORD dw = GetLastError();
			if( dw != 183 )
			{//123, 目录名不正确．　183，目录已经存在，不能再创建！
				return false;
			}
		}

		//2, 释放　OA_English.lng
		HRSRC hSrc = FindResource(NULL,MAKEINTRESOURCE(IDR_OA_ENGLISH_LNG),g_strFileData);
		if(hSrc == NULL)	
			return false;

		HGLOBAL hGlobal = LoadResource(NULL,hSrc);
		if(hGlobal == NULL)	
			return false;

		LPVOID lp = LockResource(hGlobal);
		DWORD dwSize = SizeofResource(NULL,hSrc);

		CString strFile;
		strFile = strPath + g_strEnglishName;	
	
		CFile file;
		if( file.Open(strFile, CFile::modeCreate|CFile::modeWrite))
		{
			file.Write(lp,dwSize);
			file.Close();
		}		
		FreeResource(hGlobal);
		//m_iLgCnt = 2;
		m_iCurLng = 1;
		m_slngFile = strFile;//english

		//3, 释放　OA_Chinese.lng
		hSrc = FindResource(NULL,MAKEINTRESOURCE(IDR_OA_CHINESE_LNG),g_strFileData);
		if(hSrc == NULL)	
			return false;

		hGlobal = LoadResource(NULL,hSrc);
		if(hGlobal == NULL)	
			return false;

		lp = LockResource(hGlobal);
		dwSize = SizeofResource(NULL,hSrc);
		strFile = strPath + g_strChineseName;
	
		if( file.Open(strFile, CFile::modeCreate|CFile::modeWrite))
		{
			file.Write(lp,dwSize);
			file.Close();
		}		
		FreeResource(hGlobal);


		//4, 初始化m_lngFilename，并重写 ini文件
		m_iLngCnt  = 2;		
		m_iCurLng  = 2;
		m_slngFile = strFile;//


		pApp->WriteProfileInt(TEXT("LANGUAGE"),TEXT("LNG_CURRENT"),m_iLngCnt);
		pApp->WriteProfileInt(TEXT("LANGUAGE"),TEXT("LNG_COUNT"),2);

		_stprintf(g_strBuffer,g_strLngNameTplt,1);
		pApp->WriteProfileString(TEXT("LANGUAGE"),g_strBuffer,TEXT("English"));
		_stprintf(g_strBuffer,TEXT("LNG_%d_FILE"),1);
		pApp->WriteProfileString(TEXT("LANGUAGE"),g_strBuffer,g_strEnglishName);

		_stprintf(g_strBuffer,g_strLngNameTplt,2);
		pApp->WriteProfileString(TEXT("LANGUAGE"),g_strBuffer,TEXT("简体中文"));
		_stprintf(g_strBuffer,TEXT("LNG_%d_FILE"),2);
		pApp->WriteProfileString(TEXT("LANGUAGE"),g_strBuffer,g_strChineseName);

		return true;
}

bool COACnfgDoc::fn_InitLngMenuText(void)
{
	CWinApp* pApp = AfxGetApp();
	if(NULL==pApp)
		return false;

	if( 0 != m_slistlngMenu.GetCount())
	{
		m_slistlngMenu.RemoveAll();
	}

	CString str;
	for(int i = 1; i <= m_iLngCnt; i++)
	{
		_stprintf(g_strBuffer,g_strLngNameTplt,i);
		str = pApp->GetProfileString(TEXT("LANGUAGE"),g_strBuffer,NULL);
		m_slistlngMenu.AddTail(str);
	}
		
	return true;
}

//初始化
BOOL COACnfgDoc::fn_InitText(void)
{
	if(!fn_IsLngFileExist())
	{
		return false;
	}
	fn_InitLngMenuText(); //OK

	bool b = xml.Load(m_slngFile);
	if(	!b )
	{
		//debug, 直接　false 可导致　不能启动
		_stprintf(g_strBuffer,TEXT("ERROR: Languange file <%s> cannot be load correctly!\r\n\r\nCAUSE: %s"),m_slngFile,xml.GetError());
		AfxMessageBox(g_strBuffer);
		//AfxMessageBox(xml.GetError());
		return true;//false;
	}

	if(xml.FindChildElem(TEXT("sysString")))
	{
		xml.IntoElem();
		int iSysStr = __min(_OA_STR_SYSTEM_NUMBER,_ttoi(xml.GetAttrib(TEXT("count"))));
		for(int i = 0;i<iSysStr;i++)
		{
			_stprintf(g_strBuffer,TEXT("str%d"),i+1);
			if(xml.FindChildElem(g_strBuffer))
			{		
				g_strSystem[i] = xml.GetChildData();								
			}			
		}
		xml.OutOfElem();
	}

	for(int i=0;i< _OA_FORM_VIEW_NUMBER;i++)
	{
		_stprintf(g_strBuffer,TEXT("item%d"),i+1);
		if ( xml.FindChildElem(g_strBuffer) )
		{
			xml.IntoElem();

			m_sTabLabel[i]   = xml.GetAttrib(TEXT("tabname"));
			m_sTabTooltip[i] = xml.GetAttrib(TEXT("tooltip"));
			switch(i)
			{
			case _OA_FORM_VIEW_SYSTEM: //0				
				if(xml.FindChildElem(TEXT("PlcState")))
				{
					xml.IntoElem();
					for(int j=0;j<_ITEM1_COM_PLC_STATE_CNT;j++)
					{//必须依次查找和填写
						_stprintf(g_strBuffer,TEXT("state%d"),j+1);
						g_item1_plc_state[j] = xml.GetAttrib(g_strBuffer);										
					}
					xml.OutOfElem();
				}

				if(xml.FindChildElem(TEXT("constID")))
				{
					xml.IntoElem();
					for(int j=0;j<_ITEM1_NEED_TEXT_ID_CNT;j++)
					{//必须依次查找和填写
						if(xml.FindChildElem(g_item1_ID_Name[j]))
						{		
							g_item1_ID_Text[j] = xml.GetChildData();										
						}
					}
					xml.OutOfElem();
				}

				if(xml.FindChildElem(TEXT("helpStr")))
				{
					xml.IntoElem();
					giHelpItems = __min(_ITEM1_NEED_HELP_TEXT,_ttoi(xml.GetAttrib(TEXT("count"))));
					_stprintf(g_strBuffer,TEXT("help"));
					for(int j=0;j<giHelpItems;j++)
					{//必须依次查找和填写
						//_stprintf(g_strBuffer,TEXT("help%d"),j+1);
						if(xml.FindChildElem(TEXT("help")))
						{		
							g_item1_Help_Text[j] = xml.GetChildData();										
						}
					}
					xml.OutOfElem();
				}				
				break;

			case _OA_FORM_VIEW_VACUUM: //1
				if(xml.FindChildElem(TEXT("VacState")))
				{
					xml.IntoElem();
					for(int j=0;j<_ITEM2_VAC_STATE_CNT;j++)
					{//必须依次查找和填写
						_stprintf(g_strBuffer,TEXT("state%d"),j+1);
						g_item2_Vac_state[j] = xml.GetAttrib(g_strBuffer);										
					}
					xml.OutOfElem();
				}
				
				if(xml.FindChildElem(TEXT("constID")))
				{
					xml.IntoElem();
					for(int j=0;j<gc_item2_Text_ID_CNT;j++)
					{//必须依次查找和填写
						if(xml.FindChildElem(g_item2_ID_Name[j]))
						{		
							g_item2_ID_Text[j] = xml.GetChildData();										
						}
					}
					xml.OutOfElem();
				}

				if(xml.FindChildElem(TEXT("otherString")))
				{
					xml.IntoElem();
					int iStrCnt = __min(_ITEM2_OTHER_STR_NUMBER,_ttoi(xml.GetAttrib(TEXT("count"))));
					for(int i = 0;i<iStrCnt;i++)
					{
						_stprintf(g_strBuffer,TEXT("str%d"),i+1);
						if(xml.FindChildElem(g_strBuffer))
						{		
							g_item2_String[i] = xml.GetChildData();								
						}			
					}
					xml.OutOfElem();
				}
				break;
			case _OA_FORM_VIEW_ROTATOR:
				if(xml.FindChildElem(TEXT("constID")))
				{
					xml.IntoElem();
					for(int j=0;j<gc_item3_Text_ID_CNT;j++)
					{//必须依次查找和填写
						if(xml.FindChildElem(g_item3_ID_Name[j]))
						{		
							g_item3_ID_Text[j] = xml.GetChildData();										
						}
					}
					xml.OutOfElem();
				}
				if(xml.FindChildElem(TEXT("otherString")))
				{
					xml.IntoElem();
					int iStrCnt = __min(_ITEM2_OTHER_STR_NUMBER,_ttoi(xml.GetAttrib(TEXT("count"))));
					for(int i = 0;i<iStrCnt;i++)
					{
						_stprintf(g_strBuffer,TEXT("str%d"),i+1);
						if(xml.FindChildElem(g_strBuffer))
						{		
							g_item3_String[i] = xml.GetChildData();								
						}			
					}
					xml.OutOfElem();
				}
				break;
			}
			xml.OutOfElem();
		}
	}	

	return true;
}

bool COACnfgDoc::fn_WriteCurLngInt(int iCurLng)
{
	if(iCurLng>m_iLngCnt)
		return false;
	CWinApp* pApp = AfxGetApp();
	if(NULL==pApp)
		return false;

	m_iCurLng = iCurLng;
	pApp->WriteProfileInt(TEXT("LANGUAGE"),TEXT("LNG_CURRENT"),iCurLng);

	return false;
}

LPCTSTR COACnfgDoc::fn_GetLocalTimeStr(void)
{
	SYSTEMTIME   st;   
	GetLocalTime(&st);   
	_stprintf(g_strTimeBuffer,TEXT("%4d-%02d-%02d %02d:%02d:%02d"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	return (LPCTSTR)g_strTimeBuffer;
}

//ch3 penning
double COACnfgDoc::fn_TransPenningVoltPa(double dVolt)
{
	//3400，使用 /4000，而不是原来的 /10000.0，20071009
	//return pow(10.0, dVolt * 7.0 / 4000.0 -7.0 ); //2008.02.18
	if(g_IonGauge.GetGaugeType() == 5 || g_IonGauge.GetGaugeType()==3) //VGC403 && MPG400  //2008.02.18
	{ //MPG400 . ==3 ,G1270的真空计配置与S565/S566一样，而 S565 DM9085=5, G1270 DM9085=3 .08.03.03		
		return pow( 10, dVolt / 400 * 11 / 10 - 6); // -8 mbar , 1mbar = 100 Pa,所以直接 -6
	}
	else
	{
		return pow( 10.0, dVolt * 7.0 / 4000.0 - 7.0 );
	}

	//p = 10^( (U - 7.75) / 0.75 + c ) ;// c = 2 , pa
}

void COACnfgDoc::fn_CreateCommError(_sPlcCommError& sCommErr)
{
	sCommErr.bObjectErr = ((g_pVacProThread->m_dwLastError == _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT)?true:false);
	if(!sCommErr.bObjectErr)
	{
		sCommErr.dwPlcLastError = g_plc.m_dwLastError;
		if(sCommErr.dwPlcLastError == _SYNCCOM_ERROR_SYS_API_)
		{
			sCommErr.dwPlcLastErrorEx = g_plc.m_dwLastSysApiError;
		}
		else if(sCommErr.dwPlcLastError == _SYSMAC_PLC_ERROR_END_CODE_)
		{
			sCommErr.dwPlcLastErrorEx = (DWORD)g_plc.m_iLastErrEndCode;
		}
	}
}

/***************************************************************************************************
函数名称：fn_LinearRegression
参数：
      [out] double& a;  线性回归得到的 斜率
	  [out] double& b;  线性回归得到的 截距
	  [in]  double* x;  输入参数 x 数组
	  [in]  double* y;  输入参数 y 数组
	  [in]  int n;      输入参数 x,y 数组的尺寸
说明：求线性回归方程：Y = a + bx
*****************************************************************************************************/
int COACnfgDoc::fn_LinearRegression(double& a, double& b, double* x, double* y, int n=0)
{
    int m;
    double Lxx = 0.0, Lxy = 0.0, xa = 0.0, ya = 0.0;
    if (x == 0 || y == 0 || n < 1)
        return -1;
    for (m = 0; m < n; m ++)
    {
        xa += x[m];
        ya += y[m];
    }
    xa /= n;                                     // X平均值
    ya /= n;                                     // Y平均值
    for (m = 0; m < n; m ++)
    {
        Lxx += ((x[m] - xa) * (x[m] - xa));             // Lxx = Sum((X - Xa)平方)
        Lxy += ((x[m] - xa) * (y[m] - ya));       // Lxy = Sum((X - Xa)(Y - Ya))
    }
	
    b = Lxy / Lxx;                                 // b = Lxy / Lxx
    a = ya - b * xa;                              // a = Ya - b*Xa
	return 1;
}
/*
// 求线性回归方程：Y = a + bx
// dada[rows*2]数组：X, Y；rows：数据行数；a, b：返回回归系数
// SquarePoor[4]：返回方差分析指标: 回归平方和，剩余平方和，回归平方差，剩余平方差
// 返回值：0求解成功，-1错误
int LinearRegression(double *data, int rows, double *a, double *b, double *SquarePoor)
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
*/