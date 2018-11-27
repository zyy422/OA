#include "StdAfx.h"
#include "ZYGraphPlot.h"
#include <math.h>

CZYGraphPlot::CZYGraphPlot(void)
: m_iYDrawType(_ZY_GRAPH_PLOT_TYPE_LOG)
{
	m_clrBk   = RGB(0,0,0);   //图形背景颜色，黑色
	m_clrGrid = RGB(0,128,64);//(51,128,128);//(51,153,51); //主线条颜色 51,128,128
	m_clrYScale = RGB(64,196,64);

	ixGridNum = 30;
	iyGridNum = 11;  // 对应1.0E+5 ~ 1.0E-6
	bxGridDisp = true; //显示 x 网格(竖线条)，初始为显示
	ixMinutesPerGrid = 1;  //x网格每格代表的分钟数，初始为 1

	//显示的坐标范围
	yTop = 1.0E+5;
	yBot = 1.0E-6;
	xLeft = 0;
	xRight = (double) ixGridNum*60; //60秒一格

	//m_clrSet[_ZY_GRPAH_PLOT_MAX_CURVES];
	m_clrSet[0] = RGB(255,0,0); //rgb
	m_clrSet[1] = RGB(0,0,255); //RGB(255,192
	m_clrSet[2] = RGB(0,255,0); 
	m_clrSet[3] = RGB(0,255,255); 
	m_clrSet[4] = RGB(255,0,255);  //rgb
	m_clrSet[5] = RGB(255,255,0); 
	m_clrSet[6] = RGB(192,0,192); 
	m_clrSet[7] = RGB(0,128,128); 
	m_clrSet[8] = RGB(128,128,0); 
	m_clrSet[9] = RGB(128,0,128); 
	m_clrSet[10] = RGB(0,0,192); 
	m_clrSet[11] = RGB(0,192,192); 
	m_clrSet[12] = RGB(192,0,192); 
	m_clrSet[13] = RGB(0,192,128); 
	m_clrSet[14] = RGB(192,0,0); 
	m_clrSet[15] = RGB(0,192,0); 
	/*
	for(int i=16;i<_ZY_GRPAH_PLOT_COLOR_MAX;i++)
	{
		m_clrSet[i] = m_clrSet[i%16];
	}
	*/
	fn_InitOffset();
	m_alphaTitle = 0.6; //标题显示的透明度
	m_crTitleBk = RGB(32,64,32); //标题图框的背景色
	m_crTitle = RGB(128,255,128);   //标题字体颜色

	this->m_alphaHint = 0.7;
	this->m_crHintBk = RGB(32,64,32);
	

}
void CZYGraphPlot::fn_InitOffset(void)
{
	/*菱形,25个点. 以中心点为原点 (0,0)，25点内储存的是相对位置。显示时，
					 +
					+++
				   +++++
				  +++0+++
				   +++++
					+++
					 +
	*/
	//for(int i=0;i<sizeof(m_pDiamondOffset)/sizeof(CPoint);
	//第1行 1
	m_pDiamondOffset[0].x = 0;
	m_pDiamondOffset[0].y = -3;
	//第2行, 3 y=-2, x=[-1,1];
	m_pDiamondOffset[1].x = -1;
	m_pDiamondOffset[1].y = -2;
	m_pDiamondOffset[2].x = 0;
	m_pDiamondOffset[2].y = -2;
	m_pDiamondOffset[3].x = 1;
	m_pDiamondOffset[3].y = -2;
    //第3行, 5 y=-1, x=[-2,2];
	m_pDiamondOffset[4].x = -2;
	m_pDiamondOffset[4].y = -1;
	m_pDiamondOffset[5].x = -1;
	m_pDiamondOffset[5].y = -1;
	m_pDiamondOffset[6].x = 0;
	m_pDiamondOffset[6].y = -1;
	m_pDiamondOffset[7].x = 1;
	m_pDiamondOffset[7].y = -1;
	m_pDiamondOffset[8].x = 2;
	m_pDiamondOffset[8].y = -1;
    //第4行, 7 y=0,  x=[-3,3]
	m_pDiamondOffset[9].x = -3;
	m_pDiamondOffset[9].y = 0;
	m_pDiamondOffset[10].x = -2;
	m_pDiamondOffset[10].y = 0;
	m_pDiamondOffset[11].x = -1;
	m_pDiamondOffset[11].y = 0;
	m_pDiamondOffset[12].x = 0;
	m_pDiamondOffset[12].y = 0;
	m_pDiamondOffset[13].x = 1;
	m_pDiamondOffset[13].y = 0;
	m_pDiamondOffset[14].x = 2;
	m_pDiamondOffset[14].y = 0;
	m_pDiamondOffset[15].x = 3;
	m_pDiamondOffset[15].y = 0;
	//第5行, 5 y=1,  x=[-2,2]
	m_pDiamondOffset[16].x = -2;
	m_pDiamondOffset[16].y = 1;
	m_pDiamondOffset[17].x = -1;
	m_pDiamondOffset[17].y = 1;
	m_pDiamondOffset[18].x = 0;
	m_pDiamondOffset[18].y = 1;
	m_pDiamondOffset[19].x = 1;
	m_pDiamondOffset[19].y = 1;
	m_pDiamondOffset[20].x = 2;
	m_pDiamondOffset[20].y = 1;
	//第6行, 3 y=2, x=[-1,1]
	m_pDiamondOffset[21].x = -1;
	m_pDiamondOffset[21].y = 2;
	m_pDiamondOffset[22].x = 0;
	m_pDiamondOffset[22].y = 2;
	m_pDiamondOffset[23].x = 1;
	m_pDiamondOffset[23].y = 2;
	//第7行, 1 y=3, x=0
	m_pDiamondOffset[24].x = 0;
	m_pDiamondOffset[24].y = 3;

	//三角形, 1,2...7.共28个点
	/*
	+
   ++
   +++
  ++0+
  +++++
 ++++++
 +++++++
	*/
	//第1行,y=-3
	m_pTriangleOffset[0].x =  0; 
	m_pTriangleOffset[0].y = -3;
	//第2行,y=-2
	m_pTriangleOffset[1].x = -1; 
	m_pTriangleOffset[1].y = -2;
	m_pTriangleOffset[2].x =  0;
	m_pTriangleOffset[2].y = -2;
	//第3行,y=-1
	m_pTriangleOffset[3].x = -1;
	m_pTriangleOffset[3].y = -1;
	m_pTriangleOffset[4].x =  0;
	m_pTriangleOffset[4].y = -1;
	m_pTriangleOffset[5].x =  1;
	m_pTriangleOffset[5].y = -1;
    //第4行,y=0
	m_pTriangleOffset[6].x = -2;
	m_pTriangleOffset[6].y =  0;
	m_pTriangleOffset[7].x = -1;
	m_pTriangleOffset[7].y =  0;
	m_pTriangleOffset[8].x =  0;
	m_pTriangleOffset[8].y =  0;
	m_pTriangleOffset[9].x =  1;
	m_pTriangleOffset[9].y =  0;
	//第5行,y=1
	m_pTriangleOffset[10].x =-2; 
	m_pTriangleOffset[10].y = 1;
	m_pTriangleOffset[11].x =-1; 
	m_pTriangleOffset[11].y = 1;
	m_pTriangleOffset[12].x = 0;
	m_pTriangleOffset[12].y = 1;
	m_pTriangleOffset[13].x = 1;
	m_pTriangleOffset[13].y = 1;
	m_pTriangleOffset[14].x = 2;
	m_pTriangleOffset[14].y = 1;
	//第6行,y=2. x=[-3,2]
	m_pTriangleOffset[15].x =-3;
	m_pTriangleOffset[15].y = 2;
	m_pTriangleOffset[16].x =-2; 
	m_pTriangleOffset[16].y = 2;
	m_pTriangleOffset[17].x =-1; 
	m_pTriangleOffset[17].y = 2;
	m_pTriangleOffset[18].x = 0;
	m_pTriangleOffset[18].y = 2;
	m_pTriangleOffset[19].x = 1;
	m_pTriangleOffset[19].y = 2;
	m_pTriangleOffset[20].x = 2;
	m_pTriangleOffset[20].y = 2;
	//第7行,y=3.x=[-3,3]
	m_pTriangleOffset[21].x =-3;
	m_pTriangleOffset[21].y = 3;
	m_pTriangleOffset[22].x =-2; 
	m_pTriangleOffset[22].y = 3;
	m_pTriangleOffset[23].x =-1; 
	m_pTriangleOffset[23].y = 3;
	m_pTriangleOffset[24].x = 0; 
	m_pTriangleOffset[24].y = 3;
	m_pTriangleOffset[25].x = 1;
	m_pTriangleOffset[25].y = 3;
	m_pTriangleOffset[26].x = 2;
	m_pTriangleOffset[26].y = 3;
	m_pTriangleOffset[27].x = 3;
	m_pTriangleOffset[27].y = 3;

//矩形
/*****
x [-2,3]; y[-3,2]
++++++ 
++++++
++++++
++0+++
++++++
++++++
******/
	int n=0;
	for(int i=-2;i<=3;i++)
	{
		for(int j=-3;j<=2;j++)
		{
			m_pRectOffset[n].x = i;
			m_pRectOffset[n++].y = j;
		}
	}

	// X 形状 9点
/*******
+   +
 + +
  0
 + +
+   +
********/
	m_pXShapeOffset[0].x = -2;
	m_pXShapeOffset[0].y = -2;
	m_pXShapeOffset[1].x =  2;
	m_pXShapeOffset[1].y = -2;
	m_pXShapeOffset[2].x = -1;
	m_pXShapeOffset[2].y = -1;
	m_pXShapeOffset[3].x = 1;
	m_pXShapeOffset[3].y = -1;
	m_pXShapeOffset[4].x = 0;
	m_pXShapeOffset[4].y = 0;
	m_pXShapeOffset[5].x = -1;
	m_pXShapeOffset[5].y = 1;
	m_pXShapeOffset[6].x = 1;
	m_pXShapeOffset[6].y = 1;
	m_pXShapeOffset[7].x = -2;
	m_pXShapeOffset[7].y = 2;
	m_pXShapeOffset[8].x = 2;
	m_pXShapeOffset[8].y = 2;

	//十字形状 9点
/***************
  +
  +
+++++
  +
  +
***************/
	m_pCrossOffset[0].x = 0;
	m_pCrossOffset[0].y = -2;
	m_pCrossOffset[1].x = 0;
	m_pCrossOffset[1].y = -1;
	m_pCrossOffset[2].x = -2;
	m_pCrossOffset[2].y = 0;
	m_pCrossOffset[3].x = -1;
	m_pCrossOffset[3].y = 0;
	m_pCrossOffset[4].x = 0;
	m_pCrossOffset[4].y = 0;
	m_pCrossOffset[5].x = 1;
	m_pCrossOffset[5].y = 0;
	m_pCrossOffset[6].x = 2;
	m_pCrossOffset[6].y = 0;
	m_pCrossOffset[7].x = 0;
	m_pCrossOffset[7].y = 1;
	m_pCrossOffset[8].x = 0;
	m_pCrossOffset[8].y = 2;

	//X中间多1竖线, 13点
/*****************
+ + +
 +++
  0
 +++
+ + +
*****************/
	//第1行
	m_pX1ShapeOffset[0].x = -2;
	m_pX1ShapeOffset[0].y = -2;
	m_pX1ShapeOffset[1].x =  0;
	m_pX1ShapeOffset[1].y = -2;
	m_pX1ShapeOffset[2].x =  2;
	m_pX1ShapeOffset[2].y = -2;
	//第2行
	m_pX1ShapeOffset[3].x = -1;
	m_pX1ShapeOffset[3].y = -1;
	m_pX1ShapeOffset[4].x =  0;
	m_pX1ShapeOffset[4].y = -1;
	m_pX1ShapeOffset[5].x =  1;
	m_pX1ShapeOffset[5].y = -1;
	//第3行	
	m_pX1ShapeOffset[6].x =  0;
	m_pX1ShapeOffset[6].y =  0;
	//第4行
	m_pX1ShapeOffset[7].x = -1;
	m_pX1ShapeOffset[7].y =  1;
	m_pX1ShapeOffset[8].x =  0;
	m_pX1ShapeOffset[8].y =  1;
	m_pX1ShapeOffset[9].x =  1;
	m_pX1ShapeOffset[9].y =  1;
	//第5行	
	m_pX1ShapeOffset[10].x =-2;
	m_pX1ShapeOffset[10].y = 2;
	m_pX1ShapeOffset[11].x = 0;
	m_pX1ShapeOffset[11].y = 2;
	m_pX1ShapeOffset[12].x = 2;
	m_pX1ShapeOffset[12].y = 2;
}

CZYGraphPlot::~CZYGraphPlot(void)
{
}


/*********************************************************************
函数名称：PlotLogGraph
输入参数：CDC* pDestDC, 目的DC
          CRect rtDest, 目的区域
		  double yTop,  图形顶端值
		  double yBot,  图形底端值
		  int    iyGridNum, 纵坐标Grid 数
		  int    ixGridNum, 横坐标Grid数(等分数)
		  int    iDispWidth, 图形总显示宽度
		  int    iNumber, 待显示曲线条数
		  double** pData, 待显示数据源的指针数组，每个为 double*
		  int*   ipDataDispLeft, 每条曲线在图形中的左端显示位置. 可正可负！
		  int*   ipDataLeft,     每条曲线左端在数据内部的存储位置.
		  int*   ipDataDispLen,  每条曲线显示的数据长度.
		  LPCTSTR* lpDataName, 每条显示数据源的名称． 
图形横坐标不设定单位，优点是省略一半的存储空间，符合单一采样数据特点. 
          
输出参数：
描述：　  在指定DC指定区域画曲线图．图形纵坐标为Log形式
          虽然可以画多条曲线，但限定曲线图整体只有一套纵坐标．	 　　
		  图形可以左右移动.
**********************************************************************/
bool CZYGraphPlot::PlotGraph(CDC* pDestDC, CRect rtDest)
{
	CDC memDC;
	CBitmap srcBitmap, *oldBitmap;

	CRect mrect = rtDest; //draw rect in memdc
	mrect.OffsetRect(-rtDest.left,-rtDest.top); // zero based in memdc

	//create memDC and inside bitmap
	memDC.CreateCompatibleDC(pDestDC);
	srcBitmap.CreateCompatibleBitmap(pDestDC,mrect.Width(),mrect.Height()); 
	oldBitmap = memDC.SelectObject(&srcBitmap);


	memDC.SetBkColor(m_clrBk);
	this->fn_prePlotGrid(memDC,mrect);
	
	//画数据
	double perX = (double) (mrect.Width()-4)/(xRight-xLeft);
	double perY;

	double yTempTop;
	if( m_iYDrawType == _ZY_GRAPH_PLOT_TYPE_LOG )
	{
		perY = (mrect.Height()-4)/(log10(yTop)-log10(yBot));
		yTempTop = log10(yTop);
		//k0 = ( double ) ( 2-(mrect.Height()-3)) / ( log10(yTop)-log10(yBot) );
		//k1 = (double) ( 2 - k0 * log10(yTop));
	}
	else
	{
		perY = (mrect.Height()-4)/( yTop - yBot );
		yTempTop = yTop;
		//k0 = (double) ( 2-(mrect.Height()-3)) / ( yTop-yBot );
		//k1 = (double) ( 2 - k0 * yTop);
	}
	
	CZYGraphData* p;
	CPen*   oldPen;
	POSITION pos = listData.GetHeadPosition();
	int i, j; //检索用变量.  
	int x, y; //x,y坐标位置变量
//	double dt; //临时double变量
	int iXStart,iXEnd;//作图有效的数据起始与终止位置
	bool b;
	int iCurveCount = 0; //记录实际作图的曲线数目
	int n;
	for (i=0;i < listData.GetCount();i++)
	{
		p = listData.GetNext(pos);

		if( 0 == p->iDataNumber || NULL == p->pyData )
		{ //如果没有数据，则跳过
			continue;
		}
		iCurveCount++;
		
		//获得合适的数据起点与终点 X 位置，裁减掉 落在 显示界面 外的数据，以减少耗时。
		b = fn_findPos(__max(xLeft-p->dxDispShift,p->pxData[0]),iXStart,p->pxData,p->iDataNumber,true);
		b &= fn_findPos(__min(xRight-p->dxDispShift,p->pxData[p->iDataNumber-1]),iXEnd,p->pxData,p->iDataNumber,false);
		if(!b)
		{//如果没有找到合适的起点或终点，则跳过
			continue;
		}

		if( _ZYGRAPH_STYLE_DISCRETE == p->iGraphStyle )
		{
			if(_ZYGRAPH_STYLE_DISCRETE_DOT == p->nDotStyle )
			{//点
				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop); 
					memDC.SetPixelV(x,y,p->crColor);
				}
			}
			else if(_ZYGRAPH_STYLE_DISCRETE_DIMOND == p->nDotStyle )
			{

				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					//菱形,每块菱形由25个小点,一点一点傻画
					for(n = 0;n<sizeof(m_pDiamondOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pDiamondOffset[n].x,y+m_pDiamondOffset[n].y,p->crColor);
					}					
				}
				//memDC.SelectObject(oldbr);
				//br.DeleteObject();
			}
			else if(_ZYGRAPH_STYLE_DISCRETE_RECT == p->nDotStyle )
			{//正方形

				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					//三角形,28点,一点一点傻画
					for(n = 0;n<sizeof(m_pRectOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pRectOffset[n].x,y+m_pRectOffset[n].y,p->crColor);
					}	
				}
			}
			else if(_ZYGRAPH_STYLE_DISCRETE_TRIANGLE == p->nDotStyle )
			{//三角形

				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					//三角形,28点,一点一点傻画
					for(n = 0;n<sizeof(m_pTriangleOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pTriangleOffset[n].x,y+m_pTriangleOffset[n].y,p->crColor);
					}	
				}
			}
			else if(_ZYGRAPH_STYLE_DISCRETE_X_SHAPE == p->nDotStyle )
			{

				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					//X,9点,一点一点傻画
					for(n = 0;n<sizeof(m_pXShapeOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pXShapeOffset[n].x,y+m_pXShapeOffset[n].y,p->crColor);
					}	
				}
			}
			else if(_ZYGRAPH_STYLE_DISCRETE_CROSS == p->nDotStyle )
			{

				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					//十字，9点,一点一点傻画
					for(n = 0;n<sizeof(m_pCrossOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pCrossOffset[n].x,y+m_pCrossOffset[n].y,p->crColor);
					}	
				}
			}
			else if(_ZYGRAPH_STYLE_DISCRETE_X_1_SHAPE == p->nDotStyle)
			{

				for( j=iXStart;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					//X多1竖线, 13点，一点一点傻画
					for(n = 0;n<sizeof(m_pX1ShapeOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pX1ShapeOffset[n].x,y+m_pX1ShapeOffset[n].y,p->crColor);
					}	
				}
			}
		}
		else if(_ZYGRAPH_STYLE_LINE == p->iGraphStyle )
		{
			oldPen = memDC.SelectObject(&p->penLine);
			memDC.MoveTo( 2 + perX * (p->pxData[iXStart] + p->dxDispShift-xLeft), 2 - perY*(log10(p->pyData[iXStart])-yTempTop) );
			for( j=iXStart+1;j<=iXEnd;j++)
			{
				x = 2 + perX * (p->pxData[j] + p->dxDispShift - xLeft);
				y = 2 - perY*(log10(p->pyData[j])-yTempTop);
				memDC.LineTo(x,y);
			}

			if(j == iXEnd+1)
			{//因为 LineTo 并不包括终止点，此处添加一点.
				memDC.SetPixel(x,y,p->crColor);
			}
			memDC.SelectObject(oldPen);			
		}
		else if(_ZYGRAPH_STYLE_DOT_LINE == p->iGraphStyle )
		{//点画线，在每点之上用图形，点与点之间用连线
			oldPen = memDC.SelectObject(&p->penLine);
			x = 2 + perX * ( p->pxData[iXStart] + p->dxDispShift -xLeft);
			y = 2 - perY*(log10(p->pyData[iXStart])-yTempTop);
			memDC.MoveTo(x,y);
			switch(p->nDotStyle)
			{
			case _ZYGRAPH_STYLE_DISCRETE_DIMOND:
				for(n = 0;n<sizeof(m_pDiamondOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pDiamondOffset[n].x,y+m_pDiamondOffset[n].y,p->crColor);
				}
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					for(n = 0;n<sizeof(m_pDiamondOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pDiamondOffset[n].x,y+m_pDiamondOffset[n].y,p->crColor);
					}
					memDC.LineTo(x,y);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_RECT:
				for(n = 0;n<sizeof(m_pRectOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pRectOffset[n].x,y+m_pRectOffset[n].y,p->crColor);
				}
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					for(n = 0;n<sizeof(m_pRectOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pRectOffset[n].x,y+m_pRectOffset[n].y,p->crColor);
					}
					memDC.LineTo(x,y);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_TRIANGLE:
				for(n = 0;n<sizeof(m_pTriangleOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pTriangleOffset[n].x,y+m_pTriangleOffset[n].y,p->crColor);
				}
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					for(n = 0;n<sizeof(m_pTriangleOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pTriangleOffset[n].x,y+m_pTriangleOffset[n].y,p->crColor);
					}
					memDC.LineTo(x,y);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_X_SHAPE:
				for(n = 0;n<sizeof(m_pXShapeOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pXShapeOffset[n].x,y+m_pXShapeOffset[n].y,p->crColor);
				}
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					for(n = 0;n<sizeof(m_pXShapeOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pXShapeOffset[n].x,y+m_pXShapeOffset[n].y,p->crColor);
					}
					memDC.LineTo(x,y);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_CROSS:
				for(n = 0;n<sizeof(m_pCrossOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pCrossOffset[n].x,y+m_pCrossOffset[n].y,p->crColor);
				}
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					for(n = 0;n<sizeof(m_pCrossOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pCrossOffset[n].x,y+m_pCrossOffset[n].y,p->crColor);
					}
					memDC.LineTo(x,y);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_X_1_SHAPE:
				for(n = 0;n<sizeof(m_pX1ShapeOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pX1ShapeOffset[n].x,y+m_pX1ShapeOffset[n].y,p->crColor);
				}
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					for(n = 0;n<sizeof(m_pX1ShapeOffset)/sizeof(CPoint);n++)
					{
						memDC.SetPixelV(x+m_pX1ShapeOffset[n].x,y+m_pX1ShapeOffset[n].y,p->crColor);
					}
					memDC.LineTo(x,y);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_DOT:
			default:
				for( j=iXStart+1;j<=iXEnd;j++)
				{
					x =  2 + perX * ( p->pxData[j] + p->dxDispShift -xLeft) ; //左端从2开始
					y =  2 - perY*(log10(p->pyData[j])-yTempTop);
					memDC.LineTo(x,y);
				}
				//最后再打一点，因为 lineto并不包含最后一点
				memDC.SetPixelV(2 + perX * ( p->pxData[iXEnd] + p->dxDispShift -xLeft) ,2 - perY*(log10(p->pyData[iXEnd])-yTempTop),p->crColor);
				break;
			}
			memDC.SelectObject(oldPen);
		}
	}
	//画外框
	fn_PlotFrame(memDC,mrect);

	//画标题栏及曲线提示 begin
	CDC memDC2;
	CBitmap sBmp2,*oldBmp2;
	memDC2.CreateCompatibleDC(pDestDC);
	sBmp2.CreateCompatibleBitmap(pDestDC,mrect.Width(),mrect.Height()); 
	oldBmp2 = memDC2.SelectObject(&sBmp2);
	CRect rt;
	CSize sz = memDC2.GetTextExtent(m_strTitle);
	rt.left = 0;
	rt.top  = 0;
	rt.bottom = sz.cy + 4;
	rt.right  = sz.cx + 4;
	memDC2.SetBkMode(TRANSPARENT);
	memDC2.SetBkColor(m_crTitleBk);
	//memDC2.SetDCPenColor(m_crTitle);
	memDC2.SetTextColor(m_crTitle);
	memDC2.FillSolidRect(&rt,m_crTitleBk);
	memDC2.TextOut(2,2,m_strTitle);

	BLENDFUNCTION bm;
	bm.BlendOp=AC_SRC_OVER;
	bm.BlendFlags=0;
	bm.SourceConstantAlpha = m_alphaTitle*255;
	bm.AlphaFormat=0; 
	if(!m_strTitle.IsEmpty())
	{//如果标题栏有字符	
		AlphaBlend(memDC.m_hDC,mrect.Width()/2-rt.Width()/2,3,rt.Width(),rt.Height(),memDC2.m_hDC,0,0,rt.Width(),rt.Height(),bm); 
	}
    //曲线提示，因为，单独画在右边 1/5以内.

	//int i, j; //检索用变量.  
	//int x, y; //x,y坐标位置变量
	rt.left = (mrect.Width() - 4 ) * 4 / 5; //左边沿
	rt.top  = 0;
	rt.right = mrect.Width() - 4 ; 
	rt.bottom = mrect.Height() - 4;

	memDC2.FillSolidRect(&mrect,m_crHintBk);
	sz = memDC2.GetTextExtent(CString(TEXT("TESTSTRING"))); //取得当前字符输出的高度
	y = 0; //记录起始左上角位置
	int nInterval = 4; //上下之间的间隔	
	pos = listData.GetHeadPosition();
	for (i=0;i < listData.GetCount();i++)
	{
		p = listData.GetNext(pos);
		if( 0 == p->iDataNumber || NULL == p->pyData )
		{ //如果没有数据，则跳过
			continue;
		}
		//作曲线示例
		oldPen = memDC2.SelectObject(&p->penLine);
        int y2 = y+sz.cy/2+nInterval/2;//用作新的y位置
		if( _ZYGRAPH_STYLE_LINE == p->iGraphStyle)
		{
			memDC2.MoveTo(3,y2);
			memDC2.LineTo(24,y2);
		}
		else
		{
			if( _ZYGRAPH_STYLE_DOT_LINE == p->iGraphStyle)
			{
				memDC2.MoveTo(3,y2);
				memDC2.LineTo(24,y2);
			}

			if( _ZYGRAPH_STYLE_DOT_LINE==p->iGraphStyle || _ZYGRAPH_STYLE_DISCRETE==p->iGraphStyle)
			{
				switch(p->nDotStyle)
				{
				case _ZYGRAPH_STYLE_DISCRETE_DOT:
					for(n = 3; n<24; n+=3)
					{
						//memDC2.SetPixelV(n,x,p->crColor);
						memDC2.SetPixelV(n,y2,p->crColor); //08.05.19
					}
					break;
				case _ZYGRAPH_STYLE_DISCRETE_DIMOND:
					for(n = 0;n<sizeof(m_pDiamondOffset)/sizeof(CPoint);n++)
					{
						memDC2.SetPixelV(14+m_pDiamondOffset[n].x,y2+m_pDiamondOffset[n].y,p->crColor);
					}
					break;
				case _ZYGRAPH_STYLE_DISCRETE_RECT:
					for(n = 0;n<sizeof(m_pRectOffset)/sizeof(CPoint);n++)
					{
						memDC2.SetPixelV(14+m_pRectOffset[n].x,y2+m_pRectOffset[n].y,p->crColor);
					}
					break;
				case _ZYGRAPH_STYLE_DISCRETE_TRIANGLE:
					for(n = 0;n<sizeof(m_pTriangleOffset)/sizeof(CPoint);n++)
					{
						memDC2.SetPixelV(14+m_pTriangleOffset[n].x,y2+m_pTriangleOffset[n].y,p->crColor);
					}
					break;
				case _ZYGRAPH_STYLE_DISCRETE_X_SHAPE:
					for(n = 0;n<sizeof(m_pXShapeOffset)/sizeof(CPoint);n++)
					{
						memDC2.SetPixelV(14+m_pXShapeOffset[n].x,y2+m_pXShapeOffset[n].y,p->crColor);
					}
					break;
				case _ZYGRAPH_STYLE_DISCRETE_CROSS:
					for(n = 0;n<sizeof(m_pCrossOffset)/sizeof(CPoint);n++)
					{
						memDC2.SetPixelV(14+m_pCrossOffset[n].x,y2+m_pCrossOffset[n].y,p->crColor);
					}
					break;
				case _ZYGRAPH_STYLE_DISCRETE_X_1_SHAPE:
					for(n = 0;n<sizeof(m_pX1ShapeOffset)/sizeof(CPoint);n++)
					{
						memDC2.SetPixelV(14+m_pX1ShapeOffset[n].x,y2+m_pX1ShapeOffset[n].y,p->crColor);
					}
					break;
				}
			}
		}

		memDC2.SelectObject(oldPen);
		//3，10，1，10，3	
		//写字符
		static TCHAR pcT[256];
		
		swprintf(pcT,255,TEXT("% 4d %s"),(int)p->dxDispShift,p->strName);
		memDC2.SetTextColor(p->crColor);
		memDC2.TextOut(3+10+1+10+3,y+nInterval/2,pcT);//p->strName);
		y += sz.cy + nInterval;
	}
	if( y!=0)
	{
		bm.SourceConstantAlpha = m_alphaHint*255;
		if(y >= rt.Height())
		{
			AlphaBlend(memDC.m_hDC,rt.left,0,rt.Width(),rt.Height()-1,memDC2.m_hDC,0,0,rt.Width(),rt.Height()-1,bm); 
		}
		else
		{
			AlphaBlend(memDC.m_hDC,rt.left,rt.Height()/2-y/2,rt.Width(),y,memDC2.m_hDC,0,0,rt.Width(),y,bm); 
		}
	}

	memDC2.SelectObject(oldBmp2);
	sBmp2.DeleteObject();
	//画标题栏及曲线图示 end	

	//Bitblt memory bitmap to view 
	pDestDC->BitBlt(rtDest.left,rtDest.top,mrect.Width(),mrect.Height(),&memDC,0,0,SRCCOPY);
	memDC.SelectObject(oldBitmap);
	srcBitmap.DeleteObject();
	return false;
}

//画背景与网格
void CZYGraphPlot::fn_prePlotGrid(CDC& dc, CRect& rect)
{
	//LineTo并不包括 LineTo(point)中的 point!!
	//(mrect.bottom , mrect.right)并不包含在 mrect中！
	
	//1, 画外边框. 占据上下共　4　点.
	//底色
	dc.FillSolidRect(&rect,m_clrBk); 	
	dc.SetBkMode(TRANSPARENT ); //set transparent bkmode, so the backcolor will not appear in drawtext

	//2, 画坐标轴
	CPen penGrid(PS_SOLID,1,m_clrGrid);
	CPen* oldpen = dc.SelectObject(&penGrid);
	//坐标轴外框
	dc.MoveTo(2,2);
	dc.LineTo(2,rect.bottom-3); //因为mrect.bottom不包含在内，而且已经用掉了 2 点.
	dc.LineTo(rect.right-3,rect.bottom -3);
	dc.LineTo(rect.right-3,2);
	dc.LineTo(2,2);

	int i;
	double dSize = (double) (rect.Width()-4)/ixGridNum;

	if(bxGridDisp)
	{
		for( i=1; i<ixGridNum; i++ )
		{//横坐标标尺，竖线条．分割宽
			dc.MoveTo(2+dSize*i, 2);
			dc.LineTo(2+dSize*i, rect.bottom-3);
		}
	}

	dSize = (double) (rect.Height()-4)/iyGridNum;
	for(i=1; i<iyGridNum; i++)
	//for(i=iyGridNum-1;i>=1;i--)
	{//纵坐标标尺，横线条．分割高
		dc.MoveTo(2, 2+dSize*i);
		dc.LineTo(rect.right -3, 2+dSize*i);
	}


	//写Y轴坐标字符
	dc.SetTextColor(m_clrYScale);
	CString str;
unsigned int old_exponent_format;
// Enable two-digit exponent format.2008.04.21
old_exponent_format = _set_output_format(_TWO_DIGIT_EXPONENT);
	str.Format(_T("(%.0f,%.1E)"),xRight,yTop);
	dc.TextOut(rect.Width()-2-dc.GetTextExtent(str).cx,4,str);
	str.Format(_T("(%.0f,%.1E)"),xLeft,yBot);
_set_output_format( old_exponent_format );
	//str.Format(_T("%.1E"),yBot);
	//dc.TextOut(rect.Width()-2-dc.GetTextExtent(str).cx,rect.Height()-dc.GetTextExtent(str).cy-2,str);
	dc.TextOut(4,rect.Height()-dc.GetTextExtent(str).cy-2,str);

	dc.SelectObject(oldpen);	
}

/******************************************************************************
// 在给定递增的pSrc序列中查找某个数值 应该对应的位置
输入参数：double dData 待查找的值
          double* pSrc 递增序列
		  int iSrcNum  递增序列的数据容量
		  bool bPrePos 查找到相应位置，是前一还是后一. 因为浮点数不太可能完全相等。位置提前1个或推后1个
输出参数：int& iPos    查找到的位置
返回值：  是否查找到。
******************************************************************************/
bool CZYGraphPlot::fn_findPos(double dData, int& iPos, double* pSrc, int iSrcNum, bool bPrePos)
{
	ASSERT(pSrc);
	if(iSrcNum<=1)
		return false;

	ASSERT(iSrcNum>1);


	iPos = -1;
	double dEpsilon = 1.0E-9;

	if( dData < pSrc[0] || dData > pSrc[iSrcNum-1] )
	{
		return false;
	}

	int iBegin = 0;
	int iEnd   = iSrcNum-1;
	int iMid;
	
	while( (iEnd-iBegin) > 1 )
	{
		iMid   = (iBegin+iEnd)/2;
		if( dData > pSrc[iMid] )
		{
			if( dData - pSrc[iMid] > dEpsilon )
			{
				iBegin = iMid;
			}
			else
			{
				//iPos = iMid;
				iBegin = iMid;
				iEnd = __min(iMid+1,iSrcNum-1);
				break;
			}
		}
		else
		{
			iEnd = iMid;
		}
	}

	if(bPrePos)
	{
		iPos = iBegin;
	}
	else
	{
		iPos = iEnd;
	}

	return true;
}

// 画外框
void CZYGraphPlot::fn_PlotFrame(CDC& dc , CRect& rect)
{
	COLORREF     gclr_PicLTOutside = RGB(172,168,153); //Left Top
	COLORREF     gclr_PicLTInside  = RGB(113,111,100);
	COLORREF     gclr_PicRBOutside = RGB(255,255,255); //Right bottom
	COLORREF     gclr_PicRBInside  = RGB(241,239,226);
	//LT Inside
	CPen penLTInside(PS_SOLID,1,gclr_PicLTInside);
	CPen* oldpen = dc.SelectObject(&penLTInside);
	dc.MoveTo(0,1);
	dc.LineTo(rect.right-1,1);
	dc.MoveTo(1,0);
	dc.LineTo(1,rect.bottom-1);
	//LT Outside;
	CPen penLTOutside(PS_SOLID,1,gclr_PicLTOutside);
	dc.SelectObject(&penLTOutside);

	bool b = penLTInside.DeleteObject();

	dc.MoveTo(0,0);
	dc.LineTo(rect.right,0);
	dc.MoveTo(0,0);
	dc.LineTo(0,rect.bottom);
	//RB Inside
	CPen penRBInside(PS_SOLID,1,gclr_PicRBInside);
	dc.SelectObject(&penRBInside);

	penLTOutside.DeleteObject();

	dc.MoveTo(1,rect.bottom-2);
	dc.LineTo(rect.right,rect.bottom-2);
	dc.MoveTo(rect.right-2,1);
	dc.LineTo(rect.right-2,rect.bottom);
	//RB Outside
	CPen penRBOutside(PS_SOLID,1,gclr_PicRBOutside);
	dc.SelectObject(&penRBOutside);

	penRBInside.DeleteObject();

	dc.MoveTo(0,rect.bottom-1);
	dc.LineTo(rect.right-1,rect.bottom-1);
	dc.MoveTo(rect.right-1,0);
	dc.LineTo(rect.right-1,rect.bottom);
	dc.SelectObject(oldpen);

	penRBOutside.DeleteObject();
}
/****************************************************************************
函数名称：SaveDCBitmap
函数说明：将DC中指定区域(rect)按照bmp格式写入文件(filename)中．
输入参数：CDC* pDC, 源DC
CRect rect, 指定区域
CString filename, bmp文件名称
简评：　　简单易用．　局限于　bmp文件
2008.04.03添加
****************************************************************************/
bool SaveDCBitmap(CDC* pDC,CRect rect,LPCTSTR lpszFileName)
{ 
	CDC* memDC=new CDC;
	memDC->CreateCompatibleDC(pDC);
	CBitmap* bmp=new CBitmap;
	bmp->CreateCompatibleBitmap(pDC,rect.Width(),rect.Height());
	CBitmap* oldbitmap=memDC->SelectObject(bmp);
	//此时的bmp就相当于一张桌布，在memDC中画线etc都是画在这张桌布上
	if(!memDC->BitBlt(0,0,rect.Width(),rect.Height(),pDC,0,0,SRCCOPY))
	{
		AfxMessageBox(TEXT("BitBlt Error!"));
		return false;
	}
	//memDC->Ellipse(0,0,100,100);
	memDC->SelectObject(oldbitmap);

	BITMAPINFO bi;
	bi.bmiHeader.biSize=sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth=rect.Width();
	bi.bmiHeader.biHeight=rect.Height();
	bi.bmiHeader.biPlanes=1;
	bi.bmiHeader.biBitCount=16;
	bi.bmiHeader.biCompression=BI_RGB;
	bi.bmiHeader.biSizeImage=0;
	bi.bmiHeader.biXPelsPerMeter=0;
	bi.bmiHeader.biYPelsPerMeter=0;
	bi.bmiHeader.biClrUsed=0;
	bi.bmiHeader.biClrImportant=0;

	int bitsize=rect.Width()*rect.Height()*2;

	BYTE* bits=new BYTE[bitsize];
	::GetDIBits(memDC->m_hDC,*bmp,0,rect.Height(),bits,&bi,DIB_RGB_COLORS); 

	BITMAPFILEHEADER bf;
	bf.bfType=(int)'M'*256+'B';
	bf.bfSize=bitsize;//sizeof(bf);
	bf.bfOffBits=sizeof(bi.bmiHeader)+sizeof(bf);
	bf.bfReserved1=0;
	bf.bfReserved2=0;

	CFile f(lpszFileName,CFile::modeCreate|CFile::modeWrite);
	f.Write(&bf,sizeof(bf));//注意是先写bf,再写bi
	f.Write(&bi,sizeof(bi));
	f.Write(bits,bitsize);
	f.Close();

	delete[] bits;
	delete bmp;
	delete memDC;
	return true;
} 
// 用当前背景，作线条示例. 
void CZYGraphPlot::fn_PlotIllustration(CDC* pDestDC, CRect& rect, COLORREF crClr, int nGraphStyle, int nDotStyle,int nPenStyle=PS_SOLID)
{
	CDC memDC;
	CBitmap srcBitmap, *oldBitmap;
	CRect rtDest = rect;

	CRect mrect = rtDest; //draw rect in memdc
	mrect.OffsetRect(-rtDest.left,-rtDest.top); // zero based in memdc

	//create memDC and inside bitmap
	memDC.CreateCompatibleDC(pDestDC);
	srcBitmap.CreateCompatibleBitmap(pDestDC,mrect.Width(),mrect.Height()); 
	oldBitmap = memDC.SelectObject(&srcBitmap);
	memDC.SetBkColor(m_clrBk);
	memDC.FillSolidRect(mrect,m_clrBk);

	CPen penGrid(PS_SOLID,1,m_clrGrid);
	CPen* oldpen = memDC.SelectObject(&penGrid);
	//坐标轴外框
	memDC.MoveTo(2,2);
	memDC.LineTo(2,mrect.bottom-3); //因为mrect.bottom不包含在内，而且已经用掉了 2 点.
	memDC.LineTo(mrect.right-3,mrect.bottom -3);
	memDC.LineTo(mrect.right-3,2);
	memDC.LineTo(2,2);

	CPen pen(nPenStyle,1,crClr);
	memDC.SelectObject(&pen);
	penGrid.DeleteObject();

	//画点
	if( _ZYGRAPH_STYLE_DISCRETE == nGraphStyle || _ZYGRAPH_STYLE_DOT_LINE == nGraphStyle )
	{
		//先确定需要画多少点
		int x,y,n;
		int nCount = ( mrect.Width() - 6 ) / ( 8 + 15) ;//
		double dPerDot =(double)  mrect.Width()/( nCount+1 );
		x = 0;
		y = mrect.Height()/2;
		for( int i=0; i< nCount; i++)
		{
			x =(i+1)*dPerDot;			
			switch(nDotStyle)
			{
			case _ZYGRAPH_STYLE_DISCRETE_DOT:
				memDC.SetPixelV(x,y,crClr);
				break;
			case _ZYGRAPH_STYLE_DISCRETE_DIMOND:
				for(n = 0;n<sizeof(m_pDiamondOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pDiamondOffset[n].x,y+m_pDiamondOffset[n].y,crClr);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_RECT:
				for(n = 0;n<sizeof(m_pRectOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pRectOffset[n].x,y+m_pRectOffset[n].y,crClr);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_TRIANGLE:
				for(n = 0;n<sizeof(m_pTriangleOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pTriangleOffset[n].x,y+m_pTriangleOffset[n].y,crClr);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_X_SHAPE:
				for(n = 0;n<sizeof(m_pXShapeOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pXShapeOffset[n].x,y+m_pXShapeOffset[n].y,crClr);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_CROSS:
				for(n = 0;n<sizeof(m_pCrossOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pCrossOffset[n].x,y+m_pCrossOffset[n].y,crClr);
				}
				break;
			case _ZYGRAPH_STYLE_DISCRETE_X_1_SHAPE:
				for(n = 0;n<sizeof(m_pX1ShapeOffset)/sizeof(CPoint);n++)
				{
					memDC.SetPixelV(x+m_pX1ShapeOffset[n].x,y+m_pX1ShapeOffset[n].y,crClr);
				}
				break;
			}
		}
	}

	//画线段
	if(_ZYGRAPH_STYLE_LINE == nGraphStyle || _ZYGRAPH_STYLE_DOT_LINE == nGraphStyle)
	{
		memDC.MoveTo(6,mrect.Height()/2);
		memDC.LineTo(mrect.Width()-6,mrect.Height()/2);
	}

	pDestDC->BitBlt(rtDest.left,rtDest.top,mrect.Width(),mrect.Height(),&memDC,0,0,SRCCOPY);
	memDC.SelectObject(oldBitmap);
	srcBitmap.DeleteObject();
	memDC.SelectObject(oldpen);
	pen.DeleteObject();
}

// 用内部第1条数据进行演示绘图
int CZYGraphPlot::fn_PlotIllustrUse1st(CDC* pDestDC, CRect& rect,COLORREF crBk, COLORREF crGrid)
{
	CZYGraphData* p;
	POSITION pos = listData.GetHeadPosition();
	if(pos)
	{
		p = listData.GetNext(pos);
		COLORREF crBk_b = m_clrBk;
		COLORREF crGrid_b = m_clrGrid;
		m_clrBk = crBk;
		m_clrGrid = crGrid;

		fn_PlotIllustration(pDestDC,rect,p->crColor,p->iGraphStyle,p->nDotStyle,p->nPenStyle);

		m_clrBk = crBk_b;
		m_clrGrid = crGrid_b;
	}
	return 0;
}
