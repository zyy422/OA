#pragma once
#include "..\synccommsource\synccom.h"

class CIBGLCom :
	public CSyncCom
{
public:
	CIBGLCom(void);
public:
	~CIBGLCom(void);
public:
	void InitComm(void);
};
