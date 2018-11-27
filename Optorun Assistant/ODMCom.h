#pragma once
#include "..\synccommsource\synccom.h"

class CODMCom :
	public CSyncCom
{
public:
	CODMCom(void);
public:
	~CODMCom(void);
public:
	void InitComm(void);
};
