/*********************************************************************************************************
文件名:		SyncCom.cpp
Comment:    同步串口收发类 CSyncCom 的实现文件.
Author:     cn_zhangziye@yahoo.com.cn
Last time:  2007.09.05
版权所有:   cn_zhangziye@yahoo.com.cn

说明1：这里所谓的同步收发，并不是指串口的同步执行。而是指阻塞当前线程。
(同步IO会阻塞当前的调用线程，而异步IO则允许发起IO请求的调用线程继续执行，等到IO请求被处理后，会通知调用线程。)
事实上，无论对于同步IO，还是异步IO,当IO请求发送到device driver后，device driver的执行总是异步的。
       当界面程序直接调用本类时，因为会阻塞当前线程，会显得界面变慢。对简单使用无妨。
	   稍复杂一点，可开线程运行。 主程序（界面）与线程之间通过消息进行通信。
说明2：仪器的通信通常采用应答方式。
       即上位机(pc)发送命令后，等待下位机(仪器)对当前命令的响应。pc程序再对响应内容进行分析，以便下一步操作。
	   本类针对的就是这种应答方式。
	   多数为一问一答的情况，上位机比较容易编程。
说明3：本类的 InitComm() 中设置的通信方式没有硬件握手设置，适用于 3线 连接。
       使用者可根据仪器需要自行设置通信方式。
说明4：本类突出的一点就是 设置了 帧结尾 判断。

类 CSyncCom 使用步骤:
1, 包含 本类的头文件 
   #include "SyncCom.h"
2, 添加成员变量  CSyncCom m_SyncCom. (变量名可自定)
3, 设定 判断帧结尾的方式、COM端口号、端口参数等。 参见 InitComm()函数.
   本 InitComm函数可直接用于与 OMRON SYSMAC CS/CJ 系列PLC通信(除了端口号外). 
4, 打开串口 OpenCom
  
5, 发送命令  SendCommand  //发送用命令字符串格式根据下位机的协议来定。
6, 接收命令  ReciveData   

循环 5~6

//其余的函数，意义明显。

利用成员变量设置接收帧结尾方式
m_SyncCom.m_enumCommEndType = CSyncCom::enCommNone;

注意1：本类不是线程安全型。多线程应用情况应予以注意，须自行添加 Event 等 同步手段。
注意2：内部帧缓冲区大小为512 ( _COM_PORT_BUF_LENGTH )。在默认使用内部缓冲区时，由于没有作溢出检查，所以可接收的帧长度必须小于512字节。
       
bug 1: 不可以在某个函数内用 CSyncCom myCom 进行收发。必须定义在函数体外，或类成员。
       是 SetCommState 函数报错。LastError = 87, 解释为 "参数不正确"。
       原因不明，未解决。
*********************************************************************************************************/

#include "stdafx.h"
#include "SyncCom.h"

CSyncCom::CSyncCom(void):m_dwBytesRead(0)
{
	m_dwLastError = _SYNCCOM_ERROR_NONE_;
	*m_pszAPIErrStrBuf = _T('\0');
	*m_pszAPIErrFunBuf = _T('\0');
	*m_pszRecvBuf = _T('\0');
	*m_pszSendBuf = _T('\0');
	InitComm();
}

CSyncCom::~CSyncCom(void)
{
	CloseCom();
}


//本函数无需返回值，因为不可能失败.
void CSyncCom::CloseCom(void)
{
	if( IsOpen() )
	{
		::CloseHandle(m_hCom);	//析构前，关闭打开的 COM 口.
	}
	m_hCom = INVALID_HANDLE_VALUE;
}

void CSyncCom::InitComm(void)
{
	//等待时间，具体用法参见 ReciveData函数
	m_dwWaitTime = 200; //200ms //总的最长等待时间
	m_dwSleepTime = 10; //10ms  //单次查询间隔

	//以下参数默认可读取 OMRON PLC
	//帧结尾方式
	m_enumCommEndType = enCommSubString;
	//帧结尾符号字符串
	*m_pszRecEnd = '*';
	*(m_pszRecEnd+1) = 0x0D;
	*(m_pszRecEnd+2) = 0;
	
	m_strCom = TEXT("COM1");
	m_hCom  = INVALID_HANDLE_VALUE;
	
	m_dcb.DCBlength = sizeof(DCB);
	m_dcb.BaudRate  = 38400; 
	m_dcb.ByteSize  = 7; 
	m_dcb.Parity    = EVENPARITY;
	m_dcb.StopBits  = TWOSTOPBITS; 
	m_dcb.fParity   = true; 
	m_dcb.fBinary   = true;

	m_CommTimeouts.ReadIntervalTimeout = 100; //ms
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 100; 
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000; 
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 100;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;
}

bool CSyncCom::SetComPort(unsigned int  iPort)
{
	if( IsOpen() )
	{
		m_dwLastError = _SYNCCOM_ERROR_RENAME_COM_;
		return false;
	}
	else
	{

		if( iPort >= 256 )
		{
			m_dwLastError = _SYNCCOM_ERROR_COM_OVER_255_;
			return false;
		}
		else
		{
			m_strCom.Format(TEXT("COM%d"),iPort);
			return true;
		}
	}
}

bool CSyncCom::SetComPort(LPCTSTR strComPort)
{
	//端口打开的情况下，不允许修改端口名称！
	if(IsOpen())
	{
		m_dwLastError = _SYNCCOM_ERROR_RENAME_COM_;
		return false;
	}
	else
	{
		m_strCom = strComPort;
		return true;
	}
}

bool CSyncCom::SetComPortDCB(DCB& dcb)
{
	memcpy(&m_dcb,&dcb,sizeof(DCB));
	if( IsOpen() )
	{//立即更新！
		return ::SetCommState(m_hCom,&m_dcb);
	}
	return true;	
}

bool CSyncCom::SetWaitTime(DWORD dwNewWaitTime)
{
	m_dwWaitTime = dwNewWaitTime;
	return true;
}

bool CSyncCom::SetSleepTime(DWORD dwNewSleepTime)
{
	m_dwSleepTime = dwNewSleepTime;
	return true;
}

bool CSyncCom::OpenCom(void)
{
	if( INVALID_HANDLE_VALUE != m_hCom ) 
	{
		CloseCom(); //先关闭，再重新打开！
		//return true;
	}
	
	m_hCom = ::CreateFile( (LPCTSTR)m_strCom ,
		GENERIC_READ | GENERIC_WRITE, 
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(m_hCom != INVALID_HANDLE_VALUE)
	{
		if( ::SetCommMask(m_hCom, EV_ERR|EV_RXCHAR) )
		{
			if(::SetCommTimeouts( m_hCom, &m_CommTimeouts ))
			{
				if(::SetCommState(m_hCom,&m_dcb))
				{
					::SetupComm( m_hCom, 512,512) ; 
					::PurgeComm( m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR	| PURGE_RXCLEAR ); 
					return true;
				}
				else
				{
					_tcscpy(m_pszAPIErrFunBuf, _T("CSyncCom::OpenCom -> ::SetCommState"));					
				}
			}
			else
			{
				_tcscpy(m_pszAPIErrFunBuf, _T("CSyncCom::OpenCom -> ::SetCommTimeouts"));					
			}
		}
		else
		{
			_tcscpy(m_pszAPIErrFunBuf, _T("CSyncCom::OpenCom -> ::SetCommMask"));
		}
		m_dwLastError = _SYNCCOM_ERROR_SYS_API_ ;
		m_dwLastSysApiError = ::GetLastError();
		CloseCom();
	}
	else
	{
		m_dwLastError = _SYNCCOM_ERROR_OPEN_COM_ ;
		m_dwLastSysApiError = ::GetLastError();
	}
	return false;
}

//此函数多unicode 类型的字符串，用于unicode转换为非unicode，（char类型），因为串口只能传送char类型数据
BOOL CSyncCom::SendCommand(const wchar_t* strCmd, unsigned int iLength, char* pcBuf)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		m_dwLastError = _SYNCCOM_ERROR_HANDLE_ ;
		return false;
	}

	char* pCmd = ( ( NULL != pcBuf ) ? pcBuf : m_pszSendBuf );    //转换为char类型缓冲区是否存在，不存在自己设置一个缓冲区
	DWORD dwBytesToWrite =  (( 0!=iLength) ?  iLength : wcslen(strCmd)  );		//看传入的数据长度是否有设置，未设置程序自动设置为传入数据的大小
	
	DWORD dwBytesWritten;
	::WideCharToMultiByte(CP_ACP,            // ANSI Code Page
			0,                 // no flags
			strCmd,                 // source widechar string
			-1,                // assume NUL-terminated
			pCmd,                 // target buffer
			_COM_PORT_BUF_LENGTH, // target buffer length
			NULL,              // use system default char
			NULL);             // don''t care if default used

	//清空收发缓冲区数据
	PurgeComm(m_hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
	return WriteFile(m_hCom,pCmd,dwBytesToWrite,&dwBytesWritten,NULL); 
}

BOOL CSyncCom::SendCommand(const char* pCmd ,unsigned int iLength)
{
	if(m_hCom == INVALID_HANDLE_VALUE)
	{
		m_dwLastError = _SYNCCOM_ERROR_HANDLE_ ;
		return false;
	}

	DWORD dwBytesToWrite = ( ( 0 != iLength ) ? iLength : strlen(pCmd) );
	DWORD dwBytesWritten;
	//清空收发缓冲区数据
	PurgeComm(m_hCom,PURGE_TXCLEAR|PURGE_RXCLEAR);
	if(WriteFile(m_hCom,pCmd,dwBytesToWrite,&dwBytesWritten,NULL))
	{
		return true;
	}
	else
	{
		_tcscpy(m_pszAPIErrFunBuf, _T("CSyncCom::SendCommand -> ::WriteFile"));
		m_dwLastError = _SYNCCOM_ERROR_SYS_API_;
		m_dwLastSysApiError = ::GetLastError();
		return false;
	}
}

/*******************************************************************************************
函数名称：CSyncCom::SetFrameEndStr(const char* pszEnd)
输入参数：const char* pstrEnd，新的 串口通信 结束标志字符串
返回参数：无
comment:  针对某个仪器，必须调用本函数进行结尾符号设定。 才能正确接收。
          可进一步分成 单字符( <CR=0x0D>, VGC023 )， 一个字符串(*,<CR=0x0D> PLC )，
		  几个独立字符均可(<ACK=0x06> 或 <NAK=0x15>)。
********************************************************************************************/
void CSyncCom::SetFrameEndStr(const char* pszEnd)
{
	strcpy(m_pszRecEnd, pszEnd);
}

/*******************************************************************************************
函数名称:bool CSyncCom::ReceiveData(DWORD& dwBytesRead, unsigned int iBufSize, char* pcBuf )
输入参数:DWORD& dwBytesRead ,返回 接收到的 字符数
		 unsigned int iBufSize, 接收缓冲区pcBuf的容量
         char* pcBuf, 接收缓冲区. 若为 NULL,则用 类内 接收缓冲区
         因为内部设定缓冲区的字节数为 512, 远大于通常的帧长度, 所以没有作缓冲区溢出异常处理.
返回参数:接收完成状态
         如果设置了帧结尾符号，则接收到之后返回 true。否则超时返回false。
Comment: 帧接收标志
	enCommNone,       //没有特征字符(串). 串口只要接收到字符后,就立即退出.  SleepTime足够长的话,能接收完整的帧.
	enCommOneChar,    //一个特征字符,通常为 <CR> ,即 0x0D
	enCommSubString,  //特征字符串（连续2个字符以上的字符串）,比如 OMRON PLC要求 "*\x0D"
	enCommEitherChar  //两个字母中的任意一个(2个字符中的任一个). //有的仪器，操作成功时返回一个确认字符，否则返回另一个字符表示失败。
说明:    一次性收发。如果在限定 m_dwWaitTime 内仍不完整的话，则会出错。通常需要为此设置足够的时间.
         本类应用的方便之处在于本函数。
		 通常，在SendCommand之后就可立即调用ReceiveData函数。
*******************************************************************************************/
bool CSyncCom::ReceiveData(DWORD& dwBytesRead, unsigned int iBufSize, char* pcBuf )
{
	dwBytesRead = 0;
	if( m_hCom == INVALID_HANDLE_VALUE ) //
	{
		m_dwLastError = _SYNCCOM_ERROR_HANDLE_ ;
		return false;
	}
	
	char* lpszRecBuf = ( (NULL==pcBuf) ? m_pszRecvBuf : pcBuf ); //如果函数调用时没有指定接收buffer,则用内部buffer
	char* lpRec = lpszRecBuf; //接收指针

	*lpRec = '\0'; //(相当于)清空接收缓冲区

	DWORD dwErrorFlags = 0;
	DWORD dwOneRead = 0; //单次接收到的数据个数

	SetCommMask( m_hCom , EV_RXCHAR );
	DWORD dwStart = GetTickCount();
	COMSTAT ComStat ;
	char* ptemp = NULL;
	BOOL bEnd = false;  //结束标志
	while( GetTickCount() - dwStart < m_dwWaitTime )
	{ 
		//线程让出当前CPU时间片, 兼等待响应
		Sleep(m_dwSleepTime); 
		//察看串口有无输入
		ClearCommError(m_hCom, &dwErrorFlags, &ComStat );
		if( 0 != ComStat.cbInQue )
		{				
			if(ReadFile(m_hCom,lpRec,ComStat.cbInQue,&dwOneRead,NULL))
			{
				lpRec += dwOneRead;
				*lpRec = '\0' ; //自行添加字符串结尾符

				/*查找到结尾符号，则退出；*/
				switch(m_enumCommEndType)
				{
				case enCommOneChar: 
					//依次查找接收到的数据，直到查找到 结尾标志符号。 
					//仅查找末尾字符请见enCommOneCharEx,2007.11.17
					//有的下位机(外设)连续发送几帧数据，例如 VGC023，此处接收到1完整帧标志就退出。
					//由RecevieData调用方进行实际帧数判断。
					for( ptemp = lpRec-dwOneRead; *ptemp != '\0'; ptemp++ )
					{
						if( *ptemp == m_pszRecEnd[0] )
						{
							bEnd = true;
							break;
						}
					}
					break;					

				case enCommSubString:		//子字符串
					if( NULL != strstr(lpszRecBuf,m_pszRecEnd))
					{
						bEnd = true;
					}
					break;

				case enCommEitherChar:      //二选一字符，对应 DM-1(不存在连续发送多帧)
					if( *(lpRec-1)==m_pszRecEnd[0] || *(lpRec-1)==m_pszRecEnd[1] )
					{
						bEnd = true;
					}
					break;
					/*
					for(ptemp = lpRec-dwOneRead-1; *ptemp!='\0'; ptemp++)
					{
						if( *ptemp==m_pszRecEnd[0] || *ptemp==m_pszRecEnd[1] )
						{
							bEnd = true;
							break;
						}
					}
					break;
					*/
				case enCommOneCharEx:					
					//单个结尾字符，查最后一个字符是否符合！
					if(*(lpRec-1) ==  m_pszRecEnd[0] )
					{
						bEnd = true;
					}
					break;					
					
				case enCommNone:
				default:
					bEnd = true;
					break;					
				}

				dwBytesRead += dwOneRead;
				//判别结果
				if(bEnd)
				{
					return true;
				}
				else
				{
					continue; //没有找到,则继续 while 循环,直到超时发生!
				}
			}
		}
	}

	if(dwBytesRead==0)
	{
		m_dwLastError = _SYNCCOM_ERROR_RECV_TIMEOUT_ ; 
	}
	else
	{
		m_dwLastError = _SYNCCOM_ERROR_RECV_TIMEOUT_NO_END ; 
	}
	return false;
}


/***********************************************************************************
函数名称：GetApiLastErrorStr
输入参数：DWORD dwApiErrCode，传入的 Windows API 函数的错误代码
          LPTSTR lpszBuf，解释字符串的buffer，调用者给buffer
		  int iBufSize，buffer的大小
输出参数：如正常，同 LPTSTR lpszBuf，返回解释字符串
comment:  获取 API 函数错误的解释，由调用方给解释字符串的缓冲区及其大小
************************************************************************************/
bool CSyncCom::GetApiLastErrorStr(DWORD dwApiErrCode, LPTSTR lpszBuf, int iBufSize)
{
	if(iBufSize==0)
		return false;

	*lpszBuf = _T('\0');

	if(0!=::FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM, 
		NULL, 
		dwApiErrCode, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
		lpszBuf, 
		iBufSize, 
		NULL 
		) )
		return true;	   
	else
	{
		_tcscpy(lpszBuf,_T("CSyncCom::FormatMessage Error: Unknown API Error Code"));
	}
	    return false;	
}