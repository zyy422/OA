#include "StdAfx.h"
#include "APCGauge.h"
#include <math.h>
#include <cmath>

CAPCGauge::CAPCGauge(void)
{
	m_nGaugeType = 0;
}

CAPCGauge::~CAPCGauge(void)
{
}

double CAPCGauge::TransVal2Pascal(WORD wVal)
{
	double dVolt = (double)wVal *10.0/4000;
	double dPascal;
	int	   E = 0; //2010.09.15，为ulvac BMR2\SC1 电离真空计换算准备
	double DOU = 10;
	switch(m_nGaugeType)
	{
	case _TYPE_APC_GAUGE_MPG400: //3
		dPascal = pow( 10, dVolt * 11 / 10 - 6); // -8 mbar , 1mbar = 100 Pa,所以直接 -6
		break;

	case _TYPE_APC_GAUGE_PEG100://2	//2010.04.14
		dPascal = 100 * pow( 10.0, dVolt/(9.0/7.0) - 9.0 - 7.0/9.0);
		// 10# ^ (sngVolt / (9# / 7#) - 9# - 7# / 9#)
		break;

	case _TYPE_APC_GAUGE_PSG_AND_PENNING: //0
		dPascal = pow(10,(dVolt - 7.75) / 0.75 + 2 );
		break;

	case _TYPE_APC_GAUGE_HPG400: //1
		if(dVolt >= 1.5 && dVolt <= 7.5)
            dPascal = pow(10, (dVolt - 5.5));
		else if (dVolt >= 8.5 && dVolt <= 9.75)
            dPascal = pow(10,(4 * (dVolt - 8.5)));
		else 
			dPascal = 0;
		break;

	case _TYPE_APC_GAUGE_ULVAC_BMR2_SC1://4 ulvac BMR2\SC1 电离真空计
		E	= (int)dVolt;
		if ((dVolt-(double)E)>=0.1)                 //20120223
		dPascal = 10 * (dVolt - (double)E) * pow(10.0,-(8-E));
		else
		dPascal= pow(10.0,-(8-E));
		break;

	/*case _TYPE_APC_GAUGE_PEG_NoDisp:*/
	//	DOU = log(DOU);
	//	dPascal = exp(((dVolt - 12.66) / 1.33) * DOU ) * 100;
	//	break;

	default: //其它类型真空计因暂不知该如何对应，故... 080519
//		dPascal = pow( 10.0, dVolt * 7.0 / 4000.0 - 7.0 );
		DOU = log(DOU);
		dPascal = exp(((dVolt - 12.66) / 1.33) * DOU ) * 100;
	break;

		}
	return dPascal;
}

bool CAPCGauge::SetGaugeType(int nType)
{
	if( nType>=0 && nType< _TYPE_APC_GAUGE_MAX )
	{
		m_nGaugeType = nType;
		return true;
	}
	else
	{
		return false;
	}	
}
