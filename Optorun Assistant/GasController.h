#pragma once
#include "..\synccommsource\synccom.h"

class CGasController :
	public CSyncCom
{
public:
	CGasController(void);
public:
	~CGasController(void);
public:
	void InitComm(void);
};
