#include "StdAfx.h"
#include "ZYGraphData.h"

CZYGraphData::CZYGraphData(void)
{
	bCanDelete = true;  //默认可删除
	pxData = NULL;      //x轴数据, 单调增
	pyData = NULL;      //y数据
	iDataNumber = 0;    //数据总数.

	//用于显示时图形整体左右移动.(x方向)
	//所谓左右移动，实质是，给每个 x 加一个的固定偏移量
	//偏移量为正，则显示时曲线整体向右移动，偏移量为负，曲线向左移动
	dxDispShift = 0; //显示数据的 x 起始数据值. 初始化为0
	
	iGraphStyle = _ZYGRAPH_STYLE_LINE; //曲线种类.默认为连线
	
	nPenStyle = PS_SOLID;
	nDotStyle = _ZYGRAPH_STYLE_DISCRETE_DOT;

	crColor   = RGB(255,0,0);

	penLine.CreatePen(nPenStyle,1,crColor);
}

CZYGraphData::~CZYGraphData(void)
{
	if(pxData)
	{
		delete [] pxData;
		pxData = NULL;
	}
	if(pyData)
	{
		delete [] pyData;
		pyData = NULL;
	}
	iDataNumber = 0;

	penLine.DeleteObject();
}

bool CZYGraphData::fn_ChangeColor(COLORREF crNewColor)
{
	if( crNewColor != crColor)
	{
		crColor = crNewColor;
		penLine.DeleteObject();
		penLine.CreatePen(nPenStyle,1,crColor);
	}
	return false;
}
