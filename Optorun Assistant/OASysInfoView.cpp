// OASysInfoView.cpp : implementation of the COASysInfoView class
//

#include "stdafx.h"
#include "Optorun Assistant.h"

#include "Optorun AssistantDoc.h"
#include "OASysInfoView.h"

#include "MainFrm.h"
//class CMainFrame;
#include "OACnfgDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern COACnfgDoc g_COACnfgDoc;

char pCom5TestString[1024];
char pComTestStringForGL[1024]; //GL 或 IBS的发送字符串.
char pComTestStringForGasCont[1024];

TCHAR g_buffer[1024];
char  g_chBuffer[1024];
TCHAR g_tchBuffer2[1024];

//#define WM_ITEM1_ADD_RUN_HISTORY (WM_USER+701)

// COASysInfoView


IMPLEMENT_DYNCREATE(COASysInfoView, CFormView)

BEGIN_MESSAGE_MAP(COASysInfoView, CFormView)
	ON_MESSAGE(WM_ITEM1_DISPLAY_PLC_STATE,OnDisplayPlcState)
	ON_MESSAGE(WM_ITEM1_ADD_RUN_HISTORY,OnAddRunHistroy)
	ON_MESSAGE(WM_ITEM1_GET_NAMES,OnGetNames)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_HELP_LEFT, &COASysInfoView::OnBnClickedButtonHelpLeft)
	ON_BN_CLICKED(IDC_BUTTON_HELP_RIGHT, &COASysInfoView::OnBnClickedButtonHelpRight)
	ON_WM_TIMER()
//	ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_CLOSE_CONNECTION, &COASysInfoView::OnBnClickedButtonCloseConnection)
ON_BN_CLICKED(IDC_BUTTON_CONNECT_PLC, &COASysInfoView::OnBnClickedButtonConnectPlc)
//ON_BN_CLICKED(IDC_STATIC_, &COASysInfoView::OnBnClickedStatic)
//ON_BN_CLICKED(IDC_STATIC_SERIAL_PORT_TEST, &COASysInfoView::OnBnClickedStaticTest)
ON_BN_CLICKED(IDC_BUTTON_COM2_OPEN, &COASysInfoView::OnBnClickedButtonCom2Open)
ON_BN_CLICKED(IDC_BUTTON_COM2_CLOSE, &COASysInfoView::OnBnClickedButtonCom2Close)
ON_BN_CLICKED(IDC_BUTTON_COM2_TEST, &COASysInfoView::OnBnClickedButtonCom2Test)
ON_BN_CLICKED(IDC_BUTTON_COM2_PROPERTY, &COASysInfoView::OnBnClickedButtonCom2Property)
ON_BN_CLICKED(IDC_BUTTON_COM3_OPEN, &COASysInfoView::OnBnClickedButtonCom3Open)
ON_BN_CLICKED(IDC_BUTTON_COM3_CLOSE, &COASysInfoView::OnBnClickedButtonCom3Close)
ON_BN_CLICKED(IDC_BUTTON_COM3_TEST, &COASysInfoView::OnBnClickedButtonCom3Test)
ON_BN_CLICKED(IDC_BUTTON_COM3_PROPERTY, &COASysInfoView::OnBnClickedButtonCom3Property)
ON_BN_CLICKED(IDC_BUTTON_COM4_OPEN, &COASysInfoView::OnBnClickedButtonCom4Open)
ON_BN_CLICKED(IDC_BUTTON_COM4_CLOSE, &COASysInfoView::OnBnClickedButtonCom4Close)
ON_BN_CLICKED(IDC_BUTTON_COM4_TEST, &COASysInfoView::OnBnClickedButtonCom4Test)
ON_BN_CLICKED(IDC_BUTTON_COM4_PROPERTY, &COASysInfoView::OnBnClickedButtonCom4Property)
ON_BN_CLICKED(IDC_BUTTON_COM5_OPEN, &COASysInfoView::OnBnClickedButtonCom5Open)
ON_BN_CLICKED(IDC_BUTTON_COM5_CLOSE, &COASysInfoView::OnBnClickedButtonCom5Close)
ON_BN_CLICKED(IDC_BUTTON_COM5_TEST, &COASysInfoView::OnBnClickedButtonCom5Test)
ON_BN_CLICKED(IDC_BUTTON_COM5_PROPERTY, &COASysInfoView::OnBnClickedButtonCom5Property)
//ON_BN_DOUBLECLICKED(IDC_STATIC_SERIAL_PORT_TEST, &COASysInfoView::OnBnDoubleclickedStaticSerialPortTest)
//ON_BN_CLICKED(IDC_STATIC_SERIAL_PORT_TEST, &COASysInfoView::OnBnClickedStaticSerialPortTest)
ON_STN_CLICKED(IDC_STATIC_COM2_TEXT, &COASysInfoView::OnStnClickedStaticCom2Text)
//ON_BN_CLICKED(IDC_STATIC_SERIAL_PORT_TEST, &COASysInfoView::OnBnClickedStaticSerialPortTest)
ON_STN_CLICKED(IDC_STATIC_COM3_TEXT, &COASysInfoView::OnStnClickedStaticCom3Text)
ON_STN_CLICKED(IDC_STATIC_COM4_TEXT, &COASysInfoView::OnStnClickedStaticCom4Text)
ON_STN_CLICKED(IDC_STATIC_COM5_TEXT, &COASysInfoView::OnStnClickedStaticCom5Text)
ON_BN_CLICKED(IDC_CHECK_RECORD_VACUUM, &COASysInfoView::OnBnClickedCheckRecordVacuum)
//ON_WM_ERASEBKGND()
ON_STN_CLICKED(IDC_STATIC_HELP_TITLE, &COASysInfoView::OnStnClickedStaticHelpTitle)
END_MESSAGE_MAP()

//FindCommPort，查找注册表中COM口设置，将其填充进 comboBox中
void FindCommPort( CComboBox *pComboBox ) 
{ 
	HKEY hKey;
#ifdef _DEBUG 
	ASSERT( pComboBox != NULL );
	pComboBox->AssertValid();
	pComboBox->ResetContent(); //清除现存内容
#endif
	//打开注册表中的串口信息
	if( ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,TEXT("Hardware\\DeviceMap\\SerialComm"), NULL, KEY_READ, &hKey) == ERROR_SUCCESS) 
	{ 
		int i=0;
		TCHAR portName[256],commName[256];
		DWORD dwLong,dwSize;
		while(1)
		{
			dwLong = dwSize = sizeof(portName)/sizeof(TCHAR);
			//枚举串口并添加进 pComboBox中
			if( ::RegEnumValue( hKey, i, portName, &dwLong, NULL, NULL, (PUCHAR)commName, &dwSize ) == ERROR_NO_MORE_ITEMS ) //
				break;
			pComboBox->AddString( commName );// commName
			i++;
		}
		if( pComboBox->GetCount() == 0 )
		{
			::AfxMessageBox( TEXT("Cannot Find any available COM port!") );
		}
		RegCloseKey(hKey);
	}
}


// COASysInfoView construction/destruction
COASysInfoView::COASysInfoView()
	: CFormView(COASysInfoView::IDD)
{
	// TODO: add construction code here
	m_uiHelpNumber = 0;
	m_unSysOneSecondTimer = 1;

//	gb_IonGauge = false;

	//ASSERT(0);
}

COASysInfoView::~COASysInfoView()
{

}

void COASysInfoView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_HISTORY, m_editHistory);
	DDX_Control(pDX, IDC_EDIT_HELP_TEXT, m_editHelp);
	DDX_Control(pDX, IDC_STATIC_PLC_STATE, m_staticPLCstate);
	DDX_Control(pDX, IDC_STATIC_HELP_TITLE, m_staticHelpTilte);
}

BOOL COASysInfoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void COASysInfoView::OnInitialUpdate()
{
	/*
	double y = 1923;
	double x = g_COACnfgDoc.fn_TransPenningVoltPa(10*y);
	x = g_COACnfgDoc.fn_TransPenningVoltPa(y);
	char*  p = "4f54";
	//TCHAR* x = L"OT";
	*/

	COptorunAssistantDoc* pd =(COptorunAssistantDoc*) ((CMainFrame*)AfxGetApp()->m_pMainWnd)->GetActiveDocument();
	
	CFormView::OnInitialUpdate();

	((CMainFrame*)AfxGetApp()->m_pMainWnd)->AddView(g_COACnfgDoc.m_sTabLabel[_OA_FORM_VIEW_SYSTEM],this,g_COACnfgDoc.m_sTabTooltip[_OA_FORM_VIEW_SYSTEM]);
	((CMainFrame*)AfxGetApp()->m_pMainWnd)->SetCurView(_OA_FORM_VIEW_SYSTEM);

	//从xml读取到文本，此处设置页面ID的文本
	fn_SetIDsText();

	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	m_editHistory.bkColor( RGB(255,255,255) );
	m_editHelp.bkColor( RGB(255,255,225) );
	m_staticHelpTilte.textColor(RGB(0,73,214));

	m_editHelp.SetWindowText(g_item1_Help_Text[0]);




	EnterCriticalSection(&gCriticalSection_State);
		g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_NOT_CONNECT;
	LeaveCriticalSection(&gCriticalSection_State);

	


	fn_DisplayPlcState();
	fn_ShowWindow();

	DWORD bufsize = 512;
	::GetComputerName(g_strBuffer,&bufsize);
	SetDlgItemText(IDC_STATIC_PC_NAME_AREA,g_strBuffer);

	//运行记录中添加起始时间
	SYSTEMTIME   st;   
	GetLocalTime(&st);   
	_stprintf(g_strBuffer,TEXT("%4d-%02d-%02d %02d:%02d:%02d  Optorun Assistant start up\r\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	fn_AppendHistoryText(g_strBuffer);

	//开启定时器
	m_unSysOneSecondTimer = SetTimer(m_unSysOneSecondTimer,1000,NULL);

	UINT uLimit = m_editHistory.GetLimitText();
	//debug
#ifdef _DEBUG
	//SendMessage(WM_ITEM1_ADD_RUN_HISTORY,0,(LPARAM)TEXT("\r\nFrom send message\r\n"));
	//PostMessage(WM_ITEM1_ADD_RUN_HISTORY,0,(LPARAM)TEXT("\r\nFrom post message\r\n"));
	//fn_AppendHistoryText(TEXT("\r\nHere test "));

#endif
	::SetFocus(GetDlgItem(IDC_BUTTON_CONNECT_PLC)->GetSafeHwnd());

	SetDlgItemText(IDC_STATIC_PRODUCT_VERSION_AREA,g_strProductVersion);
	SetDlgItemText(IDC_STATIC_MACHINE_TYPE_AREA,TEXT(" "));
	SetDlgItemText(IDC_STATIC_MACHINE_SN_AREA,TEXT(" "));
	SetDlgItemText(IDC_STATIC_TOUCHPANEL_VER_AREA,TEXT(" "));

	//1百万行限制！
	m_editHistory.SetLimitText(100*1000000);
	/*
	for(int i = 0; i<100000; i++)
	{
		_stprintf(g_strBuffer,TEXT("% 5d: 0123456789012345678901234567890123456789012345678901234567890123456789\r\n"),i);
		fn_AppendHistoryText(g_strBuffer);
	}*/

	//添加测试串口的代码
	for(int i=IDC_COMBO_COM2;i<=IDC_COMBO_COM5;i++)
	{
		CComboBox* pCom = (CComboBox*)GetDlgItem(i);
		FindCommPort(pCom);
	}
	//property icon
	for(int i = IDC_BUTTON_COM2_PROPERTY ;i<=IDC_BUTTON_COM5_PROPERTY ;i++)
	{
		((CButton*)GetDlgItem(i))->SetIcon( AfxGetApp()->LoadIcon(IDI_ICON_PROPERTY) ); 
	}

	memcpy(	pCom5TestString, "VER?\r\n\0", sizeof("VER?\r\n\0"));	
	memcpy(	pComTestStringForGL, "IBSQID\x0D", sizeof("IBSQID\x0D")); //2010.06.10
	memcpy(	pComTestStringForGasCont, "RS232C\x0D", sizeof("RS232C\x0D")); //2010.06.10
	
}

// COASysInfoView diagnostics
#ifdef _DEBUG
void COASysInfoView::AssertValid() const
{
	CFormView::AssertValid();
}

void COASysInfoView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

COptorunAssistantDoc* COASysInfoView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COptorunAssistantDoc)));
	return (COptorunAssistantDoc*)m_pDocument;
}
#endif //_DEBUG


// 设定页面文本，这些文本是从 xml文件中读取来的

bool COASysInfoView::fn_SetIDsText(void)
{
	//SetDlgItemText(1001,g_item1_ID_Text[0]);	
	//g_item1_ID_Text[0] = TEXT("Write");
	for(int i=0;i<_ITEM1_NEED_TEXT_ID_CNT;i++)
	{
		//SetDlgItemText(g_item1_ID[i],g_item1_ID_Text[i]);
		if(!g_item1_ID_Text[i].IsEmpty())
			SetDlgItemText(g_item1_ID[i],g_item1_ID_Text[i]);
	}
	return false;
}

void COASysInfoView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if(::IsWindow(GetSafeHwnd()))
	{//
		CWnd* pwnd = GetDlgItem(IDC_ITEM1_SYS_GROUP);
		if(::IsWindow(pwnd->GetSafeHwnd()))
		{
			//取得sys group 坐标
			CRect rt1,rt2,rt3,rt4,rect;
			pwnd->GetWindowRect(&rt1); //screen 
			ScreenToClient(&rt1);      //转化到本页面坐标

			GetClientRect(&rect);      //本页面坐标

			//-----设置 run History 编辑栏位置
			//如果 sys group坐标下沿超过 本页面范围的 2/3，则从 其下沿开始，否则从 页面的下 1/3处开始。
			rt2 = rect;
			rt2.top     = 2*rect.Height()/3 > rt1.bottom ? 2*rect.Height()/3 : rt1.bottom+20;
			rt2.left   += 2;
			rt2.right  -= 2;
			rt2.bottom -= 2;

			//页面全宽度
			//Move history group window
			pwnd = GetDlgItem(IDC_STATIC_HISTORY);
			pwnd->MoveWindow(&rt2); //MoveWindow相对于父窗口(本页面)坐标

			//move History edit 
			rt2.top +=20;
			pwnd = GetDlgItem(IDC_EDIT_HISTORY);
			pwnd->MoveWindow(&rt2);

			//-----设置 Help 编辑栏位置
			//IDC_BUTTON_HELP_LEFT IDC_EDIT_HELP_TEXT IDC_STATIC_HELP_TITLE IDC_BUTTON_HELP_RIGHT
			GetClientRect(&rect);      //再次取得本页面坐标
		    pwnd = GetDlgItem(IDC_BUTTON_HELP_RIGHT);
			pwnd->GetWindowRect(&rt2);
			ScreenToClient(&rt2);
			rt3.top    = rt1.top;
			rt3.bottom = rt3.top + rt2.Height();
			rt3.right  = rect.Width() -2 ;
			rt3.left   = rt3.right - rt2.Width();
			pwnd->MoveWindow(&rt3);

			pwnd = GetDlgItem(IDC_BUTTON_HELP_LEFT);
			rt2.top = rt3.top ;
			rt2.bottom = rt3.bottom;
			rt2.left = rect.Width() > 2 * rt1.Width() ? rect.right - 4 -rt1.Width() : rt1.right +2;
			rt2.right = rt2.left + rt3.Width();
			pwnd->MoveWindow(&rt2);

			pwnd  = GetDlgItem(IDC_STATIC_HELP_TITLE);
			rt4.top = rt2.top;
			rt4.bottom = rt2.bottom;
			rt4.left = rt2.right + 2;
			rt4.right = rt3.left -2;
			pwnd->MoveWindow(&rt4);
			
			pwnd  = GetDlgItem(IDC_EDIT_HELP_TEXT);
			rt4.left = rt2.left;
			rt4.right = rt3.right;
			rt4.top =   rt2.bottom + 2;
			rt4.bottom = rt1.bottom - 1;
			pwnd->MoveWindow(&rt4);
		}
	}
	// TODO: Add your message handler code here
}

//向历史编辑框内添加文本行
void COASysInfoView::fn_AppendHistoryText(LPCTSTR lpText)
{
	CEdit* pwnd = (CEdit*)GetDlgItem(IDC_EDIT_HISTORY);
	if(pwnd->GetSafeHwnd())
	{
		int nLen=pwnd->GetWindowTextLength (); 
		pwnd->SetSel (nLen, nLen); 
		pwnd->ReplaceSel (lpText);
	}
}

void COASysInfoView::OnBnClickedButtonHelpLeft()
{
	// TODO: Add your control notification handler code here
	m_uiHelpNumber--;

	if( m_uiHelpNumber < 0 )
	{
		//防止 giHelpItems = 0的情况;
		if(giHelpItems)
			m_uiHelpNumber = giHelpItems - 1;		
		else
			m_uiHelpNumber = 0;		
	}

	if(m_uiHelpNumber>=giHelpItems)
	{
		m_uiHelpNumber = 0;
	}

	m_editHelp.SetWindowText(g_item1_Help_Text[m_uiHelpNumber]);
}

void COASysInfoView::OnBnClickedButtonHelpRight()
{
	// TODO: Add your control notification handler code here
	m_uiHelpNumber++;
	if(m_uiHelpNumber>=giHelpItems)
	{
		m_uiHelpNumber = 0;
	}
	if( m_uiHelpNumber < 0 )
	{
		m_uiHelpNumber = 0;
	}
	m_editHelp.SetWindowText(g_item1_Help_Text[m_uiHelpNumber]);
}

LRESULT COASysInfoView::OnDisplayPlcState(WPARAM wParam, LPARAM lParam)
{
	fn_DisplayPlcState();
	return 0;
}


CString COASysInfoView::GetDMstrEvent(int nBeginWord, int nReadNumber)
{
	CString str;
	DWORD dw = WaitForSingleObject(g_hEvent_plc,500);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
#ifdef _DEBUG
		fn_AppendHistoryText(TEXT("Error Connect PLC: OnGetNames->WaitForSingleObject(g_hEvent_plc,500) ,time out;\r\n"));
		return str;
#endif //_DEBUG
	}
	
#ifdef _ZHANGZIYE_DEBUG	
	DWORD dwStart = GetTickCount();
#endif// _ZHANGZIYE_DEBUG	

	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	str = g_plc.GetDMstr(nBeginWord, nReadNumber);
	SetEvent(g_hEvent_plc);  //完成后再

#ifdef _ZHANGZIYE_DEBUG	
	_stprintf(g_strBuffer,TEXT("GetDMstrEvent: %d ms\r\n"),GetTickCount()-dwStart);	
	fn_AppendHistoryText(g_strBuffer);
#endif// _ZHANGZIYE_DEBUG	
	return str;
}

//消息响应函数
LRESULT COASysInfoView::OnGetNames(WPARAM wParam, LPARAM lParam)
{
	HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));

	_stprintf(g_strBuffer,TEXT("%s  Error: OnGetNames Error, dwLastError = %d \r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.GetLastError());
	CString str = GetDMstrEvent(gc_DM_machinetype,10);	
	if(0==str.GetLength())
	{
		_stprintf(g_strBuffer,TEXT("%s  Error: Get machine type Error, dwLastError = %d \r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.GetLastError());
		goto end_OnGetNames_Error;
	}	
	SetDlgItemText(IDC_STATIC_MACHINE_TYPE_AREA,str);

	str = GetDMstrEvent(gc_DM_sequencer,10);
	if( 0==str.GetLength())
	{
		_stprintf(g_strBuffer,TEXT("%s  Error: Get sequencer version Error, dwLastError = %d \r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.GetLastError());
		goto end_OnGetNames_Error;
	}
	SetDlgItemText(IDC_STATIC_MACHINE_SN_AREA,str);

	str = GetDMstrEvent(gc_DM_touchpanel,10);
	if( 0==str.GetLength())
	{
		_stprintf(g_strBuffer,TEXT("%s  Error: Get Touch Panel version Error, dwLastError = %d \r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.GetLastError());
		goto end_OnGetNames_Error;
	}
	SetDlgItemText(IDC_STATIC_TOUCHPANEL_VER_AREA,str);
    

	//2008.05.12添加
	WORD wRdWordAPCGauge = 0;
	g_plc.ReadPLCex(enReadDM,gc_DM_APCGaugeType,1,&wRdWordAPCGauge,true);
	g_APCGauge.SetGaugeType(wRdWordAPCGauge);
	gc_dMaxPenningPa = g_APCGauge.TransVal2Pascal(4199-1);
	gc_dMinPenningPa = g_APCGauge.TransVal2Pascal(176+1);

	WORD wRdWordIonGauge = 0;
	//设定离子规种类.如果有的话，wRdWord!=0，否则=0.
	if( g_plc.ReadPLCex(enReadDM,gc_DM_IonGaugeType,1,&wRdWordIonGauge,true) )
	{
		//g_IonGauge.SetGaugeType((int)wRdWordIonGauge); 
		/*
		bool bIonGaugeExsit;
		//2008.05.12, 9088:03 && 9085:05，对应 VGC403 Ch2 为psd，就是没有电离真空规//本句错误G1275
		if( 0 == wRdWordIonGauge || ( 3 == (int)wRdWordAPCGauge && 5 == (int)wRdWordIonGauge) )
		{
			bIonGaugeExsit = false;
		}
		else 
			bIonGaugeExsit = true;
		g_IonGauge.SetGaugeTypeEx((int)wRdWordIonGauge,bIonGaugeExsit);
		*/

		g_IonGauge.SetGaugeTypeEx((int)wRdWordIonGauge, wRdWordIonGauge!=0);	

#ifdef _ZHANGZIYE_DEBUG
		_stprintf(g_strBuffer,TEXT("Ion Gauge type = %s\r\n"),g_IonGauge.GetGaugeTypeName());
		fn_AppendHistoryText(g_strBuffer);
#endif
	}



	//temp 地址设置
	if(g_plc.ReadPLCex(enReadDM,gc_DM_AddressHeaterTempSVPV,_TEMP_HEATER_NUMBER-2,gcp_DM_HeaterTemp,true))
	{//_TEMP_HEATER_NUMBER-2是因为，侧壁地址指针位不连续
		_stprintf(g_strBuffer,TEXT("Address of MonitorHeater PV =%d, Dome PV=%d, Monitor SV=%d, Dome SV=%d\r\n"),gcp_DM_HeaterTemp[0],gcp_DM_HeaterTemp[1],gcp_DM_HeaterTemp[2],gcp_DM_HeaterTemp[3]);
		fn_AppendHistoryText(g_strBuffer);
		//添加侧壁地址
		if(g_plc.ReadPLCex(enReadDM,gc_DM_AddressDome2TempSVPV,2,&gcp_DM_HeaterTemp[_TEMP_HEATER_NUMBER-2],true))
		{
			_stprintf(g_strBuffer,TEXT("Address of DomeHeater2 PV =%d, DomeHeater2 SV=%d\r\n"),gcp_DM_HeaterTemp[4],gcp_DM_HeaterTemp[5]);
			fn_AppendHistoryText(g_strBuffer);
		}
	}
	

	/*
#ifdef _ZHANGZIYE_DEBUG
	g_plc.ReadPLC(enReadDM,gc_DM_PenningValue,1,&wRdWord);

	_stprintf(g_strBuffer,TEXT("Penning gauge Volt = %d\r\n"),(int)wRdWord);
	fn_AppendHistoryText(g_strBuffer);
	
	if(g_IonGauge.GetGaugeType())
	{
		g_plc.ReadPLC(enReadDM,gc_DM_IonGaugeValue,1,&wRdWord);
		_stprintf(g_strBuffer,TEXT("Ion Gauge Volt = %d\r\n"),(int)wRdWord);
		fn_AppendHistoryText(g_strBuffer);
	}
#endif 
	*/
	//debug end
		
	EnterCriticalSection(&gCriticalSection_State);
	g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_COMM_NORMAL;
	LeaveCriticalSection(&gCriticalSection_State);

	goto end_OnGetNames;

end_OnGetNames_Error:
	fn_AppendHistoryText(g_strBuffer);
	if( 0 != g_plc.m_dwBytesRead )
	{
		//g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_COMM_CONTENT_ERROR;
		EnterCriticalSection(&gCriticalSection_State);
		g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_COMM_CONTENT_ERROR;
		LeaveCriticalSection(&gCriticalSection_State);
	}
	else
	{
		//g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_COMM_NORESPOND;
		EnterCriticalSection(&gCriticalSection_State);
		g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_COMM_NORESPOND;
		LeaveCriticalSection(&gCriticalSection_State);
	}
end_OnGetNames:
	fn_DisplayPlcState();

	SetCursor(hcur);
	return 0;
}

LRESULT COASysInfoView::OnAddRunHistroy(WPARAM wParam, LPARAM lParam)
{
	fn_AppendHistoryText( (LPCTSTR) lParam);
	return 0;
}

void COASysInfoView::fn_DisplayPlcState(void)
{
	EnterCriticalSection(&gCriticalSection_State);
		int iPlcState = g_COACnfgDoc.m_iPlcState;
	LeaveCriticalSection(&gCriticalSection_State);
	switch(iPlcState)
	{
	case _COM_PLC_STATE_NOT_CONNECT:
		m_staticPLCstate.textColor(gc_clrBlack);
		break;
	case _COM_PLC_STATE_TRY_COMM:
		m_staticPLCstate.textColor(gc_clrTrying);
		break;
	case _COM_PLC_STATE_COMM_NORMAL:
		m_staticPLCstate.textColor(gc_clrOK);
		break;
	case _COM_PLC_STATE_OPEN_COM_ERROR:
	case _COM_PLC_STATE_COMM_CONTENT_ERROR:
	case _COM_PLC_STATE_COMM_NORESPOND:
		m_staticPLCstate.textColor(gc_clrAlert);
		break;
	}
	m_staticPLCstate.ShowWindow(SW_SHOW);
	m_staticPLCstate.SetWindowText(g_item1_plc_state[iPlcState]);

	static int iLastState = 0xFFFF;
	if(iLastState != iPlcState)
	{
		iLastState = iPlcState;
		AfxGetMainWnd()->PostMessage(MAIN_FRM_ITEMx_2_ITEMy,MAKEWPARAM(1,0),WM_ITEM2_DISPLAY_VAC_STATE);
	}

}

void COASysInfoView::fn_ShowWindow(void)
{

	EnterCriticalSection(&gCriticalSection_State);
		int iPlcState = g_COACnfgDoc.m_iPlcState;
	LeaveCriticalSection(&gCriticalSection_State);
	

	bool b = ( _COM_PLC_STATE_NOT_CONNECT == iPlcState || _COM_PLC_STATE_OPEN_COM_ERROR==iPlcState);
	GetDlgItem(IDC_BUTTON_CONNECT_PLC)->EnableWindow( b);
	GetDlgItem(IDC_BUTTON_CLOSE_CONNECTION)->EnableWindow(!b);
}

//次函数每秒钟调用一次
void COASysInfoView::OnTimer(UINT_PTR nIDEvent)  
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == m_unSysOneSecondTimer)
	{

	EnterCriticalSection(&gCriticalSection_State);
		int iPlcState = g_COACnfgDoc.m_iPlcState;
	LeaveCriticalSection(&gCriticalSection_State);

		if(_COM_PLC_STATE_COMM_NORESPOND==iPlcState  || _COM_PLC_STATE_COMM_CONTENT_ERROR ==iPlcState  || _COM_PLC_STATE_OPEN_COM_ERROR==iPlcState )
		{
			static BOOL bFlickerOn = true;
			bFlickerOn = !bFlickerOn;
			m_staticPLCstate.ShowWindow(bFlickerOn ? SW_SHOW:SW_HIDE); 			
			if(_COM_PLC_STATE_OPEN_COM_ERROR != iPlcState)
			{ //内容错或没有响应，则周期性重试
				static unsigned int uTest = 0;
				uTest++;
				if(uTest>3)   
				{
					uTest = 0;
					PostMessage(WM_ITEM1_GET_NAMES,0,0);//每隔3秒再尝试一次
				}
			}
		}

/*

*/
	}
	CFormView::OnTimer(nIDEvent);
}

void COASysInfoView::OnDestroy()
{
	if(m_unSysOneSecondTimer)
	{
		KillTimer(m_unSysOneSecondTimer);
	}
	CFormView::OnDestroy();

	// TODO: Add your message handler code here
}

void COASysInfoView::OnBnClickedButtonConnectPlc()
{
	// TODO: Add your control notification handler code here
//	g_IonGauge.SetGaugeType(1); 2010.05.11 test
//	double x = g_IonGauge.TransStr2Pascal("870");
	g_APCGauge.SetGaugeType(0);
	double x = g_APCGauge.TransVal2Pascal(2000);
	g_APCGauge.SetGaugeType(1);
	x = g_APCGauge.TransVal2Pascal(2000);
	g_APCGauge.SetGaugeType(2);
	x = g_APCGauge.TransVal2Pascal(2000);
	g_APCGauge.SetGaugeType(3);
	x = g_APCGauge.TransVal2Pascal(2000);


	//DEBUGGING	
	//((CButton*)GetDlgItem(IDC_BUTTON_COM2_PROPERTY))->SetIcon( AfxGetApp()->LoadIcon(IDI_ICON_PROPERTY) ); 

	//::g_pVacProThread->PostThreadMessageW(WM_EXIT,0,0);
	//TerminateThread (g_pVacProThread->m_hThread,0);
//	g_pVacProThread->SuspendThread();
    HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	DWORD dw = WaitForSingleObject(g_hEvent_plc,1000);
	if( WAIT_TIMEOUT == dw || WAIT_FAILED == dw )
	{
		AfxMessageBox(TEXT("Error:Debug, WaitForSingleObject(g_hEvent_plc,1000);"));
		fn_AppendHistoryText(TEXT("Error Connect PLC: WaitForSingleObject(g_hEvent_plc,1000) ,time out;\r\n"));
	}
	else
	{
		ResetEvent(g_hEvent_plc); //阻塞其他线程调用plc相关函数
		bool b = g_plc.OpenCom();
		SetEvent(g_hEvent_plc);   //先释放plc
		if( b )
		{
			//记录
			_stprintf(g_strBuffer,TEXT("%s  %s open,\r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.m_strCom);
			fn_AppendHistoryText(g_strBuffer);
			//设定plc状态


			EnterCriticalSection(&gCriticalSection_State);
			g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_TRY_COMM;
			LeaveCriticalSection(&gCriticalSection_State);


			PostMessage(WM_ITEM1_GET_NAMES,0,0);  //第一次调用消息处理函数，无需等待3s,直接调用。之后用WM_TIMER每次隔三秒调用
		}
		else
		{
			//记录			
			_stprintf(g_strBuffer,TEXT("%s  Error: cannot open %s, dwLastError = %d \r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.m_strCom,g_plc.GetLastError());
			fn_AppendHistoryText(g_strBuffer);

			EnterCriticalSection(&gCriticalSection_State);
			g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_OPEN_COM_ERROR;
			LeaveCriticalSection(&gCriticalSection_State);
		}
		fn_DisplayPlcState();
		fn_ShowWindow();

#ifdef _ZHANGZIYE_DEBUG	
	//g_plc.SetSleepTime(0);	
	DWORD dwStart = GetTickCount();
	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	CString str = g_plc.GetDMstr(9000, 1);
	SetEvent(g_hEvent_plc);  //完成后再

	_stprintf(g_strBuffer,TEXT("g_plc.GetDMstr(9000, 1) %s: %d ms\r\n"),str,GetTickCount()-dwStart);	
	fn_AppendHistoryText(g_strBuffer);
#endif// _ZHANGZIYE_DEBUG

	}
	SetCursor(hcur);
}

void COASysInfoView::OnBnClickedButtonCloseConnection()
{
	// TODO: Add your control notification handler code here
	//debug, 需添加 检查 是否正在 测试...

	bool bVacuumTest;
	EnterCriticalSection(&gCriSec_Vacuum);
			//memcpy(&sVacuumState,&gi_stVacuumTestState,sizeof(stVacuumTestState));
	bVacuumTest = gi_stVacuumTestState.bTest;
	LeaveCriticalSection(&gCriSec_Vacuum);

	if(bVacuumTest || g_bTestHearthRotation)
	{
		AfxMessageBox(g_strSystem[_OA_STR_SYSTEM_ERROR_TEST],MB_OK|MB_ICONSTOP);
		return;
	}

	HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
	DWORD dw = WaitForSingleObject(g_hEvent_plc,1000);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
		AfxMessageBox(TEXT("Error:WaitForSingleObject(g_hEvent_plc,1000);"));
		fn_AppendHistoryText(TEXT("Error close connect PLC: WaitForSingleObject(g_hEvent_plc,1000) ,time out;\r\n"));
	}
	else
	{
		g_plc.CloseCom();
		SetEvent(g_hEvent_plc);//先释放plc

		_stprintf(g_strBuffer,TEXT("%s  %s closed\r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.m_strCom);
		fn_AppendHistoryText(g_strBuffer);

		//设定plc状态
			EnterCriticalSection(&gCriticalSection_State);
			g_COACnfgDoc.m_iPlcState = _COM_PLC_STATE_NOT_CONNECT;
			LeaveCriticalSection(&gCriticalSection_State);

		fn_DisplayPlcState();
		fn_ShowWindow();
	}
	SetCursor(hcur);
}
BOOL COASysInfoView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	switch(pMsg->message)
	{
	case WM_CHAR:
		if( (GetFocus() == GetDlgItem(IDC_EDIT_HISTORY))  || (GetFocus() == GetDlgItem(IDC_BUTTON_CONNECT_PLC)))
		{
			//统一成小写字符
			TCHAR pch;
			if(pMsg->wParam >= _T('A') && pMsg->wParam<= _T('Z'))
			{
				pch = pMsg->wParam + (_T('a') - _T('A'));
			}
			else
				pch = pMsg->wParam;


			//进入参数调试状态
			static int nCharCounter = 0;
			static bool bPass = 0; //2005.06.28
			static const TCHAR cPassword[] = _T("optorun"); //
			if(bPass)
			{

				bPass =  (cPassword[nCharCounter+1] == pch);
				if(bPass)
					nCharCounter++;
				else
					nCharCounter = 0;

				if(nCharCounter == sizeof(cPassword)/sizeof(TCHAR) -1-1)
				//if(nCharCounter == _tcslen(cPassword)-1)
				{
					CFormView::PreTranslateMessage(pMsg);
					fn_InDebugState();
					nCharCounter = 0;
					bPass = 0;
					return true;
				}
			}
			else
			{
				bPass = (pch == cPassword[0]);
			}

			static int nzCharCounter = 0;
			static bool bzPass = 0; //2005.06.28
			static const TCHAR czPassword[] = _T("zhangziye"); //
			if(bzPass)
			{
				bzPass =  (czPassword[nzCharCounter+1] == pch);
				if(bzPass)
					nzCharCounter++;
				else
					nzCharCounter = 0;

				if(nzCharCounter == sizeof(czPassword)/sizeof(TCHAR) -1-1)
				//if(nCharCounter == _tcslen(czPassword)-1)
				{
					CFormView::PreTranslateMessage(pMsg);
					fn_InDebugState();
					GetDlgItem(IDC_CHECK_RECORD_VACUUM)->ShowWindow(SW_SHOW);
					nzCharCounter = 0;
					bzPass = 0;
					return true;
				}
			}
			else
			{
				bzPass = (pch == czPassword[0]);
			}

		}
		break;
	}
	return CFormView::PreTranslateMessage(pMsg);
}

// 进入Debug state,改变一些程序运行的关键参数，这些参数只作程序调试之用，不对普通用户开放
void COASysInfoView::fn_InDebugState(void)
{
	//界面告知已经进入DEBUG STATE
	_stprintf(g_strBuffer,TEXT("NOW WE ARE IN DEBUG STATE: %s\r\n"),g_COACnfgDoc.fn_GetLocalTimeStr(),g_plc.m_strCom);
	fn_AppendHistoryText(g_strBuffer);

	//
	gb_ProgrammerDebugNormal = true;

	m_editHistory.SetReadOnly(false);

	//显示所有串口测试界面
	//注	IDC_STATIC_SERIAL_PORT_TEST ~ IDC_STATIC_COM5_TEXT，是在资源中ID连续存在的。如果添加或删除，则需重新考证这段代码！
	CWnd* pwnd;
	for(int i = IDC_STATIC_SERIAL_PORT_TEST;i<=IDC_STATIC_COM5_TEXT;i++)
	{
		pwnd = GetDlgItem(i);
		if(pwnd)
		{
			pwnd->ShowWindow(SW_SHOW);
		}
	}
	fn_ShowCom();

}


//void COASysInfoView::OnBnClickedStaticSerialPortTest()
//{
//	// TODO: Add your control notification handler code here
//
//}
//void COASysInfoView::OnBnDoubleclickedStaticSerialPortTest()
//{
//	//关闭所有已打开串口, 不包括com1
//	//CloseAllOpenedCom();
//	//重新填写comboBox中的串口信息
//	for(int i=IDC_COMBO_COM2;i<=IDC_COMBO_COM5;i++)
//	{
//		CComboBox* pCom = (CComboBox*)GetDlgItem(i);
//		FindCommPort(pCom);
//	}
//	//重新显示串口信息
//	fn_ShowCom();
//}



void COASysInfoView::OnBnClickedButtonCom2Open()
{
	CString str;
	GetDlgItemText(IDC_COMBO_COM2,str);
	g_COM2_ODM.SetComPort(str);
	if(g_COM2_ODM.OpenCom())
	{
	}
	else
	{
		AfxMessageBox(_T("串口没有打开"));
	}	
	fn_ShowCom();
}

void COASysInfoView::OnBnClickedButtonCom2Close()
{
	// TODO: Add your control notification handler code here
	g_COM2_ODM.CloseCom();
	fn_ShowCom();
}


#define _COM_ACK 0x06
#define _COM_NAK 0x15
void COASysInfoView::OnBnClickedButtonCom2Test()
{
	// TODO: Add your control notification handler code here
	if(g_COM2_ODM.IsOpen())
	{
		HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
		//debugging
		char* pcTest;
		pcTest = "H\x06";
			
		g_COM2_ODM.SendCommand(pcTest,strlen(pcTest));
		DWORD dwRdBytes=0;
		g_COM2_ODM.ReceiveData(dwRdBytes,1024,g_chBuffer);
		if(dwRdBytes!=0)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString str;
			MultiByteToWideChar(CP_UTF8,0,g_chBuffer,strlen(g_chBuffer) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			str.Format(_T("%4d-%02d-%02d %02d:%02d:%02d\r\n[SEND]H\r\n[RECV]%s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,g_buffer);
			AfxMessageBox(str);
		}
		else
		{
			AfxMessageBox(_T("Error: Not receive any char.\r\n串口没有接收到任何字符。\r\n\r\n没有连接上 XTC 或 ODM "),MB_ICONSTOP);
			//AfxMessageBox(_T("错误！串口没有接收到任何字符。\r\n\r\n没有连接上 XTC 或 ODM "),MB_ICONSTOP);
		}

		SetCursor(hcur);
	}

}

void COASysInfoView::OnBnClickedButtonCom2Property()
{
	// TODO: Add your control notification handler code here
}

void COASysInfoView::OnBnClickedButtonCom3Open()
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_COMBO_COM3,str);
	g_COM3_IBGL.SetComPort(str);
	if(g_COM3_IBGL.OpenCom())
	{
	}
	else
	{
		AfxMessageBox(_T("串口没有打开"));
	}	
	fn_ShowCom();
}

void COASysInfoView::OnBnClickedButtonCom3Close()
{
	// TODO: Add your control notification handler code here
	g_COM3_IBGL.CloseCom();
	fn_ShowCom();
}

void COASysInfoView::OnBnClickedButtonCom3Test()
{
	// TODO: Add your control notification handler code here
	if(g_COM3_IBGL.IsOpen())
	{
		HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
		//debugging
		char* pcTest;
		pcTest = pComTestStringForGL;//初始时 "IBSQID\x0D"; //2010.04.14
			
		g_COM3_IBGL.SendCommand(pcTest,strlen(pcTest));
		DWORD dwRdBytes=0;
		g_COM3_IBGL.ReceiveData(dwRdBytes,1024,g_chBuffer);
		if(dwRdBytes!=0)
		{
			//2010.04.14
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString str;
			MultiByteToWideChar(CP_UTF8,0,g_chBuffer,strlen(g_chBuffer) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			MultiByteToWideChar(CP_UTF8,0,pcTest,strlen(pcTest) + 1,g_tchBuffer2,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			str.Format(_T("%4d-%02d-%02d %02d:%02d:%02d\r\n[SEND]%s\r\n[RECV]%s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,g_tchBuffer2,g_buffer);
			AfxMessageBox(str);

			/* //2010.04.14删除
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString str;
			MultiByteToWideChar(CP_UTF8,0,g_chBuffer,strlen(g_chBuffer) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			str.Format(_T("%4d-%02d-%02d %02d:%02d:%02d\r\n[SEND]RC\r\n[RECV]%s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,g_buffer);
			AfxMessageBox(str);
			*/
		}
		else
		{
			AfxMessageBox(_T("Error: Not receive any char.\r\n错误！串口没有接收到任何字符。\r\n\r\n没有连接上IB或GL"),MB_ICONSTOP);
		}

		SetCursor(hcur);
	}
}
#include "DlgSetSendChar.h"
//设定IBS/GL 的待发送字符串
void COASysInfoView::OnBnClickedButtonCom3Property()
{
	// TODO: Add your control notification handler code here
	CDlgSetSendChar dlg;
	dlg.m_strComment = _T("请设定 IBS/GL 的测试命令");
	int iLength = strlen(pComTestStringForGL);
	dlg.m_b0D0A = false;
	if( iLength>=2)
	{
		if( pComTestStringForGL[iLength-1] == 0x0D)
		{
			dlg.m_b0D0A = true;	
		}
	}

	if(dlg.m_b0D0A)
	{
		pComTestStringForGL[iLength-1] = 0;   //结尾符号，相当于去除 结尾的 0D，后面补充
		MultiByteToWideChar(CP_UTF8,0,pComTestStringForGL,strlen(pComTestStringForGL)  + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
		pComTestStringForGL[iLength-1] = 0x0D; //重新添加 0x0D
	}
	else
	{
		MultiByteToWideChar(CP_UTF8,0,pComTestStringForGL,strlen(pComTestStringForGL) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
	}
	
	dlg.m_strSend.Format(_T("%s"),g_buffer);
	
	if(IDOK == dlg.DoModal())
	{
		if( dlg.m_strSend.GetLength() < 1024 )
		{
			iLength = WideCharToMultiByte(CP_UTF8,0,dlg.m_strSend,dlg.m_strSend.GetLength()+1,pComTestStringForGL,1024,NULL,NULL);
			pComTestStringForGL[iLength] = '\0';
		}
	}
}

void COASysInfoView::OnBnClickedButtonCom4Open()
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_COMBO_COM4,str);
	g_COM4_GasController.SetComPort(str);
	if(g_COM4_GasController.OpenCom())
	{
	}
	else
	{
		AfxMessageBox(_T("串口没有打开"));
	}	
	fn_ShowCom();
}

void COASysInfoView::OnBnClickedButtonCom4Close()
{
	// TODO: Add your control notification handler code here
	g_COM4_GasController.CloseCom();
	fn_ShowCom();
}

void COASysInfoView::OnBnClickedButtonCom4Test()
{
	// TODO: Add your control notification handler code here
	if(g_COM4_GasController.IsOpen())
	{
		HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
		//debugging
		char* pcTest;
		pcTest = pComTestStringForGasCont;//初始默认"RS232C\x0D\x0A";
			
		g_COM4_GasController.SendCommand(pcTest,strlen(pcTest));
		DWORD dwRdBytes=0;
		g_COM4_GasController.ReceiveData(dwRdBytes,1024,g_chBuffer);
		if(dwRdBytes!=0)
		{
			//2010.04.14
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString str;
			MultiByteToWideChar(CP_UTF8,0,g_chBuffer,strlen(g_chBuffer) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			MultiByteToWideChar(CP_UTF8,0,pcTest,strlen(pcTest) + 1,g_tchBuffer2,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			str.Format(_T("%4d-%02d-%02d %02d:%02d:%02d\r\n[SEND]%s\r\n[RECV]%s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,g_tchBuffer2,g_buffer);
			AfxMessageBox(str);
/*
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString str;
			MultiByteToWideChar(CP_UTF8,0,g_chBuffer,strlen(g_chBuffer) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			str.Format(_T("%4d-%02d-%02d %02d:%02d:%02d\r\n[SEND]RS232C\r\n[RECV]%s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,g_buffer);
			AfxMessageBox(str);

*/
		}
		else
		{
			AfxMessageBox(_T("Error: Not receive any char.\r\n错误！串口没有接收到任何字符。\r\n\r\n没有连接上 GAS CONTROLLER "),MB_ICONSTOP);
		}

		SetCursor(hcur);
	}
}

//设定 发送给 Gas Controller的字符串.(默认在COM4)
void COASysInfoView::OnBnClickedButtonCom4Property()
{
	// TODO: Add your control notification handler code here
	CDlgSetSendChar dlg;
	dlg.m_strComment = _T("请设定 Gas Controller 的测试命令.\rPlease set the command characters for Gas Controller");
	int iLength = strlen(pComTestStringForGasCont);
	dlg.m_b0D0A = false;
	if( iLength>=2)
	{
		if( pComTestStringForGasCont[iLength-1] == 0x0D )
		{
			dlg.m_b0D0A = true;	
		}
	}

	if(dlg.m_b0D0A)
	{
		pComTestStringForGasCont[iLength-1] = 0;   //结尾符号，相当于去除 结尾的 0D0A，后面补充
		MultiByteToWideChar(CP_UTF8,0,pComTestStringForGasCont,strlen(pComTestStringForGasCont)  + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
		pComTestStringForGasCont[iLength-1] = 0x0D; //重新添加 0x0D
	}
	else
	{
		MultiByteToWideChar(CP_UTF8,0,pComTestStringForGasCont,strlen(pComTestStringForGasCont) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
	}
	
	dlg.m_strSend.Format(_T("%s"),g_buffer);
	
	if(IDOK == dlg.DoModal())
	{
		if( dlg.m_strSend.GetLength() < 1024 )
		{
			iLength = WideCharToMultiByte(CP_UTF8,0,dlg.m_strSend,dlg.m_strSend.GetLength(),pComTestStringForGasCont,1024,NULL,NULL);
			pComTestStringForGasCont[iLength] = '\0';
		}
	}
}

void COASysInfoView::OnBnClickedButtonCom5Open()
{
	// TODO: Add your control notification handler code here
	CString str;
	GetDlgItemText(IDC_COMBO_COM5,str);
	g_COM5_VGC500.SetComPort(str);
	if(g_COM5_VGC500.OpenCom())
	{
	}
	else
	{
		AfxMessageBox(_T("串口没有打开"));
	}	
	fn_ShowCom();
}

void COASysInfoView::OnBnClickedButtonCom5Close()
{
	// TODO: Add your control notification handler code here
	g_COM5_VGC500.CloseCom();
	fn_ShowCom();
}



void COASysInfoView::OnBnClickedButtonCom5Test()
{
	// TODO: Add your control notification handler code here
	if(g_COM5_VGC500.IsOpen())
	{
		//pCom5TestString
		HCURSOR hcur = SetCursor(LoadCursor(NULL,IDC_WAIT));
		//debugging
		char* pcTest;
		pcTest = pCom5TestString;//"VER?\x0D\x0A";
			
		g_COM5_VGC500.SendCommand(pcTest,strlen(pcTest));
		DWORD dwRdBytes=0;
		g_COM5_VGC500.ReceiveData(dwRdBytes,1024,g_chBuffer);
		if(dwRdBytes!=0)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString str;
			MultiByteToWideChar(CP_UTF8,0,g_chBuffer,strlen(g_chBuffer) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			MultiByteToWideChar(CP_UTF8,0,pcTest,strlen(pcTest) + 1,g_tchBuffer2,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
			str.Format(_T("%4d-%02d-%02d %02d:%02d:%02d\r\n[SEND]%s\r\n[RECV]%s"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,g_tchBuffer2,g_buffer);
			AfxMessageBox(str);
		}
		else
		{
			AfxMessageBox(_T("Error: Not receive any data.\r\n错误！串口没有接收到任何字符。\r\n\r\n没有连接上 VCC500 "),MB_ICONSTOP);
		}

		SetCursor(hcur);
	}
}

#include "Vcc500PropertyDlg.h"
void COASysInfoView::OnBnClickedButtonCom5Property()
{
	// TODO: Add your control notification handler code here
	Vcc500PropertyDlg dlg;
	int iLength = strlen(pCom5TestString);
	dlg.m_b0D0A = false;
	if( iLength>=2)
	{
		if( pCom5TestString[iLength-2] == 0x0D && pCom5TestString[iLength-1]==0x0A)
		{
			dlg.m_b0D0A = true;	
//			pCom5TestString[iLength-2] = 0; //结尾符号，相当于去除 结尾的 0D0A，后面补充
		}
	}

	if(dlg.m_b0D0A)
	{
		pCom5TestString[iLength-2] = 0;   //结尾符号，相当于去除 结尾的 0D0A，后面补充
		MultiByteToWideChar(CP_UTF8,0,pCom5TestString,strlen(pCom5TestString)  + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
		pCom5TestString[iLength-2] = 0x0D; //重新添加 0x0D
	}
	else
	{
		MultiByteToWideChar(CP_UTF8,0,pCom5TestString,strlen(pCom5TestString) + 1,g_buffer,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
	}
	
	dlg.m_strSend.Format(_T("%s"),g_buffer);
	
	if(IDOK == dlg.DoModal())
	{
		if( dlg.m_strSend.GetLength() < 1024 )
		{
			iLength = WideCharToMultiByte(CP_UTF8,0,dlg.m_strSend,dlg.m_strSend.GetLength(),pCom5TestString,1024,NULL,NULL);
			pCom5TestString[iLength] = '\0';
		}
	}
	
}

// 显示COM2~COM5的相关界面（根据内部参数）
int COASysInfoView::fn_ShowCom(void)
{
    if(g_COM2_ODM.IsOpen())
	{
		GetDlgItem(IDC_COMBO_COM2)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM2_OPEN)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM2_CLOSE)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM2_TEST)->EnableWindow(true);
	//	GetDlgItem(IDC_BUTTON_COM2_PROPERTY)->EnableWindow(false);		
	}
	else
	{
		GetDlgItem(IDC_COMBO_COM2)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM2_OPEN)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM2_CLOSE)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM2_TEST)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM2_PROPERTY)->EnableWindow(true);
	}

    if(g_COM3_IBGL.IsOpen())
	{
		GetDlgItem(IDC_COMBO_COM3)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM3_OPEN)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM3_CLOSE)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM3_TEST)->EnableWindow(true);
	//	GetDlgItem(IDC_BUTTON_COM3_PROPERTY)->EnableWindow(false);		
	}
	else
	{
		GetDlgItem(IDC_COMBO_COM3)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM3_OPEN)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM3_CLOSE)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM3_TEST)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM3_PROPERTY)->EnableWindow(true);
	}

	if(g_COM4_GasController.IsOpen())
	{
		GetDlgItem(IDC_COMBO_COM4)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM4_OPEN)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM4_CLOSE)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM4_TEST)->EnableWindow(true);
	//	GetDlgItem(IDC_BUTTON_COM4_PROPERTY)->EnableWindow(false);		
	}
	else
	{
		GetDlgItem(IDC_COMBO_COM4)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM4_OPEN)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM4_CLOSE)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM4_TEST)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM4_PROPERTY)->EnableWindow(true);
	}
		  
	if(g_COM5_VGC500.IsOpen())
	{
		GetDlgItem(IDC_COMBO_COM5)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM5_OPEN)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM5_CLOSE)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM5_TEST)->EnableWindow(true);
	//	GetDlgItem(IDC_BUTTON_COM5_PROPERTY)->EnableWindow(false);		
	}
	else
	{
		GetDlgItem(IDC_COMBO_COM5)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM5_OPEN)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON_COM5_CLOSE)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM5_TEST)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_COM5_PROPERTY)->EnableWindow(true);
	}
	return 0;
}

void COASysInfoView::OnStnClickedStaticCom2Text()
{
	// TODO: Add your control notification handler code here
	AfxMessageBox(_T("xxxx"));
}


void COASysInfoView::OnStnClickedStaticCom3Text()
{
	// TODO: Add your control notification handler code here
}

void COASysInfoView::OnStnClickedStaticCom4Text()
{
	// TODO: Add your control notification handler code here
}

void COASysInfoView::OnStnClickedStaticCom5Text()
{
	// TODO: Add your control notification handler code here
}

void COASysInfoView::OnBnClickedCheckRecordVacuum()
{
	// TODO: Add your control notification handler code here
	if( BST_CHECKED == ((CButton*)GetDlgItem(	IDC_CHECK_RECORD_VACUUM ))->GetCheck())
	{
		g_bOffLineDebug = true;
	}
	else
	{
		g_bOffLineDebug = false;
	}
}

//BOOL COASysInfoView::OnEraseBkgnd(CDC* pDC)
//{
//	// TODO: Add your message handler code here and/or call default
//	if( _en_Coater_UNKNOWN_TYPE == g_enOptorunCoaterType )
//	{
//		pDC->TextOutW(100,100,CString(TEXT("UnKnown Type")));
//	}
//
//	return CFormView::OnEraseBkgnd(pDC);
//}

void COASysInfoView::OnStnClickedStaticHelpTitle()
{
	// TODO: Add your control notification handler code here
}
