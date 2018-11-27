/*******************************************************************************
文件名："SysmacPLC.h"
描述：   类 CSysmacPLC 的头文件。
         CSysmacPLC，是用于 同 OMRON  SYSMAC CS/CJ系列 PLC 进行串口通信的类。
		 基类 CSynCom 是 同步串口通信类，负责通信。
		 本类主要进行通信协议的解读。
		 协议请参见 OMRON Cat.No.W342-E1-08 
		 <SYSMAC CS/CJ Series Communications Commands REFERENCE MANUAL>
Author:  Zhang ziye,2007.09.05
********************************************************************************/

#pragma once
#include "synccom.h"

#define  _SYSMAC_PLC_ERROR_END_CODE_    ( _SYNCCOM_DERIVE_ERROR_ + 1 )
#define  _SYSMAC_PLC_ERROR_FCS_		    ( _SYSMAC_PLC_ERROR_END_CODE_ +1 )

/******************************************************************************
欧姆龙PLC内存区说明：	
AR:特殊辅助继电器区														 (未使用)
HR:保持继电器区,电源断开时能记住ON/OFF的状态							 (未使用）
IO/CIO:CIO区，用作控制I/O点的数据，也可以用作内部处理和存储数据的工作位
LR:链接继电器区，可以作为内部辅助继电器使用								 (未使用)
DM:动态数据缓冲区
TC:特殊继电器，分配有特殊功能	
TCStatus:																 (未使用)
*******************************************************************************/
enum _enumSysmacRdType
{
	enReadCIO,enReadLR,enReadHR,enReadTC,enReadTCStatus,enReadDM,enReadAR
};

enum _enumSysmacWrType
{
	enWriteCIO,enWriteLR,enWriteHR,enWriteTC,enWriteDM,enWriteAR
};


const int ciRW_FCS_Pos = 13; //FCS在读写PLC的命令字符串中的起始位置.( 第 1 个位置为 0 )
const int ciEndCodePos = 5;  //PLC返回串中 End code的起始位置( 第一个位置为 0.).
                             //End code 为 2 字节，是plc对命令的响应状态代码。
const int ciRD_Len = 17;     //通常的读命令长度

class CSysmacPLC :	public CSyncCom
{
public:
	CSysmacPLC(void);
public:
	~CSysmacPLC(void);
public:
	static char* CreateFCS(const char* pstr, int iCount, char* pFCS);
public:
	static BOOL IsFCSOK(const char* pstr, int iCount, char* pFCS);
public:
	virtual void InitComm(void);

	int    m_iLastErrEndCode;  //最近一次通信检查出错时的EndCode.
private:
	// PLC Unit numer
	unsigned int m_nUnitNumber;

public:
	BOOL   SetUnitNumber(unsigned int nUnitNumber);
public:
	//例如 "@00RD" 的字符串长度 为5
	char   CreateFCSChar(const char* pstr , int iCount=5);

public:
	BOOL FormatCmdBufferRD(char* lpszBuffer, enum _enumSysmacRdType enType, int nBeginWord, int nReadNumber);
	BOOL FormatCmdBufferWR(char* lpszBuffer, enum _enumSysmacWrType enType, int nBeginWord, int nWriteNumber,int nWriteArray[]);
	//BOOL FormatCmdBufferWR(char* lpszBuffer, enum _enumSysmacRdType enType, int nBeginWord, int iWriteNumber,int nWriteArray[]);
public:
	static int  HexString2Int(const char* ps, int iLength);
	static WORD HexString2Word(const char* ps); //iLength = 4
public:
	static int  DecString2Int(const char* ps, int iLength);
public:
	bool IsEndCodeOK(char* lpszRecvBuf);
public:
	LPCTSTR GetEndCodeMeaning(int iEndCode);//const char* lpszRecvBuf);
public:
	bool ReadPLC(enum _enumSysmacRdType enRdType, int nBeginWord, int nReadNumber, WORD wReadArray[], char* pszSendBuf = NULL, char* pszRecvBuf = NULL);
	bool WritePLC(enum _enumSysmacWrType enType, int nBeginWord, int nWriteNumber, int  nWriteArray[], char* pszSendBuf = NULL, char* pszRecvBuf = NULL);

	bool ReadPLCex(enum _enumSysmacRdType enRdType, int nBeginWord, int nReadNumber, WORD wReadArray[], bool bDecimal = true,char* pszSendBuf = NULL, char* pszRecvBuf = NULL);
	CString GetDMstr(int nBeginWord, int nReadNumber);
public:
	BOOL HexString2TCHARstring(const char* ps, int iLength, TCHAR* pD);
};
