// DlgInitVacTst.cpp : implementation file
//

#include "stdafx.h"
#include "Optorun Assistant.h"
#include "DlgInitVacTst.h"

#include "OACnfgDoc.h"

// CDlgInitVacTst dialog

IMPLEMENT_DYNAMIC(CDlgInitVacTst, CDialog)

CDlgInitVacTst::CDlgInitVacTst(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInitVacTst::IDD, pParent)
{
	m_unCurrentTimeEvent = 0;
	m_dwMaxWaitingTime = 10;
	m_nRetValue = 0;
}

CDlgInitVacTst::~CDlgInitVacTst()
{

}

void CDlgInitVacTst::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgInitVacTst, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CDlgInitVacTst::OnBnClickedCancel)
	ON_WM_TIMER()
//	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgInitVacTst message handlers

void CDlgInitVacTst::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	m_nRetValue = _DLG_INIT_VAC_TST_RT_CANCEL;
	OnCancel();
}

BOOL CDlgInitVacTst::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	TCHAR buf[1024];

	m_bFirstTimer = true;

	m_unCurrentTimeEvent = SetTimer(1,1000,NULL);
	m_dwInitTick = GetTickCount();

	if( 0 == m_unCurrentTimeEvent)
	{
		AfxMessageBox(TEXT("Error: Failed to setup timer."),MB_OK|MB_ICONSTOP);
		m_nRetValue = 0;
		return false;
	}
	
	_stprintf(buf,TEXT("%02d:%02d:%02d"),m_dwMaxWaitingTime/3600,(m_dwMaxWaitingTime/60)%60 ,m_dwMaxWaitingTime%60);
	SetDlgItemText(IDC_STATIC_MAX_WAITING_TIME,buf);

	_stprintf(buf,TEXT("%.1E Pa"),	m_dStartPressure);
	SetDlgItemText(IDC_STATIC_START_PRESSURE,buf);

	_stprintf(buf,TEXT("%02d:%02d:%02d"),m_dwPressureKeepTime/3600,(m_dwPressureKeepTime/60)%60 ,m_dwPressureKeepTime%60);
	SetDlgItemText(IDC_STATIC_PRESSURE_MIN_KEEP_TIME,buf);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgInitVacTst::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	TCHAR buf[1024];
	if(nIDEvent == m_unCurrentTimeEvent)
	{
		double dPressure;
		bool   b;
		
		static int  iKeepTime = 0;
		if(m_bFirstTimer)
		{
			iKeepTime = 0;
			m_bFirstTimer = false;
		}

		int iTemp = (GetTickCount()-m_dwInitTick)/1000;

		MSG    msg;
		static bool bLbuttonDonw = false;
		CPoint pt;
		CRect  rect;
		int    istart;

		if( g_pVacProThread->fn_GetVGCch3Pressure(dPressure))
		{
			//通信成功
			if(dPressure<=m_dStartPressure)
			{//此时如果主阀处于关闭状态，则立即退出对话框，　进入记录状态．
				if( g_pVacProThread->fn_CheckMainValveStatus(b))
				{
					if( !b )
					{// bOpen == false. 关闭状态． 退出对话框，进入记录状态
						m_nRetValue =_DLG_INIT_VAC_TST_RT_OK;
						OnOK();
						return;
					}
					else
					{// bOpen == true.  开启状态． 
						iKeepTime++ ;
						if(iKeepTime >= m_dwPressureKeepTime)
						{
							//等待完成，应　模拟手工　关闭主阀，进入记录状态.
							if(m_unCurrentTimeEvent)
							{
								KillTimer(m_unCurrentTimeEvent);
							}
							m_unCurrentTimeEvent = 0;
							//1, 提示用户，程序开始模拟按触摸屏，用户不应按触摸屏．

							

							//2, 首先确定　<手动排气>　模式
							if( g_pVacProThread->fn_SetExhaustMode(true) )
							{
							}
							else
							{
								goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
							}

							Sleep(200);
							//确认处于 <手工排气>模式
							bool bManualMode = false;
							istart = GetTickCount();
							while( (GetTickCount()-istart)/1000 < 10 )
							{

								_stprintf(buf,TEXT("确认<手动排气>中...%d 秒"), (GetTickCount()-istart)/1000);
								SetDlgItemText(IDC_STATIC_STATUS,buf);


								if( g_pVacProThread->fn_CheckExhaustMode(bManualMode))
								{
									if(bManualMode)
									{
										break; //是手工模式，则退出确认　<手工排气>流程
									}
								}
								else
								{
									goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
								}

								while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
								{
									if( msg.message==WM_TIMER && msg.hwnd==GetSafeHwnd())
									{ //不再响应　WM_TIMER

									}
									else if(msg.message==WM_LBUTTONDOWN)
									{
										GetCursorPos(&pt);
										GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
										if(rect.PtInRect(pt))
										{
											bLbuttonDonw = true;
										}
										else
										{
											bLbuttonDonw = false;
										}
									}
									else if(msg.message==WM_LBUTTONUP)
									{
										GetCursorPos(&pt);	
										GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
										if(rect.PtInRect(pt))
										{
											if(bLbuttonDonw)
											{
												m_nRetValue =_DLG_INIT_VAC_TST_RT_CANCEL;
												KillTimer(m_unCurrentTimeEvent);
												m_unCurrentTimeEvent = 0;
												AfxMessageBox(TEXT("Exit: hehehe 退出！"));  //可以放到 调用处触发．
												OnCancel();
												return;
											}
										}					
									}
									else
									{
										TranslateMessage(&msg);
										DispatchMessage(&msg);
									}
								}
								Sleep(200);
							}

							if(!bManualMode)
							{//在 10 秒钟内仍没有进入　<手工排气> 模式．报错，退出.
								m_nRetValue =_DLG_INIT_VAC_TST_RT_MANUAL_ERR;
								if(m_unCurrentTimeEvent)
									KillTimer(m_unCurrentTimeEvent);
								m_unCurrentTimeEvent = 0;

								AfxMessageBox(g_item2_String[_ITEM2_STR_MANUAL_EXHUAST_MODE_ERR].IsEmpty()?
									TEXT("出错：规定时间内没能进入手动排气模式．\r\n提示：默认１０秒！\r\nError:Cannot get into manual exhuast mode in 10 seconds."):g_item2_String[_ITEM2_STR_MANUAL_EXHUAST_MODE_ERR]);
								OnCancel();	
								return;
							}

							//3, 模拟触摸屏按一次 <主阀>.
							//提示：确认处于 <主阀关闭>  中.
							if(g_pVacProThread->fn_SwitchMainValve())
							{
							}
							else
							{
								goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
							}

							Sleep(200);
							bool bMainValveOpen = true; //抽气状态
							istart = GetTickCount();
							while( (GetTickCount()-istart)/1000 < 10 )
							{
								_stprintf(buf,TEXT("确认<主阀关闭>中...%d 秒"), (GetTickCount()-istart)/1000);
								SetDlgItemText(IDC_STATIC_STATUS,buf);

								if( g_pVacProThread->fn_CheckMainValveStatus(bMainValveOpen) )
								{
									if(!bMainValveOpen)
									{//主阀关闭，可以进行记录了．

										m_nRetValue =_DLG_INIT_VAC_TST_RT_OK;
										OnOK();
										return;
									}
								}
								else
								{
									goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
								}

								while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
								{
									if( msg.message==WM_TIMER && msg.hwnd==GetSafeHwnd())
									{ //不再响应　WM_TIMER

									}
									else if(msg.message==WM_LBUTTONDOWN)
									{
										GetCursorPos(&pt);
										GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
										if(rect.PtInRect(pt))
										{
											bLbuttonDonw = true;
										}
										else
										{
											bLbuttonDonw = false;
										}
									}
									else if(msg.message==WM_LBUTTONUP)
									{
										GetCursorPos(&pt);	
										GetDlgItem(IDCANCEL)->GetWindowRect(&rect);
										if(rect.PtInRect(pt))
										{
											if(bLbuttonDonw)
											{
												m_nRetValue =_DLG_INIT_VAC_TST_RT_CANCEL;
												if(m_unCurrentTimeEvent)
													KillTimer(m_unCurrentTimeEvent);
												m_unCurrentTimeEvent = 0;
												AfxMessageBox(TEXT("Exit: hehehe 退出！"));  //可以放到 调用处触发．
												OnCancel();
												return;
											}
										}					
									}
									else
									{
										TranslateMessage(&msg);
										DispatchMessage(&msg);
									}
								}
								Sleep(200);
							}

							//主阀必然处于开启状态
							ASSERT(bMainValveOpen);
							// bMainValveOpen
							//在 10 秒钟内仍没有关闭高阀，则报错并退出．

							m_nRetValue =_DLG_INIT_VAC_TST_RT_MV_ERR;
							if(m_unCurrentTimeEvent)
								KillTimer(m_unCurrentTimeEvent);
							m_unCurrentTimeEvent = 0;
							AfxMessageBox(g_item2_String[_ITEM2_STR_MAIN_VALVE_CLOSE_ERR].IsEmpty()?
									TEXT("出错：规定时间内主阀没有关闭．\r\n提示：默认１０秒！\r\nError: Main Valve didn't close in 10 seconds."):g_item2_String[_ITEM2_STR_MAIN_VALVE_CLOSE_ERR]);
							//AfxMessageBox(TEXT("出错：规定时间内主阀没有关闭．\r\n提示：默认１０秒！"));  //可以放到 调用处触发．
							OnCancel();	
							return;

						}				

					}
				}
				else
				{
					goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
				}					
			}
			else
			{ 
				iKeepTime--;
				if(iKeepTime<0)
				{
					iKeepTime = 0;
				}

				// 起始真空度没有达到， 则进一步检查主阀状态，此时应该为关闭，说明仍在抽气，否则报错退出．
				if( g_pVacProThread->fn_CheckMainValveStatus(b))
				{
					if( !b )
					{// bOpen == false. 关闭状态．　没有达到起始真空度，此时的主阀却已经关闭．报错
						if(  0.1 <  ( dPressure - m_dStartPressure ) / m_dStartPressure )
						{
							//真空度与设定起始真空度差别超过　10%，则报错．
							m_nRetValue =_DLG_INIT_VAC_TST_RT_PRESS_ERR;
							KillTimer(m_unCurrentTimeEvent);
							m_unCurrentTimeEvent = 0;
							AfxMessageBox(g_item2_String[_ITEM2_STR_START_PRESSURE_ERR].IsEmpty()?
								TEXT("Error: 起始真空度没有达到．\r\n提示：起始真空度没有达到的情况下，主阀须处于开启状态．才可进行测试！\r\nError: Main valve is closed unexpectedly."):g_item2_String[_ITEM2_STR_START_PRESSURE_ERR]);
							//AfxMessageBox(TEXT("Exit: 起始真空度没有达到．\r\n提示：起始真空度没有达到的情况下，主阀须处于开启状态．才可进行测试！"));  //可以放到 调用处触发．
							OnCancel();	
							return;
						}
					}
					else
					{//等待

					}

				}
				else
				{
					goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
				}
			}			
		}
		else
		{
			goto LABEL_TIMER_PLC_COMM_ERROR_RETURN;
		}		

		if((iTemp >= ( m_dwMaxWaitingTime + m_dwPressureKeepTime)) || (iTemp>= m_dwMaxWaitingTime  && iKeepTime <= 0) )
		{//超时则退出.
			m_nRetValue = _DLG_INIT_VAC_TST_RT_OVERTIME;
			KillTimer(m_unCurrentTimeEvent);
			m_unCurrentTimeEvent = 0;

			AfxMessageBox(g_item2_String[_ITEM2_STR_WAITTING_TIME_OVER].IsEmpty()?
				TEXT("错误: 等待超时！\r\nError:Waiting time over!"):g_item2_String[_ITEM2_STR_WAITTING_TIME_OVER]);
			//AfxMessageBox(TEXT("Exit: 等待超时！"));
			OnCancel();
			return;
		}

		//始终显示当前的过去时间
		_stprintf(buf,TEXT("%02d:%02d:%02d"),iTemp/3600,(iTemp/60)%60 ,iTemp%60);
		SetDlgItemText(IDC_STATIC_ELAPSED_TIME,buf);

		_stprintf(buf,TEXT("%02d:%02d:%02d"),iKeepTime/3600,(iKeepTime/60)%60 ,iKeepTime%60);
		SetDlgItemText(IDC_STATIC_PRESSURE_KEEPED_TIME,buf);

		//显示当前真空度
		_stprintf(buf,TEXT("%.2E Pa"),dPressure);
		SetDlgItemText(IDC_STATIC_CUR_PRESSURE,buf);

	}
	CDialog::OnTimer(nIDEvent);
	return;

LABEL_TIMER_PLC_COMM_ERROR_RETURN:
	//没有通信，提示并退出！
	m_nRetValue =_DLG_INIT_VAC_TST_RT_COMM_ERR;
	if(m_unCurrentTimeEvent)
		KillTimer(m_unCurrentTimeEvent);
	m_unCurrentTimeEvent = 0;
	AfxMessageBox(g_item2_String[_ITEM2_STR_PLC_COMM_ERR].IsEmpty()?
				TEXT("plc通信不正常！\r\n plc communication failed."):g_item2_String[_ITEM2_STR_PLC_COMM_ERR]);
	//AfxMessageBox(TEXT("Exit: plc通信不正常！"));  //可以放到 调用处触发．
	OnCancel();	
	return;
}
