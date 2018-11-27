#pragma once
/******************************************************************
用于显示用数据类。存储一条二维曲线数据及数据显示参数。
*******************************************************************/
#define _ZYGRAPH_STYLE_DISCRETE  0 //画离散点
#define _ZYGRAPH_STYLE_LINE      1 //画线，画线时画笔的风格由 nLinePenStyle定义，默认为PS_SOLID
#define _ZYGRAPH_STYLE_DOT_LINE  2 //点画线

#define _ZYGRAPH_STYLE_DISCRETE_DOT    0   //仅打点
#define _ZYGRAPH_STYLE_DISCRETE_DIMOND 1   //菱形
#define _ZYGRAPH_STYLE_DISCRETE_RECT   2   //正方形
#define _ZYGRAPH_STYLE_DISCRETE_TRIANGLE 3 //三角形
#define _ZYGRAPH_STYLE_DISCRETE_X_SHAPE 4 //X形状  
#define _ZYGRAPH_STYLE_DISCRETE_CROSS    5 //十字形
#define _ZYGRAPH_STYLE_DISCRETE_X_1_SHAPE 6 // X中间多1竖线

class CZYGraphData
{
public:
	CZYGraphData(void);
public:
	virtual ~CZYGraphData(void);

	double* pxData;      //x轴数据, 持续增加
	double* pyData;      //   对应y数据
	int     iDataNumber; //数据总数.

	//用于显示时图形整体左右移动.(x方向)
	//所谓左右移动，实质是，给每个 x 加一个的固定偏移量
	//偏移量为正，则显示时曲线整体向右移动，偏移量为负，曲线向左移动
	double  dxDispShift; //显示数据的 x 起始数据值. 初始化为0
	
	CString strName;     //曲线名称

	int     iGraphStyle; //曲线种类.默认为点
	int     nDotStyle;

	int     nPenStyle;
	CPen    penLine;    //画曲线时的画笔.

	COLORREF crColor;
	bool    bCanDelete; //主要用于当前测试结果数据不能删除。
	                   //默认可删除(true)，而当前测试数据需要额外写成false.
	
public:
	bool fn_ChangeColor(COLORREF crNewColor);
};
