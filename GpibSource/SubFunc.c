// ************************************************************ [CheckRet] function ***
long CheckRet(char *szFunc, long Ret, char *szBuf)
{
	long	RetCode,Ret1;
	
	RetCode = 0;				// Normality
	Ret1 = Ret & 0xff;			// 0xFF Masking
	if(Ret1 >= 3){				// Unsual
		RetCode = 1;			// Return of function(Unsual)
		switch(Ret1){
		case 3:
			wsprintf(szBuf, "%s%s", szFunc, " : Data has been left within FIFO.");
			break;
		case 80:
			wsprintf(szBuf, "%s%s", szFunc, " : It is I/O address error.");
			break;
		case 82:
			wsprintf(szBuf, "%s%s", szFunc, " : Resistry setup is wrong. Review resistry setup.");
			break;
		case 128:
			wsprintf(szBuf, "%s%s", szFunc, " : Data receiving was exceeded.and polling is not.");
			break;
		case 200:
			wsprintf(szBuf, "%s%s", szFunc, " : Thread can not be made.");
			break;
		case 201:
			wsprintf(szBuf, "%s%s", szFunc, " : Other event is in practice.");
			break;
		case 210:
			wsprintf(szBuf, "%s%s", szFunc, " : DMA could not be established.");
			break;
		case 240:
			wsprintf(szBuf, "%s%s", szFunc, " : [Esc] key was pushed.");
			break;
		case 241:
			wsprintf(szBuf, "%s%s", szFunc, " : It is an I/O error of file.");
			break;
		case 242:
			wsprintf(szBuf, "%s%s", szFunc, " : Address appoint is wrong.");
			break;
		case 243:
			wsprintf(szBuf, "%s%s", szFunc, " : Buffer Error.");
			break;
		case 244:
			wsprintf(szBuf, "%s%s", szFunc, " : Error of array size.");
			break;
		case 245:
			wsprintf(szBuf, "%s%s", szFunc, " : A buffer is too small.");
			break;
		case 246:
			wsprintf(szBuf, "%s%s", szFunc, " : It is an unjust object name.");
			break;
		case 247:
			wsprintf(szBuf, "%s%s", szFunc, " : A side check of a device name is invalid.");
			break;
		case 248:
			wsprintf(szBuf, "%s%s", szFunc, " : It is an unjust data model.");
			break;
		case 249:
			wsprintf(szBuf, "%s%s", szFunc, " : Device can not be added further.");
			break;
		case 250:
			wsprintf(szBuf, "%s%s", szFunc, " : A device name is not found out.");
			break;
		case 251:
			wsprintf(szBuf, "%s%s", szFunc, " : Delimiter is wrong between device.");
			break;
		case 252:
			wsprintf(szBuf, "%s%s", szFunc, " : It is an error of GP-IB.");
			break;
		case 253:
			wsprintf(szBuf, "%s%s", szFunc, " : Only a delimiter was receiving.");
			break;
		case 254:
			wsprintf(szBuf, "%s%s", szFunc, " : It was done a time out.");
			break;
		case 255:
			wsprintf(szBuf, "%s%s", szFunc, " : It is a parameter error.");
			break;
		}
	}else{
		wsprintf(szBuf, "%s%s", szFunc, " : It was terminated normally.");
	}

// It was message when [Ifc] [Srq] received. //
    Ret1 = Ret & 0xff00;					// 0xFF Masking
	switch(Ret1){
	case 0x100:						// 10 -> [256]
		wsprintf(szBuf, "%s%s", szFunc, "-[SRQ] was receiving.<STATUS>");
		break;
	case 0x200:						// 10 -> [512]
		wsprintf(szBuf, "%s%s", szFunc, "-[IFC] was receiving.<STATUS>");
		break;
	case 0x300:						// 10 -> [768]
		wsprintf(szBuf, "%s%s", szFunc, "-[SRQ],[IFC] was receiving.<STATUS>");
		break;
	}
	return	RetCode;
}

//********************************************************* [Square] Function ***
int Square(int x, int y)
{
	int tmp = 1;

	while (y-- > 0)						// Repeat counts
		tmp *= x;					// Multiply
	return(tmp);
}

//**************************************************** [Chr -> Hex] Function ***
DWORD chr2hex(char* ch)
{
	DWORD	ret = 0, rettmp = 0;
	long	count, length;

	length	= lstrlen(ch);					// Search length of ch
	for(count = 0;count < length ;count++){			// Repeat
		if((ch[count] >= 0x30) && (ch[count] <=0x39))	// Get ASCII code to number
		  ret = (ch[count] - 0x30) * Square(0x10,(length - (count + 1)));	// 0 - 9 
		else if((ch[count] >= 0x41) && (ch[count] <= 0x46))
		  ret = (ch[count] - 0x37) * Square(0x10,(length - (count + 1)));	// A - F 
		else if((ch[count] >= 0x61) && (ch[count] <= 0x66))
		  ret = (ch[count] - 0x57) * Square(0x10,(length - (count + 1)));	// a - f 
		else
		  ret = 0xff;					// Return is wrong when ret = 0xFF
	
		rettmp = rettmp + ret;
	}
	return rettmp;
}

//**************************************************** [Initialize] function ***
long GpibInit(char *szTextRet)					// Normal = TRUE,Abnormal = FALSE
{
	//int	Delim,Eoi;						// Delimita,EOI
	int Timeout,Ifctime,Ret;			// TimeOut,IFC-Time,Return value
	DWORD	Master;						// Get Mode

	Ret = GpExit();						// Keep off initialize repeat
	Ret = GpIni();						// GPIB initialize
	if((Ret & 0xFF) != 0){					// Check of GpIni function
		CheckRet("GpIni", Ret, szTextRet);
		return	1;
	}

	GpBoardsts(0x0a, &Master);				// Get Master/Slave mode
	
	if(Master == 0){
		Ifctime = 1;					// Default
		Ret = GpIfc(Ifctime);
		if((Ret & 0xFF) != 0){				// Check of GpIfc function
			CheckRet("GpIfc", Ret, szTextRet);
			return	1;
		}
		Ret = GpRen();
		if((Ret & 0xFF) != 0){				// Check of GpRen function
			CheckRet("GpRen", Ret, szTextRet);
			return	1;
		}
	}

	/*Delim = 1;						// Default = CR+LF
	Eoi = 1;						// Default = Use
	Ret = GpDelim(Delim, Eoi);					
	if((Ret & 0xFF) != 0){					// Check of GpDelim
		CheckRet("GpDelim", Ret, szTextRet);
		return	1;
	}*/
	Timeout = 10000;					// 10sec
	Ret = GpTimeout(Timeout);
	if((Ret & 0xFF) != 0){					// Check of GpTimeout fucntion
		CheckRet("GpTimeout", Ret, szTextRet);
		return	1;
	}

	lstrcpy(szTextRet, "Initialized.");			// Normality Ended.
	return	0;
}

//******************************************************* [Exit] function ***
void GpibExit()
{
	DWORD	Master;						// M/S mode
	long	Ret;
	DWORD	Cmd[32];
	
	Ret = GpBoardsts(0x0a, &Master);			// Get mode
	if(Ret == 80) return;					// If found error then not doing
	
	if(Master == 0){					// When mode is Master
		Cmd[0] = 2;					// Command count
		Cmd[1] = 0x3f;					// Unlisten / UNL
		Cmd[2] = 0x5f;					// Untalken / UNT
		Ret = GpComand(Cmd);				// Send command
	}
		Ret = GpResetren();				// Cancel remote
	
	Ret = GpExit();
}

//**************************************************** [Str -> Num] ***
void Str2Num(char *str, DWORD str_len , int *num, DWORD num_len)
{
	DWORD i, cnt;
	char *start;

	start = str;
	cnt = 0;
	for (i=0; i<str_len; i++) {
		/* string to integer */
		if (str[i] == ',') {
			str[i] = '\0';
			num[cnt] = atoi(start);
			str[i] = ',';
			start = &str[i+1];
			cnt++;
			if (cnt >= num_len) break;
		}
	}
	if (cnt >= num_len) {
		num[cnt] = atoi(start);
	}
}

//****************************************************** [Draw graph] function ***
// Use picture dialog
void DrawGraph(HWND hDlg, DWORD Picture, int *num, DWORD num_len, int min, int max)
{
	HWND	Disp_handle;
	RECT	Rect;
	HDC		hDC;
	HPEN	hPen_Black, hPen_Red, hPen_White;
	HBRUSH	hBrush1, hBrush2, hBrush3;
	POINT	Point[4];
	int		x_max, y_max;
	int		x_width, y_width;
	float	x_unit, y_unit;
	DWORD	i;

	/* Initialize */
	Disp_handle = GetDlgItem(hDlg, Picture);
	GetClientRect(Disp_handle, &Rect);
	x_max = Rect.right;
	y_max = Rect.bottom;
	hDC = GetDC(Disp_handle);

	hPen_Black = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));	/* Black */
	hPen_Red   = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));	/* Red   */
	hPen_White = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));/* White */	
	/* Draw Structure */
	hBrush3 = (HBRUSH)SelectObject(hDC, hPen_White);
	Rectangle(hDC, Rect.left, Rect.top, Rect.right, Rect.bottom);
	hBrush1 = (HBRUSH)SelectObject(hDC, hPen_Black);
	for (i=0; i<=10; i++) {
		MoveToEx(hDC, (x_max / 10) * i, 0, NULL);
		LineTo(  hDC, (x_max / 10) * i, y_max);
	}
	for (i=0; i<=10; i++) {
		MoveToEx(hDC, 0,   (y_max / 10) * i, NULL);
		LineTo(hDC, x_max, (y_max / 10) * i);
	}
	hBrush2 = (HBRUSH)SelectObject(hDC, hPen_Red);
	Point[0].x = (x_max / 10) * 5 - 1; Point[0].y = 0;
	Point[1].x = (x_max / 10) * 5 + 1; Point[1].y = 0;
	Point[2].x = (x_max / 10) * 5 + 1; Point[2].y = y_max;
	Point[3].x = (x_max / 10) * 5 - 1; Point[3].y = y_max;
	Polyline(hDC, &Point[0], 4);
	Point[0].x = 0;     Point[0].y = (y_max / 10) * 5 - 1;
	Point[1].x = 0;     Point[1].y = (y_max / 10) * 5 + 1;
	Point[2].x = x_max; Point[2].y = (y_max / 10) * 5 + 1;
	Point[3].x = x_max; Point[3].y = (y_max / 10) * 5 - 1;
	Polyline(hDC, &Point[0], 4);
	/* Draw Graph */
	hBrush1 = (HBRUSH)SelectObject(hDC, hPen_Black);
	x_width = num_len;
	y_width = max - min;
	x_unit = (float)((float)x_max / (float)x_width);
	y_unit = (float)((float)y_max / (float)y_width);
	for (i=0; i<num_len-1; i++) {
		MoveToEx(hDC, (int)(x_unit * i)      , (int)((y_width - (num[i] - min)) * y_unit) , NULL);
		LineTo(  hDC, (int)(x_unit * (i + 1)), (int)((y_width - (num[i+1] - min)) * y_unit));
	}
	/* ending */
	SelectObject(hDC, hBrush1);
	DeleteObject(hPen_Black);
	SelectObject(hDC, hBrush2);
	DeleteObject(hPen_Red);
	SelectObject(hDC, hBrush3);
	DeleteObject(hPen_White);
	ReleaseDC(Disp_handle, hDC);
}


long GpibPrint(long DevAddr, char *Str)
{
char	srbuf[10000], ErrText[255];
DWORD	MyAddr, Cmd[16];
long	Ret, RetTmp, srlen;

	Ret = GpBoardsts(0x08, &MyAddr);
	Cmd[0] = 2;
	Cmd[1] = MyAddr;
	Cmd[2] = DevAddr;

	srlen = lstrlen((LPCTSTR)&Str[0]);
	lstrcpy(srbuf, (LPCTSTR)&Str[0]);
    Ret = GpTalk(Cmd, srlen, srbuf);

	if (Ret >= 3){
		RetTmp = CheckRet("GpTalk", Ret, ErrText);
		Ret = MessageBox(NULL, ErrText, "Continue?", MB_YESNO);
		if (Ret == IDNO) return 1;
	}
	return 0;
}

long GpibInput(long DevAddr, char *Str)
{
BYTE	srbuf[10000];
DWORD	MyAddr,srlen, Cmd[16];
char	ErrText[255];
long	Ret, RetTmp;

   	memset( srbuf, '\0', 10000 );

    Ret = GpBoardsts(0x08, &MyAddr);
	Cmd[0] = 2;
	Cmd[1] = DevAddr;
	Cmd[2] = MyAddr;
	srlen = sizeof(srbuf);
    Ret = GpListen(Cmd, &srlen, srbuf);

	if (Ret >= 3){
		RetTmp = CheckRet("GpListen", Ret, ErrText);
		Ret = MessageBox(NULL, ErrText, "Continue?", MB_YESNO);
		if (Ret == IDNO) return 1;
	}
	lstrcpy(Str, &srbuf[0]);
	return 0;
}

long GpibInputB(long DevAddr, BYTE *IntData)
{
BYTE	szData[10000];
DWORD	Ret, MyAddr, Cmd[8], srlen;
char	ErrText[255];
long	RetTmp;

	memset(szData, '\0', 10000);

	Ret = GpDelim(0, 1);
    Ret = GpBoardsts(0x08, &MyAddr);
	Cmd[0] = 2;
	Cmd[1] = DevAddr;
	Cmd[2] = MyAddr;
	srlen = 2;
	Ret = GpListen(Cmd, &srlen, szData);
	Cmd[0] = 0;
	srlen = atoi((LPCTSTR)&szData[1]);
	Ret = GpListen(Cmd, &srlen, szData);
	srlen = (atoi((LPCTSTR)&szData)) + 1;
	Ret = GpListen(Cmd, &srlen, IntData);
	if (Ret >= 3){
		RetTmp = CheckRet("GpListen", Ret, ErrText);
		Ret = MessageBox(NULL, ErrText, "Continue?", MB_YESNO);
		if (Ret == IDNO) return 1;
	}
	Ret = GpDelim(3, 1);
	return 0;
}

long GpibCommand(long DevAddr)
{
DWORD	Cmd[16];
char	ErrText[255];
long	Ret, RetTmp;

	Cmd[0] = 2;
	Cmd[1] = 0x3F;
	Cmd[2] = 0x5F;

	Ret = GpComand(Cmd);

	if (Ret != 0){
		RetTmp = CheckRet("GpComand", Ret, ErrText);
		MessageBox(NULL, ErrText, NULL, MB_OK);
		return 1;
	}
	return 0;
}

void WaitOPC(long Dev)
{
	long	Ret;
	char RdData[255];

	Ret = GpibPrint(Dev, "*OPC?");
	Ret = GpibInput(Dev, RdData);
}
