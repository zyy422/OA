/************************************************************************************
File name: IonGauge.h
comment:   Head file of class CIonGauge.
Author:    zhang ziye
Last Edit: 2007.08.22
备注：本类仅提供对 罗列的几种 离子真空规 进行识别和 电压--> Pa 真空度计算
************************************************************************************/
#pragma once

//以下定义来自OPTORUN PLC I/O式样表	（以下定义了5种离子真空计的电压pascal换算方法）
#define _TYPE_ION_GAUGE_NONE   0	//（无离子真空计）
#define _TYPE_ION_GAUGE_GIM2   1
#define _TYPE_ION_GAUGE_M832   2
#define _TYPE_ION_GAUGE_BPG400 3
#define _TYPE_ION_GAUGE_GID7   4
#define _TYPE_ION_GAUGE_BPG402 5 //2008.02.14添加. VGC403？
#define _TYPE_ION_GAUGE_MAX    5

//电离规的模拟输出式样
#define _ION_GAUGE_MODE_LINEAR 0 //默认值
/****************以下两种电离规输出样式都没有使用********************************/
#define _ION_GAUGE_MODE_LOG    1
#define _ION_GAUGE_REC_HOLD    2 //GI-M2 有此设定

class CIonGauge
{
public:
	CIonGauge(void);
public:
	~CIonGauge(void);
public:
	// Ion Gauge Type
	int m_nGaugeType;
	int m_nOutputMode;
	bool m_bIonGaugeExist; //在SetGaugeType调用时设置。
public:
	// Set Gauge Type from PLC DM RE
	bool SetGaugeType(LPCSTR strGaugeType);
public:
	// Get Gauge Type Name
	void GetGaugeTypeName(CString& strTypeName);
	CString GetGaugeTypeName(void);
public:

	int GetGaugeType(void)
	{
		return m_nGaugeType;
	}
public:
	// Translate vacuum pressure from volt to pascal
	double TransStr2Pascal(LPCSTR strVolt);

	//Translate plc DM buffer value TO pascal
	double TransVal2Pascal(WORD wVal);
public:
	bool SetGaugeType(int nType);
public:

	bool HasIonGauge(void)
	{
		return m_bIonGaugeExist;
		//return ( m_nGaugeType != _TYPE_ION_GAUGE_NONE );
	}
public:
	int SetOutputMode(int nNewOutputMode);
public:
	bool SetGaugeTypeEx(int nType, bool bExsit);
};
