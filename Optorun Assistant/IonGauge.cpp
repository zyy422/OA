/************************************************************
File name: IonGauge.cpp
comment:   Ion Gauge class cpp file.
Last Edit: 2007.08.22, zhangziye
*************************************************************/
#include "StdAfx.h"
#include "IonGauge.h"
#include <math.h>

//Ionization Gauge(Ion Gauge)所能表示的上下范围(4199,500 参考plc内部)
#define _MAX_ION_GAUGE_VALUE  (4199-1)
#define _MIN_ION_GAUGE_VALUE  (500)

//本类内在SetupType中修改
double  gc_dMaxIonGaugePa = 10000;
double  gc_dMinIonGaugePa = 0;

CIonGauge::CIonGauge(void)
: m_nGaugeType(0)
{
	m_nOutputMode = _ION_GAUGE_MODE_LINEAR;
	m_bIonGaugeExist = false;
}

CIonGauge::~CIonGauge(void)
{
}

void CIonGauge::GetGaugeTypeName(CString& strTypeName)
{
	int nIonGaugeType = m_nGaugeType;
	if( m_bIonGaugeExist == false)
	{
		nIonGaugeType = _TYPE_ION_GAUGE_NONE;
	}

	switch (nIonGaugeType)
	{
	case _TYPE_ION_GAUGE_GIM2:
		strTypeName = TEXT("GI-M2");
		break;
	case _TYPE_ION_GAUGE_M832:
		strTypeName = TEXT("M832");
		break;
	case _TYPE_ION_GAUGE_BPG400:
		strTypeName = TEXT("BPG400");
		break;
	case _TYPE_ION_GAUGE_GID7:
		strTypeName = TEXT("GID7");
		break;
	case _TYPE_ION_GAUGE_BPG402:
		strTypeName = TEXT("BPG402");
		break;
	case _TYPE_ION_GAUGE_NONE:
	default:
		strTypeName = TEXT(" "); //no additonal ion gauge
		break;
	}
}

CString CIonGauge::GetGaugeTypeName(void)
{
	CString strTypeName;
	GetGaugeTypeName(strTypeName);
	return strTypeName;
}

//Translate vacuum pressure from PLC DM read buffer to pascal( 4 char )
//buffer 0000~4000
double CIonGauge::TransStr2Pascal(LPCSTR strVolt)
{
	return TransVal2Pascal(atoi(strVolt));
}

double CIonGauge::TransVal2Pascal(WORD wVal)
{
	if( 0 == wVal)
	{
		return 0;
	}
	int nVolt = wVal;

	double dPa,dTmp;
	int nExp;
	//2007.11.07，增加种类设定
	switch (m_nGaugeType)
	{
	case _TYPE_ION_GAUGE_GIM2: //设置方法没有调试.2007.11.09, 默认线性 _MODE_LINEAR
	    if(_ION_GAUGE_MODE_LINEAR == m_nOutputMode)
		{
			dTmp  = ((double)nVolt) * 2.5;
			nVolt = (int)dTmp;
			nExp  = (int)(nVolt/1000);
			dPa   = (((double)(nVolt%1000))/100.0)  *  pow(10.0,-(8-nExp));
		}
		else //(_ION_GAUGE_MODE_LOG == m_nOutPutMode)
		{
			dPa   = pow(10.0,(nVolt*2.5/1000-8.0));
		}
		//else
		//{
		//}
		//dPa = 0;
		break;

	case _TYPE_ION_GAUGE_M832: //Recoder输出类型，增加选择
	    if(_ION_GAUGE_MODE_LINEAR == m_nOutputMode)
		{
			dTmp  = ((double)nVolt) * 2.5;
			nVolt = (int)dTmp;
			nExp  = (int)(nVolt/1000);
			dPa   = (((double)(nVolt%1000))/100.0)  *  pow(10.0,-(8-nExp));
		}
		else //if(m_nOutputMode) //_ION_GAUGE_MODE_LOG
		{
			dPa   = pow(10.0,(nVolt*2.5/1000-8.0));
		}
		break;

	case _TYPE_ION_GAUGE_BPG402: //2008.02.14
		dTmp = (double)(nVolt * 10); 
		dPa = pow(10.0 , (dTmp - 7.75) / 0.75 + 2);
		break;

	case _TYPE_ION_GAUGE_BPG400:
		dTmp  = ((double)( nVolt * 10 )) / 4000.0;   //201606
		dPa   =  pow(10.0, (dTmp-7.75)/0.75 + 2);
		break;
	case _TYPE_ION_GAUGE_GID7:
		//dPa   = pow(10.0,(nVolt*2.5/1000-7)); //2007.10.17
		
        dTmp  = ((double)nVolt) * 2.5; 
		nVolt = (int)dTmp;
		nExp  = (int)(nVolt/1000);
		dPa   = (((double)(nVolt%1000))/100.0)  *  pow(10.0,-(7-nExp));		
		
		break;
	case _TYPE_ION_GAUGE_NONE:
	default:
		dPa = 0; //no additonal ion gauge
		break;
	}
	return dPa;
}


bool CIonGauge::SetGaugeType(int nType)
{
	if( nType> 0 && nType<=_TYPE_ION_GAUGE_MAX )
	{
		m_nGaugeType = nType;
		gc_dMaxIonGaugePa = TransVal2Pascal(_MAX_ION_GAUGE_VALUE);
		gc_dMinIonGaugePa = TransVal2Pascal(_MIN_ION_GAUGE_VALUE);
		return true;
	}
	else
	{
		m_nGaugeType = 0;
		return false;
	}
}

// Set Gauge Type from PLC DM READ Data strGaugeType( 4 char )
bool CIonGauge::SetGaugeType(LPCSTR strGaugeType)
{
	return SetGaugeType(atoi(strGaugeType));
}

bool CIonGauge::SetGaugeTypeEx(int nType, bool bExsit)
{
	m_bIonGaugeExist = bExsit;
	if( nType> 0 && nType<=_TYPE_ION_GAUGE_MAX )
	{
		m_nGaugeType = nType;
		gc_dMaxIonGaugePa = TransVal2Pascal(_MAX_ION_GAUGE_VALUE);
		gc_dMinIonGaugePa = TransVal2Pascal(_MIN_ION_GAUGE_VALUE);
		return true;
	}
	else
	{
		m_nGaugeType = 0;
		m_bIonGaugeExist = false;
		return false;
	}
}

//设定IonGauge的模式
//输入参数：待设定的新模式值
//返回参数：设定之前的模式值
//若设定失败，则返回值为 -1.
int CIonGauge::SetOutputMode(int nNewOutputMode)
{
	int nOldOutputMode = -1;
	bool bChange = false;

	if(nNewOutputMode>=0)
	{
		switch(m_nGaugeType)
		{
		case _TYPE_ION_GAUGE_GIM2:
			if(nNewOutputMode<=2)
			{
				bChange = true;
			}
			break;
		case _TYPE_ION_GAUGE_M832:
			if(nNewOutputMode<=1)
			{
				bChange = true;
			}
			break;
		case _TYPE_ION_GAUGE_BPG400:
			break;
		case _TYPE_ION_GAUGE_GID7:
			break;
		}
	}

	if(bChange)
	{	 
		nOldOutputMode = m_nOutputMode;
		m_nOutputMode  = nNewOutputMode;
	}
	
	return nOldOutputMode;
}


