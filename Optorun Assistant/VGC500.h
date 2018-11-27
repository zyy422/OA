#pragma once
#include "..\synccommsource\synccom.h"

//VCC500Ö®Îó
class CVGC500 :
	public CSyncCom
{
public:
	CVGC500(void);
public:
	~CVGC500(void);
public:
	void InitComm(void);
};
