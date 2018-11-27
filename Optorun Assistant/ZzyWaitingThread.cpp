// ZzyWaitingThread.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "ZzyWaitingThread.h"


// CZzyWaitingThread

IMPLEMENT_DYNCREATE(CZzyWaitingThread, CWinThread)

CZzyWaitingThread::CZzyWaitingThread()
{
}

CZzyWaitingThread::~CZzyWaitingThread()
{
}

BOOL CZzyWaitingThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CZzyWaitingThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CZzyWaitingThread, CWinThread)
	ON_THREAD_MESSAGE(WM_ZZY_WAITING_THREAD_WORK,  OnZzyWaiting)
END_MESSAGE_MAP()


// CZzyWaitingThread message handlers

void CZzyWaitingThread::OnZzyWaiting(WPARAM wParam, LPARAM lParam)
{
	_stZzyWaiting* pst = (_stZzyWaiting*)lParam;

	DWORD dwStartTickCount = GetTickCount();
	bool bBreak = false;
	while( GetTickCount()-dwStartTickCount < pst->dwWaitingMilliSeconds )
	{
		Sleep(10);
		EnterCriticalSection(&pst->cs_ZzyWaiting);
			bBreak = pst->bCancelWaiting;
		LeaveCriticalSection(&pst->cs_ZzyWaiting);

		if(bBreak)
			break;
	}

	EnterCriticalSection(&pst->cs_ZzyWaiting);
		pst->bWaiting  = false; //È¡ÏûµÈ´ý×´Ì¬
	LeaveCriticalSection(&pst->cs_ZzyWaiting);

}