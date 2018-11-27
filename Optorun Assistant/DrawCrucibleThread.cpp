// DrawCrucibleThread.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DrawCrucibleThread.h"
#include "OACnfgDoc.h"

#include <math.h>
#define pi 3.1415926535897932
// CDrawCrucibleThread

#define ZZY_LED_NUMBER_WIDTH  15  //数值应当随所定义的图片尺寸而定.
#define ZZY_LED_NUMBER_HEIGHT 20


COLORREF g_clrHearthExteriorCircle = RGB(0,128,0); 
COLORREF g_clrHearthCrucibleCircle = RGB(0,128,0); //pen color
COLORREF g_clrHearthCrucibleInside = GetSysColor(COLOR_3DFACE); //brush color
COLORREF g_clrHearthCrucibleText   = RGB(0,0,128); //font
COLORREF g_clrHearthAngle		   = RGB(0,0,0);
COLORREF g_clrHearthOriginLine     = RGB(200,0,0); //原点线


IMPLEMENT_DYNCREATE(CDrawCrucibleThread, CWinThread)

CDrawCrucibleThread::CDrawCrucibleThread()
{
}

CDrawCrucibleThread::~CDrawCrucibleThread()
{
}

BOOL CDrawCrucibleThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	m_LEDNumber.LoadBitmap(IDB_LED_NUMBER);
	return TRUE;
}

int CDrawCrucibleThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CDrawCrucibleThread, CWinThread)
	ON_THREAD_MESSAGE(WM_ITEM3_DRAW_CRUCIBLE,  OnDrawCrucible)
END_MESSAGE_MAP()

/*
根据 lParam 的数据进行Crucible绘图，无需再反馈！
*/
void CDrawCrucibleThread::OnDrawCrucible(WPARAM wParam, LPARAM lParam)
{
	struct _stDrawHearth* pstDrawData = (struct _stDrawHearth*)lParam;

	if( ! IsWindow(pstDrawData->hwnd))
	{
		return;
	}

//	if(pstDrawData->iCrucibleMaxNumber == 0)
//		return; //环形坩埚，暂时不管.

	HDC hDC = GetDC(pstDrawData->hwnd);
	CDC* pDC = CDC::FromHandle(hDC);

	CDC memDC;
	CBitmap srcBitmap, *oldBitmap;

	//create memDC and inside bitmap
	memDC.CreateCompatibleDC(pDC);
	srcBitmap.CreateCompatibleBitmap(pDC,pstDrawData->rect.Width(),pstDrawData->rect.Height()); 
	oldBitmap = memDC.SelectObject(&srcBitmap);

	memDC.SetBkColor(GetSysColor(COLOR_3DFACE)); //TEST


	int iSquareWidth = __min(pstDrawData->rect.Width(),pstDrawData->rect.Height());
	/************************************************************************************
	令正方形边长为 W, 小坩埚直径为 d ,　则坩埚中心圆直径 W-d.
	作图中为不至于　令小坩埚挤在一起，　每个坩埚应占据　中心圆边长　约　1.2*d.
	可得方程　1.2*d*n = pi * (W-d)　　其中　n 为坩埚总数， pi
	d = pi * W /(1.2*n + pi)
	*************************************************************************************/
	double d = pi * (double)iSquareWidth / (1.4 * pstDrawData->iCrucibleMaxNumber + pi);

	//之所以将直径转化成奇数，是因为屏幕是以　点　作计算单位的，整数． 而圆心需要占据 1 点.
	//下面的直径和半径都是以点作单位的，　半径 = (直径-1)/2.
	//小坩埚直径
	int iCrucibleDiameter = (int) d;
	//转成 奇数
	if( iCrucibleDiameter % 2 == 0)
		iCrucibleDiameter++;

	//中心圆直径,保证奇数 .
	int iExteriorDiameter = iSquareWidth  -  iCrucibleDiameter;
	if( iExteriorDiameter % 2 == 0)
		iExteriorDiameter--;

	iSquareWidth = iCrucibleDiameter + iExteriorDiameter - 1; //已包含右下边

	//总画图尺寸 (包含右下)
	CRect rect;
	rect.left = 0;
	rect.top  = 0;
	rect.right  = iSquareWidth;
	rect.bottom = iSquareWidth;

	//LineTo并不包括 LineTo(point)中的 point!!

	//1,画底色
	memDC.FillSolidRect(0,0,pstDrawData->rect.Width(),pstDrawData->rect.Height(),GetSysColor(COLOR_3DFACE)); 	
	//memDC.FillSolidRect(0,0,pstDrawData->rect.Width(),pstDrawData->rect.Height(),RGB(0,0,0)); 	

	memDC.SetBkMode(TRANSPARENT ); //set transparent bkmode, so the backcolor will not appear in drawtext

	CPen penExteriorCircle(PS_SOLID,1,g_clrHearthExteriorCircle);
	CPen* oldpen = memDC.SelectObject(&penExteriorCircle);

	//在左上角写上Hearth号
	memDC.TextOut(0,0,pstDrawData->iHearth==0?TEXT("1"):TEXT("2") ,1);

	CBrush* oldbrush = NULL;
	CFont*  oldfont = NULL;

	int cx,cy;
	if(pstDrawData->iCrucibleMaxNumber == 0)
	{//画环形坩埚
		CRect rtOutside = rect;
		//外圆
		rtOutside.DeflateRect(2,2);
		memDC.Arc(&rtOutside,CPoint(0,0),CPoint(0,0)); //中心圆
		//内圆
		rtOutside.DeflateRect(rtOutside.Width()/8,rtOutside.Width()/8);
		memDC.Arc(&rtOutside,CPoint(0,0),CPoint(0,0)); //中心圆

		//编码器000位置
		cx = rtOutside.CenterPoint().x;
		cy = rtOutside.CenterPoint().y;
		
		double dAngle = 0; //
		double dStartAngle = pstDrawData->iCoderVal * pi /180; 

		//计算小圆中心在　画图上的坐标.  //视图的坐标　右下为正. 而小圆起点在下方正中
		int x = cx - (double)( rtOutside.Width()/2 * sin(dStartAngle-dAngle) - 0.5 ); //角度相对于原点　来说　是负值，所以需要减去
		int y = cy + (double)( rtOutside.Width()/2 * cos(dStartAngle-dAngle) + 0.5 );
		memDC.MoveTo(cx,cy);
		memDC.LineTo(x,y);

		//当前位置线
		CPen penHearthOrigin(PS_SOLID,1,g_clrHearthOriginLine);
		memDC.SelectObject(&penHearthOrigin);
		memDC.MoveTo(cx,cy);
		int iHalfWidth = rtOutside.Width() /2;
		memDC.LineTo(cx,cy + iHalfWidth );
		//小箭头
		memDC.MoveTo(cx,cy + iHalfWidth );
		memDC.LineTo(cx-4,cy + iHalfWidth-8 );
		memDC.MoveTo(cx,cy + iHalfWidth  );
		memDC.LineTo(cx+4,cy + iHalfWidth -8 );

		//在中心位置输入编码器数值 以及当前速度
		int iNumberX = cx - ZZY_LED_NUMBER_WIDTH * 1.5;
		int iNumberY = cy;//- ZZY_LED_NUMBER_HEIGHT/2;
		if( pstDrawData->iCoderVal>1000 ||  pstDrawData->iCoderVal< 0 )
		{
			pstDrawData->iCoderVal = abs(pstDrawData->iCoderVal)%1000;
		}
		CDC   csDC;
		csDC.CreateCompatibleDC(&memDC);
		CBitmap* oldNumberBitmap = csDC.SelectObject(&m_LEDNumber);
		
		int iNumber;
		for( int i=0;i<3;i++)
		{
			iNumber = 0==i ? pstDrawData->iCoderVal/100:(1==i?(pstDrawData->iCoderVal/10)%10:pstDrawData->iCoderVal%10) ;
			memDC.BitBlt(iNumberX+i*ZZY_LED_NUMBER_WIDTH,iNumberY, ZZY_LED_NUMBER_WIDTH,ZZY_LED_NUMBER_HEIGHT,&csDC, ZZY_LED_NUMBER_WIDTH*iNumber,0,SRCCOPY);

			//
			iNumber = 0==i ? pstDrawData->iSpeed/100:(1==i?(pstDrawData->iSpeed/10)%10:pstDrawData->iSpeed%10);
			memDC.BitBlt(iNumberX+i*ZZY_LED_NUMBER_WIDTH,iNumberY-ZZY_LED_NUMBER_HEIGHT-1, ZZY_LED_NUMBER_WIDTH,ZZY_LED_NUMBER_HEIGHT,&csDC, ZZY_LED_NUMBER_WIDTH*iNumber,0,SRCCOPY);
		}

		csDC.SelectObject(oldNumberBitmap);
	}
	else
	{//画点坩埚

		//2, 画中心圆
		//获取中心圆的外框. Arc函数在msdn中没有看到不包含　右下边的说法，而 Ellipse不包含右下
		CRect rtExteriorRect;
		rtExteriorRect.left   = iCrucibleDiameter/2 ; 
		rtExteriorRect.top    = iCrucibleDiameter/2 ;
		rtExteriorRect.right  = rtExteriorRect.left + iExteriorDiameter;
		rtExteriorRect.bottom = rtExteriorRect.top  + iExteriorDiameter;//扣掉右下
		//	memDC.Arc(&rtExteriorRect,CPoint(0,0),CPoint(0,0)); //中心圆

		//3,　画坩埚（小圆）
		CPen penCrucibleCircle(PS_SOLID,1,g_clrHearthCrucibleCircle);
		memDC.SelectObject(&penCrucibleCircle);

		CBrush brCrucibleInside(g_clrHearthCrucibleInside);
		oldbrush = memDC.SelectObject(&brCrucibleInside);

		CFont ftCrucibleTextFont;
		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
		lf.lfHeight = iCrucibleDiameter - 4 ;                      // request a 12-pixel-height font
		_tcscpy (lf.lfFaceName, TEXT("Arial"));        // request a face name "Arial"
		VERIFY(ftCrucibleTextFont.CreateFontIndirect(&lf));  // create the font
		oldfont = memDC.SelectObject(&ftCrucibleTextFont);

		memDC.SetTextColor(g_clrHearthCrucibleText);
		//坩埚中心均匀分布在　中心圆上，　起始角度在　下方正中.
		//坩埚逆时针分布
		//起始角度 就是 读数
		int x,y; //小圆中心
		int r = iCrucibleDiameter / 2 ;//小圆半径

		//中心圆　圆心坐标
		cx = rtExteriorRect.CenterPoint().x ;
		cy = rtExteriorRect.CenterPoint().y ;
		//中心圆　半径
		int cr = rtExteriorRect.Width() / 2;

		CString str;
		CRect rtCrucible;
		double dIncAngle = 2 * pi / pstDrawData->iCrucibleMaxNumber; //角度递增
		double dStartAngle = pstDrawData->iCoderVal * pi /180;
		//double dStartAngle = 20 * pi /180;
		for(int i=0;i<pstDrawData->iCrucibleMaxNumber;i++)
		{
			//计算小圆中心在　画图上的坐标.  //视图的坐标　右下为正. 而小圆起点在下方正中
			x = cx - (double)( cr*sin(dStartAngle-i*dIncAngle) - 0.5 ); //角度相对于原点　来说　是负值，所以需要减去
			y = cy + (double)( cr*cos(dStartAngle-i*dIncAngle) + 0.5 );

			if(i==0)
			{
				memDC.MoveTo(cx,cy);
				memDC.LineTo(x,y);
			}

			//得出小圆的外接矩形
			rtCrucible.left = x - r;
			rtCrucible.top  = y - r;
			rtCrucible.right = x + r + 1; //Ellipse函数不包括　右下边沿，故　+1
			rtCrucible.bottom = y + r + 1;

			memDC.Ellipse(&rtCrucible);
			str.Format( TEXT("%d"), i+1 );
			memDC.DrawText(str,&rtCrucible,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		}

		//当前位置线
		CPen penHearthOrigin(PS_SOLID,1,g_clrHearthOriginLine);
		memDC.SelectObject(&penHearthOrigin);
		memDC.MoveTo(cx,cy);
		memDC.LineTo(cx,cy + iSquareWidth/2 - iCrucibleDiameter );
		//小箭头
		memDC.MoveTo(cx,cy + iSquareWidth/2 - iCrucibleDiameter );
		memDC.LineTo(cx-4,cy + iSquareWidth/2 - iCrucibleDiameter-8 );
		memDC.MoveTo(cx,cy + iSquareWidth/2 - iCrucibleDiameter );
		memDC.LineTo(cx+4,cy + iSquareWidth/2 - iCrucibleDiameter-8 );


		//在中心位置输入编码器数值 以及坩埚号
		int iNumberX = cx - ZZY_LED_NUMBER_WIDTH * 1.5;
		int iNumberY = cy;//- ZZY_LED_NUMBER_HEIGHT/2;
		if( pstDrawData->iCoderVal>1000 ||  pstDrawData->iCoderVal< 0 )
		{
			pstDrawData->iCoderVal = abs(pstDrawData->iCoderVal)%1000;
		}
		CDC   csDC;
		csDC.CreateCompatibleDC(&memDC);
		CBitmap* oldNumberBitmap = csDC.SelectObject(&m_LEDNumber);
		
		int iNumber;
		for( int i=0;i<3;i++)
		{
			iNumber = 0==i ? pstDrawData->iCoderVal/100:(1==i?(pstDrawData->iCoderVal/10)%10:pstDrawData->iCoderVal%10) ;
			memDC.BitBlt(iNumberX+i*ZZY_LED_NUMBER_WIDTH,iNumberY, ZZY_LED_NUMBER_WIDTH,ZZY_LED_NUMBER_HEIGHT,&csDC, ZZY_LED_NUMBER_WIDTH*iNumber,0,SRCCOPY);
		}

		iNumber = pstDrawData->iCurCurcible/10;
		memDC.BitBlt(iNumberX+ZZY_LED_NUMBER_WIDTH,iNumberY-ZZY_LED_NUMBER_HEIGHT-1,ZZY_LED_NUMBER_WIDTH,ZZY_LED_NUMBER_HEIGHT,&csDC, ZZY_LED_NUMBER_WIDTH*iNumber,0,SRCCOPY);
		iNumber = pstDrawData->iCurCurcible%10;
		memDC.BitBlt(iNumberX+2*ZZY_LED_NUMBER_WIDTH,iNumberY-ZZY_LED_NUMBER_HEIGHT-1,ZZY_LED_NUMBER_WIDTH,ZZY_LED_NUMBER_HEIGHT,&csDC, ZZY_LED_NUMBER_WIDTH*iNumber,0,SRCCOPY);

		csDC.SelectObject(oldNumberBitmap);


	} //画点坩埚结束


	//Bitblt memory bitmap to view 
	pDC->BitBlt(pstDrawData->rect.left,pstDrawData->rect.top,pstDrawData->rect.Width(),pstDrawData->rect.Height(),&memDC,0,0,SRCCOPY);
	memDC.SelectObject(oldBitmap);
	srcBitmap.DeleteObject();
	memDC.SelectObject(oldpen);
	if(oldbrush)
		memDC.SelectObject(oldbrush);
	if(oldfont)
		memDC.SelectObject(oldfont);

	ReleaseDC(pstDrawData->hwnd,hDC);
}

// CDrawCrucibleThread message handlers
