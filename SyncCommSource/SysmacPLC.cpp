#include "StdAfx.h"
#include "SysmacPLC.h"

CSysmacPLC::CSysmacPLC(void)
: m_nUnitNumber(0)
{
	InitComm();
}

CSysmacPLC::~CSysmacPLC(void)
{

}
/********************************************************************************************
Function name: CreateFCS
Parameters:   const char* pstr, 待生成校验帧的字符串
			  int   iCount,     pstr字符串待校验的长度
			  char* pFCS,       校验结果存储区
返回值：      指向pFCS首地址
comment:      OMRON SYSMAC 系列PLC C mode 通信校验帧生成
调用方式:     可将 pstr 指向待校验字符串的首字符， pFCS指向待校验串的FCS区域(两字节)，而iCount可设定为
              pFCS-pstr，这样，本函数完成后，pstr就是完整的可发送给PLC的字符串。
********************************************************************************************/
char* CSysmacPLC::CreateFCS(const char* pstr, int iCount, char* pFCS)
{
	if(pstr==NULL || pFCS==NULL)
		return NULL;
    char ch = *pstr;

	//所有字符参与异或
	for(int i=1;i<iCount;i++)
	{ 
		ch ^= *(pstr+i);
	}	

	//高4位变成一个字符 (16进制， 0~F)
	char ct = ch >> 4; 
	if(ct>9) 
		ct += 'A'-10;
	else 
		ct += '0';
    *pFCS = ct; 

	//低4位变成一个字符(16进制， 0~F)
    ct = ch & 0x0F;
	if(ct>9) 
		ct += 'A'-10;
	else 
		ct += '0';

    *(pFCS+1) = ct;
	
	return pFCS;
}

//参数的格式请参见函数 CreateFCS
BOOL CSysmacPLC::IsFCSOK(const char* pstr, int iCount, char* pFCS)
{
	char cTmp[2];
	CreateFCS(pstr,iCount,&cTmp[0]);
	if(*pFCS == cTmp[0] && *(pFCS+1)== cTmp[1])
		return true;
	else
		return false;

}
void CSysmacPLC::InitComm(void)
{
	//等待时间，具体用法参见 ReciveData函数
	m_dwWaitTime = 200; //200ms
	m_dwSleepTime = 10; //10ms

	//帧结尾方式
	m_enumCommEndType = enCommSubString;
	//帧结尾符号字符串
	*m_pszRecEnd = '*';
	*(m_pszRecEnd+1) = 0x0D;
	*(m_pszRecEnd+2) = 0;
	
	//以下参数默认可读取 OMRON PLC
	m_strCom = TEXT("COM1");
	m_hCom  = INVALID_HANDLE_VALUE;
	
	m_dcb.DCBlength = sizeof(DCB);
	m_dcb.BaudRate  = 38400; 
	m_dcb.ByteSize  = 7; 
	m_dcb.Parity    = EVENPARITY;
	m_dcb.StopBits  = TWOSTOPBITS; 
	m_dcb.fParity   = true; 
	m_dcb.fBinary   = TRUE;

	m_CommTimeouts.ReadIntervalTimeout = 100; //ms
	m_CommTimeouts.ReadTotalTimeoutMultiplier = 100; 
	m_CommTimeouts.ReadTotalTimeoutConstant = 1000; 
	m_CommTimeouts.WriteTotalTimeoutMultiplier = 100;
	m_CommTimeouts.WriteTotalTimeoutConstant = 1000;
}

BOOL CSysmacPLC::SetUnitNumber(unsigned int nUnitNumber)
{
	if( nUnitNumber <= 99 ) //必须是2位整数
	{
		m_nUnitNumber = nUnitNumber;
		return true;
	}
	else
	{
		return 0;
	}	
}

/***************************************************************************************************
函数名称：CreateFCSChar
          生成一个FCS字符
输入参数：const char* pstr, 待生成字符串
          iCount, 指定参与生成的长度
返回值：  生成的FCS字符
备注：    本函数供临时生成一个中间字符，如果再延长字符串来生成FCS，则可以此中间字符开始，以加快生成完整FCS的速度。
          似乎可以用在生成诸如 RD、WD等命令中，但因为 如此加速的话，效率可能是上去一点，但程序灵活性将大大下降。
          因为完整的生成一个FCS串，所用到的时间实际非常短，所以在其他地方本函数并没有用。
***************************************************************************************************/
char CSysmacPLC::CreateFCSChar(const char* pstr , int iCount)
{
	char chTmp = *pstr;
	for(int i=1;i<iCount;i++)
	{
		chTmp ^= *(pstr+i);
	}
	return chTmp;
}


/******************************************************************************************************
函数名称：FormatCmdBufferRD
格式化读命令缓冲区
输入参数：
返回值：
comment:  用于生成发送给plc的读命令
note:     不支持 RE命令（读取EM）！因为RE读取命令格式比普通的读取命令多了一个Bank No. 还是特殊对待吧。
******************************************************************************************************/
BOOL CSysmacPLC::FormatCmdBufferRD(char* lpszBuffer, enum _enumSysmacRdType enType, int nBeginWord, int nReadNumber)
{
	switch (enType)
	{
	case enReadCIO:
		sprintf(lpszBuffer,"@%02dRR%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	case enReadLR:
		sprintf(lpszBuffer,"@%02dRL%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	case enReadHR:
		sprintf(lpszBuffer,"@%02dRH%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	case enReadTC:
		sprintf(lpszBuffer,"@%02dRC%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	case enReadTCStatus:
		sprintf(lpszBuffer,"@%02dRG%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	case enReadAR:
		//nBeginWord: 0~959
		sprintf(lpszBuffer,"@%02dRJ%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	case enReadDM:		
	default:	
		sprintf(lpszBuffer,"@%02dRD%04d%04dXX*\x0D",m_nUnitNumber,nBeginWord,nReadNumber);
		break;
	}
	
	CreateFCS(lpszBuffer,ciRW_FCS_Pos,lpszBuffer + ciRW_FCS_Pos);  //Zhou yi :将变换后字符串放在未变换的后面，地址往后移13word
	return true;
}

/******************************************************************************************************
函数名称：FormatCmdBufferWR
          格式化写命令缓冲区
输入参数：char* lpszBuffer， 命令缓冲区
          enum _enumSysmacRdType enType, 命令类型
		  int  nBeginWord, 开始字地址
		  int  iWriteNumber, 待写字节数
		  int  nWriteArray[], 待写入的数组
返回值：  
comment:  用于生成发送给plc的写命令
note:     1, 不支持 WE命令（写EM）！因为WE命令格式比普通的读取命令多了一个Bank No. 还是特殊对待吧。
          2, 函数调用方必须指定好 nWriteArray, 本函数不负责进行数值转换 和 nWriteArray 容量检查
		  3, 例如,置位 CIO 3248 的第 n (n:0~15)位,  nWriteArray的值应带入  1<<n
******************************************************************************************************/
BOOL CSysmacPLC::FormatCmdBufferWR(char* lpszBuffer, enum _enumSysmacWrType enType, int nBeginWord, int nWriteNumber,int nWriteArray[])
{
	int nByteWrite = 0;
	switch (enType)
	{
	case enWriteCIO:
		nByteWrite = sprintf(lpszBuffer,"@%02dWR%04d",m_nUnitNumber, nBeginWord);
		break;
	case enWriteLR:
		nByteWrite = sprintf(lpszBuffer,"@%02dWL%04d",m_nUnitNumber, nBeginWord);
		break;
	case enWriteHR:
		nByteWrite = sprintf(lpszBuffer,"@%02dWH%04d",m_nUnitNumber, nBeginWord);
		break;
	case enWriteTC:
		nByteWrite = sprintf(lpszBuffer,"@%02dWC%04d",m_nUnitNumber, nBeginWord);
		break;
	case enWriteAR:
		nByteWrite = sprintf(lpszBuffer,"@%02dWJ%04d",m_nUnitNumber, nBeginWord);
		break;
	case enWriteDM:
		nByteWrite = sprintf(lpszBuffer,"@%02dWD%04d",m_nUnitNumber, nBeginWord);
	default:
		break;
	}

	for( int i = 0; i < nWriteNumber; i++)
	{
		nByteWrite += sprintf( lpszBuffer + nByteWrite, "%04X", *(nWriteArray+i));
	}
	CreateFCS(lpszBuffer,nByteWrite,lpszBuffer + nByteWrite);	
	
	nByteWrite += 2; //避开FCS区
	*(lpszBuffer + nByteWrite)	    = '*';
	*(lpszBuffer + nByteWrite + 1)  = '\x0D';  //<CR>
	*(lpszBuffer + nByteWrite + 2)  = '\0';    //添加字符串结尾符

	return true;
}

/**********************************************************************************
函数名称：DecString2Int
          将指定字符串指定长度转换成 Int数据//类似于 atoi ，简单情况下直接处理.
输入参数：const char* ps, 待转换的源字符串
          int iLength,    待转换的长度
返回值：  转换后的 int 数据.
**********************************************************************************/
int CSysmacPLC::DecString2Int(const char* ps, int iLength)
{
	int sum = 0;
	for(int i=0;i<iLength;i++)
	{
		if(*(ps+i)>='0' && *(ps+i)<='9')
		{
			sum += *(ps+i) - '0' ;
			sum *=10;
		}
	}
	return sum/10;
}

/********************************************************************************************************
函数名称：HexString2Int
          将 16进制 字符串转换成 int 型
输入参数：const char* ps, 源字符串
          iLength, 待转换字符串长度
返回值：  转换后的 int 型数值
********************************************************************************************************/
int CSysmacPLC::HexString2Int(const char* ps, int iLength)
{
	int sum = 0;
	for(int i=0; i<iLength; i++)
	{
		if(*(ps+i)>='0' && *(ps+i)<='9')
		{
			sum += *(ps+i) - '0' ;
		}
		else if( *(ps+i)>='A' && *(ps+i)<='F' )
		{
			sum += *(ps+i) - 'A' + 10;
		}
		else if( *(ps+i)>='a' && *(ps+i)<='f' )
		{
			sum += *(ps+i) - 'a' + 10;
		}
		sum <<= 4; //sum = sum*16;				
	}
	return sum >> 4 ;
}
WORD CSysmacPLC::HexString2Word(const char* ps) //iLength = 4
{
	DWORD sum=0; //如果是 WORD则存在溢出.
	for(int i=0; i<4; i++)
	{
		if(*(ps+i)>='0' && *(ps+i)<='9')
		{
			sum += *(ps+i) - '0' ;
		}
		else if( *(ps+i)>='A' && *(ps+i)<='F' )
		{
			sum += *(ps+i) - 'A' + 10;
		}
		else if( *(ps+i)>='a' && *(ps+i)<='f' )
		{
			sum += *(ps+i) - 'a' + 10;
		}
		sum <<= 4; //sum = sum*16;				
	}
	return (WORD)(sum >> 4) ;

}

/********************************************************************************************************
函数名称：HexString2TCHARstring
          将 16进制字符表示的 字符串转换成 字符串。
		  例如 "4F54" --> TEXT("OT").
		  其中 O 的 ASCII码为 4F
			   T 的 ASCII码为 54
输入参数：const char* ps, 源字符串
          iLength, 待转换字符串长度
输出参数：TCHAR* pD
返回值：  转换后的 int 型数值
说明：    iLength必须是偶数，每2个源字符组成一个ASCII TCHAR.
********************************************************************************************************/
BOOL CSysmacPLC::HexString2TCHARstring(const char* ps, int iLength, TCHAR* pD)
{
	if( 0!=iLength%2 || NULL==ps || NULL==pD )
	{
		return false;
	}

	char* pt = (char*)pD;
	int iTimes = iLength>>1;
	for(int i=0; i< iTimes; i++)
	{
#ifdef _UNICODE
		*pt++ = HexString2Int( ps + i*2 , 2 ); //little endian, 低字节在前，高字节在后.
		*pt++ = 0;
#else
		*pt++ = HexString2Int( ps + i*2 , 2 );
#endif
	}
	return true;
}
/********************************************************************************************************
函数名称：IsEndCodeOK
          接收缓冲区Endcode是否 normal completion
输入参数：接收缓冲区指针
返回值：  判断结果
提示：    End code 是  plc返回帧的状态。当为 "00" 时，为 normal completion.
          ciEndCodePos 是常量，定义见 "SysmacPLC.h" 文件. 指明 End code在 接收缓冲区的起始位置。
********************************************************************************************************/
bool CSysmacPLC::IsEndCodeOK(char* lpszRecvBuf)
{
	if( *(lpszRecvBuf + ciEndCodePos) == '0' &&  *(lpszRecvBuf + ciEndCodePos + 1) == '0' )
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**********************************************************************************************
函数名称：GetEndCodeMeaning
          获取 从plc接收到的帧 里 End code的含义
输入参数：const char* lpszRecvBuf，指向从plc接收的数据帧。
          int iEndCode
返回值：  解释字符串。
note：
          End code 及 解释字符串 请参见 P63 Section 4-2
**********************************************************************************************/
LPCTSTR CSysmacPLC::GetEndCodeMeaning(int iEndCode)//const char* lpszRecvBuf)
{
	//int iEndCode = HexString2Int(lpszRecvBuf + ciEndCodePos, 2);
	switch( iEndCode )
	{
	case 0x00:
		return _T("Normal completion");
	case 0x01:
		return _T("Not executable in RUN mode");
	case 0x02:
		return _T("Not executalbe in MONITOR mode");
	case 0x03:
		return _T("UM write-protected");
	case 0x04:
		return _T("Address over");
	case 0x0B:
		return _T("Not executable in PROGRAM mode");
	case 0x13:
		return _T("FCS error");
	case 0x14:
		return _T("Format error");
	case 0x15:
		return _T("Entry number data error");
	case 0x16:
		return _T("Command not supported");
	case 0x18:
		return _T("Frame length error");
	case 0x19:
		return _T("Not executable");
	case 0x20:
		return _T("Could not create I/O table");
	case 0x21:
		return _T("Not executable due to CPU Unit error");
	case 0x23:
		return _T("User memory protected");
	case 0xA3:
		return _T("Aborted due to FCS error in transmission data");
	case 0xA4:
		return _T("Aborted due to format error in trnasimission data");
	case 0xA5:
		return _T("Aborted due to entry number error in trnasimission data");
	case 0xA8:
		return _T("Aborted due to Frame length error in trnasimission data");
	default:
		return _T("Unknown End code!");
	}
}

/*********************************************************************************************************
函数名： ReadPLC
         读取 PLC 内部某地址内容
输出参数：
         enum _enumSysmacRdType enRdType, 读取区域种类 
		 int  nBeginWord,                 读取的开始地址
		 int  nReadNumber                 欲读取的word个数. 
		 int  nReadArray[]                读取内容存放地址
		 char* pszSendBuf				  发送命令生成缓冲区
		 char* pszRecvBuf				  接收数据缓冲区
返回值： 读取成功与否
备注：   nRdNumWords >= 1
         nReadArray[] 为接收读取数据的数组，必须留有足够的空间。 空间个数 >= nRdNumWords.
		 当 nRdNumWords == 1 , nReadArray可以为某int数。 int&即可！
		 当收发缓冲区指针为NULL时，启用类对象内部缓冲区
流程：   1，根据输入参数生成命令字符串
         2，发送命令至 plc
		 3，接收 plc数据， 并转换至 接收 nReadArray中.
note:    nRdNumberWords 请务必 < 30; //省得多帧处理起来麻烦.
         Read EM 不在此列
*********************************************************************************************************/
bool CSysmacPLC::ReadPLC(enum _enumSysmacRdType enRdType, int nBeginWord, int nReadNumber, 
							WORD wReadArray[]/*int  nReadArray[]*/,char* pszSendBuf, char* pszRecvBuf)
{
	char* lpszBuffer = ( (NULL == pszSendBuf )? m_pszSendBuf: pszSendBuf); //指向发送命令缓冲区
	FormatCmdBufferRD(lpszBuffer,enRdType,nBeginWord,nReadNumber);

	if(	!SendCommand(lpszBuffer,ciRD_Len))
	{//SendCommand内部已经记录错误代码，本处不在分析。
		return false;
	}

	DWORD& dwBytesRead = m_dwBytesRead;
	lpszBuffer       = ( (NULL == pszRecvBuf )? m_pszRecvBuf: pszRecvBuf); //指向接收数据缓冲区
	if(ReceiveData(dwBytesRead, 512, lpszBuffer))
	{
		if(IsEndCodeOK(lpszBuffer))
		{
			if(IsFCSOK( lpszBuffer, dwBytesRead-4, lpszBuffer + dwBytesRead - 4 )) //FCS距离帧结尾有 4个字节(2个FCS字符，以及*\x0D)
			{
				int iBegin = ciEndCodePos + 2;
				if(enReadTC == enRdType)
				{//惟有 Read TC 命令返回的是10进制，其余为16进制.
					for(int i = 0;i<nReadNumber;i++,iBegin += 4)
					{
						//将10进制表示的字符串转换成int数据。 
						wReadArray[i] = DecString2Int(lpszBuffer + iBegin,4); //固定为 4 个字符代表 1 个 words.
					}
				}
				else
				{
					for(int i = 0;i<nReadNumber;i++,iBegin += 4)
					{
						//将16进制表示的字符串转换成 WORD数据。 //RC是10进制，但RC命令暂时不用！
						wReadArray[i] = HexString2Word(lpszBuffer + iBegin); //固定为 4 个字符代表 1 个 word.
					}					
				}
				return true;
			}
			else
			{
				m_dwLastError = _SYSMAC_PLC_ERROR_FCS_;
			}
		}
		else
		{
			m_dwLastError	  = _SYSMAC_PLC_ERROR_END_CODE_;
			m_iLastErrEndCode = HexString2Int( lpszBuffer + ciEndCodePos, 2);
		}
	}
	//ReceiveData内部已经记录错误代码，本处不在分析。
	return false;
}

/*********************************************************************************************************
参见 ReadPLC
DM区的数据，在 OMRON <SYSMAC CS/CJ Series Communications Commands REFERENCE MANUAL> 书中，说的是 16进制，实际
下来却是10进制，故此另列出来。
*********************************************************************************************************/
bool CSysmacPLC::ReadPLCex(enum _enumSysmacRdType enRdType, int nBeginWord, int nReadNumber, WORD wReadArray[], bool bDecimal,char* pszSendBuf , char* pszRecvBuf)
{
	char* lpszBuffer = ( (NULL == pszSendBuf )? m_pszSendBuf: pszSendBuf); //指向发送命令缓冲区
	FormatCmdBufferRD(lpszBuffer,enRdType,nBeginWord,nReadNumber);

	if(	!SendCommand(lpszBuffer,ciRD_Len))
	{//SendCommand内部已经记录错误代码，本处不在分析。
		return false;
	}

	DWORD& dwBytesRead = m_dwBytesRead;
	lpszBuffer       = ( (NULL == pszRecvBuf )? m_pszRecvBuf: pszRecvBuf); //指向接收数据缓冲区
	if(ReceiveData(dwBytesRead, 512, lpszBuffer))
	{
		if(IsEndCodeOK(lpszBuffer))
		{
			if(IsFCSOK( lpszBuffer, dwBytesRead-4, lpszBuffer + dwBytesRead - 4 )) //FCS距离帧结尾有 4个字节(2个FCS字符，以及*\x0D)
			{
				int iBegin = ciEndCodePos + 2;
				if(bDecimal)
				{//指定10进制数据
					for(int i = 0;i<nReadNumber;i++,iBegin += 4)
					{
						//将10进制表示的字符串转换成int数据。 
						wReadArray[i] = DecString2Int(lpszBuffer + iBegin,4); //固定为 4 个字符代表 1 个 words.
					}
				}
				else
				{//若 bDecimal == false，则为 16进制
					for(int i = 0;i<nReadNumber;i++,iBegin += 4)
					{
						wReadArray[i] = HexString2Word(lpszBuffer + iBegin); //固定为 4 个字符代表 1 个 word.
					}					
				}
				return true;
			}
			else
			{
				m_dwLastError = _SYSMAC_PLC_ERROR_FCS_;
			}
		}
		else
		{
			m_dwLastError	  = _SYSMAC_PLC_ERROR_END_CODE_;
			m_iLastErrEndCode = HexString2Int( lpszBuffer + ciEndCodePos, 2);
		}
	}
	//ReceiveData内部已经记录错误代码，本处不在分析。
	return false;
}

/*********************************************************************************************************
函数名称：	GetDMstr
输入参数：	nBeginWord, 起始地址
            nReadNumber，读取的WORD数。注，DM是按WORD存储数据的。一个WORD展开后是2个字符。
返回值：    转化过的字符串，注意此时已经是 兼容 unicode的代码
函数说明：  从DM区域指定起始地址读取指定个数数据，并将其转化成字符串
            因为DM区域的字符串读取出来后是16进制数字字符串形式。 需要将此数字字符串再转化成真正的字符。
			例如 : "4F544643" 分别是 "OTFC"的ASCII码.
			本函数基本是ReadPLC的翻版。
			nReadNumber < 31
			本函数仅应在主线程中使用。
**********************************************************************************************************/
CString CSysmacPLC::GetDMstr(int nBeginWord, int nReadNumber)
{
	ASSERT( nReadNumber < 31 );  //确保只需一帧完成！

	CString strR;
	char* lpszBuffer = m_pszSendBuf; //指向发送命令缓冲区
	FormatCmdBufferRD(lpszBuffer,enReadDM,nBeginWord,nReadNumber);

	if(	!SendCommand(lpszBuffer,ciRD_Len))
	{//SendCommand内部已经记录错误代码，本处不再分析。
		return strR;
	}

	DWORD& dwBytesRead = m_dwBytesRead;
	lpszBuffer       = m_pszRecvBuf; //指向接收数据缓冲区
	if(ReceiveData(dwBytesRead, 512, lpszBuffer))
	{
		if(IsEndCodeOK(lpszBuffer))
		{
			if(IsFCSOK( lpszBuffer, dwBytesRead-4, lpszBuffer + dwBytesRead - 4 )) //FCS距离帧结尾有 4个字节(2个FCS字符，以及*\x0D)
			{
				ASSERT((dwBytesRead-ciEndCodePos-2-4) == (nReadNumber << 2) );

				TCHAR*   pcht = new TCHAR[(nReadNumber<<1)+1]; //因为本函数用的次数少，所以直接用 new and delete.
				pcht[nReadNumber<<1] = _T('\0'); //字符串末尾符号
				//ASSERT
				HexString2TCHARstring(lpszBuffer+ciEndCodePos+2,nReadNumber<<2,pcht);
				strR.Format(TEXT("%s"),pcht);
				delete  [] pcht;
				return strR;
			}
			else
			{
				m_dwLastError = _SYSMAC_PLC_ERROR_FCS_;
			}
		}
		else
		{
			m_dwLastError	  = _SYSMAC_PLC_ERROR_END_CODE_;
			m_iLastErrEndCode = HexString2Int( lpszBuffer + ciEndCodePos, 2);
		}
	}
	//ReceiveData内部已经记录错误代码，本处不分析。
	return strR;	
}

/*********************************************************************************************************
函数名： WritePLC
         写入 PLC 内部某地址内容
输出参数：
         enum _enumSysmacWrType enType, 待写区域种类 
		 int  nBeginWord,                 开始地址
		 int  nWriteNumber                写word个数. 
		 int  nWriteArray[]               内容存放地址
		 char* pszSendBuf				  发送命令生成缓冲区
		 char* pszRecvBuf				  接收数据缓冲区
返回值： 成功与否
备注：   nWriteNumber >= 1
         nWriteArray[] 为接收读取数据
		 的数组，必须留有足够的空间。 空间个数 >= nRdNumWords.
		 当 nWriteNumber == 1 , nWriteArray可以为某int数。 int&即可！
		 当收发缓冲区指针为NULL时，启用类对象内部缓冲区
流程：   1，根据输入参数生成命令字符串
         2，发送命令至 plc
		 3，Write 命令发送后，正常的话是EndCode= 00，即宣告OK.
note:    nWriteNumber 请务必 < 30; //省得多帧处理起来麻烦.
         Write EM 不在此列
*********************************************************************************************************/
bool CSysmacPLC::WritePLC(enum _enumSysmacWrType enType, int nBeginWord, int nWriteNumber, int  nWriteArray[], char* pszSendBuf, char* pszRecvBuf)
{
	char* lpszBuffer = ( (NULL == pszSendBuf )? m_pszSendBuf: pszSendBuf); //指向发送命令缓冲区
	FormatCmdBufferWR(lpszBuffer,enType, nBeginWord, nWriteNumber,nWriteArray);

	if(	!SendCommand(lpszBuffer,strlen(lpszBuffer)))
	{//SendCommand内部已经记录错误代码，本处不再分析。
		return false;
	}

	DWORD& dwBytesRead = m_dwBytesRead;
	lpszBuffer       = ( (NULL == pszRecvBuf )? m_pszRecvBuf: pszRecvBuf); //指向接收数据缓冲区
	if(ReceiveData(dwBytesRead, 512, lpszBuffer))
	{
		if(IsEndCodeOK(lpszBuffer))
		{
			if(IsFCSOK( lpszBuffer, dwBytesRead - 4, lpszBuffer + dwBytesRead - 4 )) //FCS距离帧结尾有 4个字节(2个FCS字符，以及*\x0D)
			{
				return true;
			}
			else
			{
				m_dwLastError = _SYSMAC_PLC_ERROR_FCS_;
			}
		}
		else
		{
			m_dwLastError	  = _SYSMAC_PLC_ERROR_END_CODE_;
			m_iLastErrEndCode = HexString2Int( lpszBuffer + ciEndCodePos, 2);
		}
	}
	//ReceiveData内部已经记录错误代码，本处不再分析。
	return false;
}