#pragma once
#include "d:\生产技术部开发课\项目-机台诊断软件\开发源程序\optorun assistant\synccommsource\synccom.h"

class Cdm1Com :
	public CSyncCom
{
public:
	Cdm1Com(void);
public:
	~Cdm1Com(void);
public:
	void InitComm(void);
};
