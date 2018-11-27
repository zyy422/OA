#include "StdAfx.h"
#include "IBGLCom.h"

CIBGLCom::CIBGLCom(void)
{
	InitComm();
}

CIBGLCom::~CIBGLCom(void)
{
}

void CIBGLCom::InitComm(void)
{
	//等待时间，具体用法参见 ReciveData函数
	m_dwWaitTime = 500; //200ms
	m_dwSleepTime = 10; //10ms

	//帧结尾方式
	m_enumCommEndType = enCommNone;
	//帧结尾符号字符串
	*m_pszRecEnd     = 0x0D;
	*(m_pszRecEnd+1) = 0x15;
	*(m_pszRecEnd+2) = 0;
	
	//以下参数默认可读取 OMRON PLC
	m_strCom = TEXT("COM3");
	m_hCom  = INVALID_HANDLE_VALUE;
	
	m_dcb.DCBlength = sizeof(DCB);
	m_dcb.BaudRate = 9600; 
	m_dcb.ByteSize =8; 
	m_dcb.Parity = NOPARITY; 
	m_dcb.StopBits = ONESTOPBIT ;
	m_dcb.fBinary = TRUE; 
	m_dcb.fParity = false;

	m_CommTimeouts.ReadIntervalTimeout = 100; //ms
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 100; 
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000; 
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 100;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;
}
