#pragma once

/******************************************************************
读取 plc 的泛用线程 基础类.

*******************************************************************/
// CPlcRdWrThread
class CPlcRdWrThread : public CWinThread		//ZHOU YI 创建了一个UI线程
{
	DECLARE_DYNCREATE(CPlcRdWrThread)

protected:
	CPlcRdWrThread();           // protected constructor used by dynamic creation
	virtual ~CPlcRdWrThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
protected:
	afx_msg void On_Plc_RW_Command(WPARAM wParam, LPARAM lParam); 
	DECLARE_MESSAGE_MAP()

public:
	DWORD m_dwLastError;
public:
	//写plc CIO 某一位 为1， 对应触摸屏动作
	bool SetPLCCIOBitEvent(WORD wAddress, WORD wBit);
	//写plc CIO
	bool WritePLCCIOEvent(WORD wAddress, WORD wValue);	
	bool ReadPLCCIOEvent(WORD wAddress,WORD &wReadValue);
	bool ReadPLCDMEvent(WORD wAddress,WORD &wReadValue,bool bDec = true);
public:
	// 写DM区
	bool WritePLCDMEvent(WORD wAddress, WORD wWriteValue);
};


