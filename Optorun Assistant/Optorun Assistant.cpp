// Optorun Assistant.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "MainFrm.h"

#include "Optorun AssistantDoc.h"
#include "OASysInfoView.h"

#include "OACnfgDoc.h"
#include "pictureex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// COptorunAssistantApp
BEGIN_MESSAGE_MAP(COptorunAssistantApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &COptorunAssistantApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	//ON_THREAD_MESSAGE( MAIN_FRM_TEST, OnMainFrmTest )
END_MESSAGE_MAP()

// COptorunAssistantApp construction

COptorunAssistantApp::COptorunAssistantApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only COptorunAssistantApp object

COptorunAssistantApp theApp;


// COptorunAssistantApp initialization

BOOL COptorunAssistantApp::InitInstance()
{
	//实例只启动一次
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, TEXT("Optorun Assistant"));
	// 检查错误代码
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// 如果已有互斥量存在则释放句柄并复位互斥量
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		// 程序退出
		AfxMessageBox(TEXT("Error:\r\nAnother copy of \"Optorun Assisatant\" is already running!\r\n错误：Optorun Assistant已经在运行。"));
		return FALSE;
	}

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	// Next 2 lines are canceled by zhangziye,2007.09.12
//	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
//	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)
    
	//::ShowWindow(::FindWindow(TEXT("Shell_TrayWnd"),NULL),SW_HIDE);//隐藏任务栏
	//

	CString sPath;
    GetModuleFileName(NULL,sPath.GetBufferSetLength (MAX_PATH+1),MAX_PATH);
    sPath.ReleaseBuffer();
    int nPos = sPath.ReverseFind(TEXT('\\'));
    sPath = sPath.Left(nPos);
	sPath += TEXT("\\OptorunAssistant.ini");
	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(sPath);

	//
	HANDLE hFile = CreateFile(m_pszProfileName,0,FILE_SHARE_READ,0,OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,NULL);
	if(GetLastError()==0)
	{
		CloseHandle(hFile);
	}
	else
	{
		//没有找到原始配置文件，即从 资源中析出　.ini文件.
		HRSRC hSrc = FindResource(NULL,MAKEINTRESOURCE(IDR_OA_INI_FILE),_T("OA_FILE_DATA"));
		if(hSrc == NULL)	
			return false;

		HGLOBAL hGlobal = LoadResource(NULL,hSrc);
		if(hGlobal == NULL)	
			return false;

		LPVOID lp = LockResource(hGlobal);
		DWORD dwSize = SizeofResource(NULL,hSrc);

		CFile file;
		if(file.Open(m_pszProfileName,CFile::modeCreate|CFile::modeWrite))
		{
			file.Write(lp,dwSize);  //lp:缓冲区指针。从缓冲区的数据写入CFile对象file中，dwsize为缓冲区调用字节数
			file.Close();
		}
		FreeResource(hGlobal);				
	}

	// Register the application's document templates.  Document templates
	// serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(COptorunAssistantDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(COASysInfoView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	//m_pMainWnd->FindWindow(_T("Shell_TrayWnd"),NULL)->ShowWindow(SW_HIDE);//隐藏任务栏
	//虽然可以隐藏任务栏，但最大化并不难 到达屏幕底部(即不能占据原任务栏位置)。而且需要退出前再恢复任务栏。
	//不算是一个好的方法.

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	m_nCmdShow = SW_SHOWMAXIMIZED; //2007.09.10
	/*
	在如下函数内MFC 开始　CMainWnd, CView 的启动.
	*/
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	//m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand


	CMainFrame* pMainWnd = (CMainFrame*)m_pMainWnd; 
	COptorunAssistantDoc* pDoc = (COptorunAssistantDoc*)pMainWnd->GetActiveDocument();
	
	int nFullWidth=GetSystemMetrics(SM_CXSCREEN);
	int nFullHeight=GetSystemMetrics(SM_CYSCREEN);
	//::MoveWindow(pMainWnd->m_hWnd,0,0,nFullWidth,nFullHeight,1);
	BOOL x = ::SetWindowPos(m_pMainWnd->m_hWnd,HWND_TOP,0,0,nFullWidth,nFullHeight,SWP_SHOWWINDOW);

	//TEST
#define MAKE_DWORD(a3,a2,a1,a0) ((BYTE)a3<<24 | (BYTE)a2<<16 | (BYTE)a1<<8 | (BYTE)a0)
#define EXTRACT_DWORD(iByte,dwX) ( (BYTE) ( dwX >> ((iByte%4)<<3 )))

	DWORD Y = MAKE_DWORD(1,2,3,4);
	BYTE  Z = EXTRACT_DWORD(1,Y); //取出的是 a1 , 结果 Z=3

	return TRUE;
}

LRESULT COptorunAssistantApp::OnMainFrmTest(WPARAM wParam, LPARAM lParam)
{
	AfxMessageBox(LPCTSTR(lParam));
	return 0;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	CPictureEx m_picLogo;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOGO, m_picLogo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void COptorunAssistantApp::OnAppAbout()
{
	CAboutDlg aboutDlg;

	aboutDlg.DoModal();
}

// COptorunAssistantApp message handlers
BOOL COptorunAssistantApp::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	//中转接力
	if(pMsg->message == MAIN_FRM_TEST)
	{//直接中转会造成死循环
		::PostMessage(GetMainWnd()->GetSafeHwnd(),pMsg->message,pMsg->wParam,pMsg->lParam);
		return 0; //return 1?
	}
	else if( OA_APP_VACUUM_THREAD_ERR_OCCUR == pMsg->message )
	{
		::PostMessage(GetMainWnd()->GetSafeHwnd(),MAIN_FRM_VACUUM_THREAD_ERR_OCCUR,pMsg->wParam,pMsg->lParam);
		return 0;
	}
	
	else if( OA_APP_DISPLAY_VAC_VALUE == pMsg->message )
	{
		::PostMessage(GetMainWnd()->GetSafeHwnd(),MAIN_FRM_DISPLAY_VAC_VALUE,pMsg->wParam,pMsg->lParam);
		return 0;
	}
	
	return CWinApp::PreTranslateMessage(pMsg);
}

extern TCHAR g_buffer[];
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//CString str;

	TCHAR cbuf[1024];
	_stprintf(g_buffer,TEXT("Optorun Assistant Version: %.3f\x0d\x0a"),g_dProgramVersion);
	SetDlgItemText(IDC_STATIC_PROGRAM_VERSION,g_buffer);
	
	MultiByteToWideChar(CP_UTF8,0,__DATE__,strlen(__DATE__) + 1,cbuf,1024); // strlen(..) + 1，保证连字符串的结束符号一并转化 
	_stprintf(g_buffer,TEXT("Last update: %s\x0d\x0a"),cbuf);
	SetDlgItemText(IDC_STATIC_COMPILE_DATE,g_buffer);

	if(m_picLogo.Load(MAKEINTRESOURCE(IDR_LOGO_GIF),_T("GIF")))
	{
		m_picLogo.SetBkColor(GetSysColor(COLOR_3DFACE));
		m_picLogo.Draw();	
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
