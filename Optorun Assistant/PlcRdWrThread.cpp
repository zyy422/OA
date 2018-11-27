// PlcRdWrThread.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "PlcRdWrThread.h"
#include "OACnfgDoc.h"

//extern COACnfgDoc g_COACnfgDoc;
// CPlcRdWrThread

//可将 g_hEvent_plc 和 g_plc制作成内部变量，AfxBeginThread创建线程时先suspend，设定好这些变量后再Resume thread.
//本类的使用方法虽可行，但别扭，违反了类的封装性特点。
IMPLEMENT_DYNCREATE(CPlcRdWrThread, CWinThread)

CPlcRdWrThread::CPlcRdWrThread(): m_dwLastError(0)
{
}

CPlcRdWrThread::~CPlcRdWrThread()
{
}

BOOL CPlcRdWrThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CPlcRdWrThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CPlcRdWrThread, CWinThread)
	ON_THREAD_MESSAGE(WM_PLC_RD_WR_MSG,On_Plc_RW_Command)
END_MESSAGE_MAP()


// CPlcRdWrThread message handlers
void CPlcRdWrThread::On_Plc_RW_Command(WPARAM wParam,  LPARAM lParam) 
{

}

bool CPlcRdWrThread::SetPLCCIOBitEvent(WORD wAddress, WORD wBit)
{
	//WritePLC(enum _enumSysmacWrType enType, int nBeginWord, int nWriteNumber, int  nWriteArray[], char* pszSendBuf, char* pszRecvBuf)
	int nWriteValue = 1 << wBit;

	DWORD dw = WaitForSingleObject(g_hEvent_plc,_VACUUM_PROCESS_WAIT_OBJECT_TIME);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT;
		return false;
	}
	
	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	bool b = g_plc.WritePLC(enWriteCIO,wAddress,1,&nWriteValue,vt_pszSendBuf,vt_pszRecvBuf);	
	SetEvent(g_hEvent_plc);
	if(!b)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_COMM;//通信出错
	}
	return b;
}


bool CPlcRdWrThread::WritePLCCIOEvent(WORD wAddress, WORD wValue)
{
	DWORD dw = WaitForSingleObject(g_hEvent_plc,_VACUUM_PROCESS_WAIT_OBJECT_TIME);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT;
		return false;
	}
	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	int  nWriteValue = wValue;
	bool b = g_plc.WritePLC(enWriteCIO,wAddress,1,&nWriteValue,vt_pszSendBuf,vt_pszRecvBuf);	
	SetEvent(g_hEvent_plc);
	if(!b)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_COMM;//通信出错
	}
	return b;
}

//带Event的读取CIO
bool CPlcRdWrThread::ReadPLCCIOEvent(WORD wAddress,WORD &wReadValue)
{
	DWORD dw = WaitForSingleObject(g_hEvent_plc,_VACUUM_PROCESS_WAIT_OBJECT_TIME);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT;
		return false;
	}
	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	bool b = g_plc.ReadPLC(enReadCIO,wAddress,1,&wReadValue,vt_pszSendBuf,vt_pszRecvBuf);	
	SetEvent(g_hEvent_plc); 
	if(!b)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_COMM;//通信出错
	}
	return b;
}

//bool bDec，表示是否10进制
//true:  10进制, 默认为10进制
//false: 16进制
bool CPlcRdWrThread::ReadPLCDMEvent(WORD wAddress,WORD &wReadValue,bool bDec /*=true*/) //第二个参数为引用
{
	DWORD dw = WaitForSingleObject(g_hEvent_plc,_VACUUM_PROCESS_WAIT_OBJECT_TIME);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT;
		return false;
	}
	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	bool b = g_plc.ReadPLCex(enReadDM,wAddress,1,&wReadValue,bDec,vt_pszSendBuf,vt_pszRecvBuf);	
	SetEvent(g_hEvent_plc); 
	if(!b)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_COMM;//通信出错
	}
	return b;
}
// 写DM区
bool CPlcRdWrThread::WritePLCDMEvent(WORD wAddress, WORD wWriteValue)
{
	DWORD dw = WaitForSingleObject(g_hEvent_plc,_VACUUM_PROCESS_WAIT_OBJECT_TIME);
	if(WAIT_TIMEOUT==dw || WAIT_FAILED==dw)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_WAIT_OBJECT;
		return false;
	}

	int iValue = wWriteValue;

	ResetEvent(g_hEvent_plc);//先阻塞其他线程的plc读取操作
	bool b = g_plc.WritePLC(enWriteDM,wAddress,1,&iValue,vt_pszSendBuf,vt_pszRecvBuf);	
	SetEvent(g_hEvent_plc); 
	if(!b)
	{
		m_dwLastError = _VACUUM_PROCESS_LAST_ERR_COMM;//通信出错
	}
	return b;
}
