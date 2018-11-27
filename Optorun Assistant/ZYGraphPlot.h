// "ZYGraphPlot.h"
#pragma once
#include "atltypes.h"

#include "ZYGraphData.h"//CZYGraphData类头文件

#define _ZY_GRAPH_PLOT_TYPE_LIN 0  //线性绘图
#define _ZY_GRAPH_PLOT_TYPE_LOG 1  //对数性

#define _ZY_GRPAH_PLOT_COLOR_MAX 16 //用于为新数据分配颜色

bool SaveDCBitmap(CDC* pDC,CRect rect,LPCTSTR lpszFileName);

class CZYGraphPlot
{
public:
	CZYGraphPlot(void);
public:
	~CZYGraphPlot(void);

	COLORREF m_clrSet[_ZY_GRPAH_PLOT_COLOR_MAX];  //用于为新数据到来时分配颜色.
	//新数据颜色 = m_clrSet[i%_ZY_GRPAH_PLOT_COLOR_MAX];

	//图形背景颜色及坐标颜色. 在初始化时后中间基本无需更改.
	COLORREF m_clrBk;     //背景颜色
	COLORREF m_clrGrid;   //坐标轴颜色
	COLORREF m_clrYScale; //y轴的字体颜色

	//
	CString m_strTitle;   //标题，在图形中间上部
	double  m_alphaTitle; //标题显示的透明度
	COLORREF m_crTitleBk; //标题图框的背景色
	COLORREF m_crTitle;   //标题字体颜色
	
	double  m_alphaHint;  //图形提示的透明度
	COLORREF m_crHintBk;  //图形提示的背景色.

	//x,y轴划分的网格数. 在初始化后中间基本无需更改.
	int      ixGridNum;
	int      iyGridNum;

	bool     bxGridDisp; //显示 x 网格(竖线条)，初始为显示
	int      ixMinutesPerGrid;  //x网格每格代表的分钟数，初始为 1

	//显示的坐标范围
	double   yTop;
	double   yBot;
	double   xLeft;
	double   xRight;

	//绘图类型, y轴表示类型
	int m_iYDrawType; //默认为对数

	CPoint m_pDiamondOffset[25]; //菱形，由25个小点.
	CPoint m_pRectOffset[36];    //矩形，36个小点.
	CPoint m_pTriangleOffset[28];//三角形，28个小点.
	CPoint m_pXShapeOffset[9];   //X形状, 9个点
	CPoint m_pCrossOffset[9];    //十字架,9个点
	CPoint m_pX1ShapeOffset[13]; //X中间多1竖线，13个点

	CList <CZYGraphData*, CZYGraphData* &> listData;

public:
	bool PlotGraph(CDC* pDestDC,CRect rtDest);
private:
	void fn_InitOffset(void);//初始化画点时的偏移量
	void fn_prePlotGrid(CDC& dc, CRect& rect);
	// 画外框
	void fn_PlotFrame(CDC& dc , CRect& rect);
public:
	// 在给定递增的pSrc序列中查找某个数值 应该对应的位置
	bool fn_findPos(double dData, int& iPos, double* pSrc, int iSrcNum,bool bPrePos);

public:
	// 用当前背景，作线条示例. 
	void fn_PlotIllustration(CDC* pDestDC, CRect& rect, COLORREF crClr, int nGraphStyle, int nDotStyle,int nPenStyle);
public:
	// 用内部第1条数据进行演示绘图
	int fn_PlotIllustrUse1st(CDC* pDestDC, CRect& rect,COLORREF crBk, COLORREF crGrid);
};
