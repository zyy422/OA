/***********************************************************************/
/*          API-GPIB(98/PC)W95,NT                 Header  File         */
/*                                                File Name SUBFUNC.H */
/***********************************************************************/

#ifndef __SUBFUNC_H__
#define __SUBFUNC_H__

long CheckRet(char *Func, long Ret, char *csBuf);
int Square(int x, int y);
DWORD chr2hex(char *ch);
long GpibInit(char *TextRet);
long GpibPrint(long DevAddr, char *TextRet);
long GpibInput(long DevAddr, char *TextRet);
long GpibInputB(long DevAddr, BYTE *IntData);
long GpibCommand(char *TextRet);
void GpibExit(void);
void WaitOPC(long Dev);
void Str2Num(char *str, DWORD str_len, int *num, DWORD num_len);
void DrawGraph(HWND hDlg, DWORD Picture, int *num, DWORD num_len, int min, int max);

#endif
