/************************************************************************************
File name: APCGauge.h
comment:   Head file of class CAPCGauge.
Author:    zhang ziye
Last Edit: 2008.05.19
备注：     本类用于对应机台APC用真空规
************************************************************************************/
#pragma once
//以下定义来自OPTORUN PLC I/O式样表	（一下定义了5种真空计以及在实现函数中定义其电压和pascal的换算关系）
#define _TYPE_APC_GAUGE_PSG_AND_PENNING 0	//（默认真空计类型）
#define _TYPE_APC_GAUGE_HPG400	        1
//#define _TYPE_APC_GAUGE_UNKNOWN			2 
#define _TYPE_APC_GAUGE_PEG100			2	//2010.04.14 
#define _TYPE_APC_GAUGE_MPG400			3
#define _TYPE_APC_GAUGE_ULVAC_BMR2_SC1	4	//2010.09.15  这种为新加的电离真空计
#define _TYPE_APC_GAUGE_MAX				5



class CAPCGauge
{
public:
	CAPCGauge(void);
public:
	~CAPCGauge(void);
public:
	int m_nGaugeType;
	int m_nOutputMode; //暂时未用
	
public:
	int GetGaugeType(void)
	{
		return m_nGaugeType;
	}
public:
	double TransVal2Pascal(WORD wVal);
public:
	bool SetGaugeType(int nType);
};
