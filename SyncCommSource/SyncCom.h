#pragma once
#define _COM_PORT_BUF_LENGTH         512
#define _SYN_COM_END_STR_LENGTH      12

//存储系统函数出错字符串的缓冲区，免得外部分配
#define _SYNC_COM_API_ERR_STR_LENGTH 512  

//派生类的  Error 类型，须在 _SYNCCOM_DERIVE_ERROR_ 之上定义. 100~65535
#define _SYNCCOM_DERIVE_ERROR_					100 

#define _SYNCCOM_ERROR_NONE_					0
#define _SYNCCOM_ERROR_SYS_API_					1  //系统API函数出错，此时m_dwLastSysApiError将记录系统的 lasterror.
#define _SYNCCOM_ERROR_HANDLE_					2  //COM 端口的 Handle 非法(COM口未打开)
#define _SYNCCOM_ERROR_OPEN_COM_				3  //COM口打开失败，可能原因 1，该口被其他程序占用；2，该口不存在
#define _SYNCCOM_ERROR_RENAME_COM_				4  //更改待打开串口出错，当前处于打开状态。必须先关闭。
#define _SYNCCOM_ERROR_COM_OVER_255_			5  //更改的串口端口号超过 255，出错

#define _SYNCCOM_ERROR_RECV_TIMEOUT_			20  //接收串口数据超时，没有接收到字符
#define _SYNCCOM_ERROR_RECV_TIMEOUT_NO_END		21  //接收串口数据超时，接收到字符，但在限定时间内没有检测到帧结尾符号


class CSyncCom
{
public:
	CSyncCom(void);
public:
	virtual ~CSyncCom(void);
public:
	//串口相关参数
	HANDLE		 m_hCom;
	CString		 m_strCom;
	COMMTIMEOUTS m_CommTimeouts;
	DCB          m_dcb;

	//读串口循环过程中的最大等待时间
	DWORD        m_dwWaitTime;
	//读串口循环过程中尚未有 字符 且通信未完成 的避让线程时间。
	DWORD		 m_dwSleepTime;

	//最后一次出错代码//没有错误发生时，将不记录错误代码
	DWORD        m_dwLastError;       
	//最后一次出错，如果是API等系统函数原因，则记录错误代码。如果是其它类型原因，则本错误代码无意义。
	DWORD        m_dwLastSysApiError;

	//收发缓冲区，仪器设备使用的是 char
	char  m_pszSendBuf[_COM_PORT_BUF_LENGTH];
	char  m_pszRecvBuf[_COM_PORT_BUF_LENGTH];
	DWORD m_dwBytesRead;

	TCHAR m_pszAPIErrStrBuf[_SYNC_COM_API_ERR_STR_LENGTH];//取得API函数错误的内容
	TCHAR m_pszAPIErrFunBuf[_SYNC_COM_API_ERR_STR_LENGTH];//出错API函数记录

/************************************************************************************
	帧尾标志字符(串):
    在应答式(一问一答)串口通信中, 每次通信的内容为一帧.
		有的通信过程, 有帧头和帧尾来标识完整的一帧;
		有的只有帧尾标志字符(串).
	本类提供检测 帧尾标志字符(串). 以完成一次读取帧的动作。如有帧头需判断，则需调用方进行。
**************************************************************************************/
enum _enumCommFrameEndType
{
	enCommNone,       //没有特征字符(串). 串口只要接收到字符后,就立即退出.  //SleepTime足够长的话,能接收完整的帧.
	enCommOneChar,    //一个特征字符,通常为 <CR> ,即 0x0D。（从头找，找到特征字符则退出）
	enCommSubString,  //特征字符串（连续2个字符以上的字符串）,比如 OMRON PLC要求 "*\x0D"
	enCommEitherChar,  //两个字母中的任意一个(2个字符中的任一个)
	enCommOneCharEx   //一个特征字符,（仅查找结尾字符）。是 enCommOneChar 的扩展。
};

	_enumCommFrameEndType m_enumCommEndType;
	char m_pszRecEnd[_SYN_COM_END_STR_LENGTH]; //接收帧字符串结尾标志符号

public:
	inline BOOL IsOpen(void) {return (m_hCom != INVALID_HANDLE_VALUE); }
public:
	void CloseCom(void);
public:
	virtual void InitComm(void);
public:
	bool SetComPort(unsigned int  iPort);
public:
	bool SetComPort(LPCTSTR strComPort);
public:
	bool SetComPortDCB(DCB& dcb);
public:
	bool SetWaitTime(DWORD dwNewWaitTime);				//2018-8-30 未使用
public:
	bool SetSleepTime(DWORD dwNewSleepTime);
public:
	bool OpenCom(void);
public:
	BOOL SendCommand(const wchar_t* strCmd, unsigned int iLength = 0, char* pcBuf=NULL);
public:
	BOOL SendCommand(const char* pCmd , unsigned int iLength = 0);
public:
	void SetFrameEndStr(const char* pszEnd);
public:
	bool ReceiveData(DWORD& dwBytesRead, unsigned int iBufSize = 0, char* pcBuf = NULL);
public:

	DWORD GetLastError(void)
	{
		return m_dwLastError;
	}
public:
	bool GetApiLastErrorStr(DWORD dwApiErrCode, LPTSTR lpszBuf = NULL, int iBufSize = 0);
public:

	// are there any characters read last time
	bool IsLastReadAnyChar(void)
	{
		return m_dwBytesRead!=0;
	}
};
