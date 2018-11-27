#include "StdAfx.h"
#include "VGC500.h"

CVGC500::CVGC500(void)
{
	InitComm();
}

CVGC500::~CVGC500(void)
{
}

//VCC500,
void CVGC500::InitComm(void)
{
	//�ȴ�ʱ�䣬�����÷��μ� ReciveData����
	m_dwWaitTime = 500; //200ms
	m_dwSleepTime = 10; //10ms

	//֡��β��ʽ
	m_enumCommEndType = enCommNone;
	//֡��β�����ַ���
	*m_pszRecEnd     = 0x0D;
	*(m_pszRecEnd+1) = 0x15;
	*(m_pszRecEnd+2) = 0;
	
	m_strCom = TEXT("COM5");
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