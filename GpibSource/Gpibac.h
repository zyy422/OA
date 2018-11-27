/***********************************************************************/
/*          API-GPIB(98/PC)W95,NT                 Header  File         */
/*                                                File Name GPIBAC.H   */
/***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif 

DWORD WINAPI GpIni(void);
DWORD WINAPI GpIfc(DWORD);
DWORD WINAPI GpRen(void);
DWORD WINAPI GpResetren(void);
DWORD WINAPI GpTalk(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpListen(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpPoll(DWORD *, DWORD *);
DWORD WINAPI GpSrq(DWORD);
DWORD WINAPI GpStb(DWORD);
DWORD WINAPI GpDelim(DWORD, DWORD);
DWORD WINAPI GpTimeout(DWORD);
DWORD WINAPI GpChkstb(DWORD *, DWORD *);
DWORD WINAPI GpReadreg(DWORD, DWORD *);
DWORD WINAPI GpDma(DWORD, DWORD);
DWORD WINAPI GpExit(void);
DWORD WINAPI GpComand(DWORD *);
DWORD WINAPI GpDmainuse(void);
DWORD WINAPI GpStstop(DWORD);
DWORD WINAPI GpDmastop(void);
DWORD WINAPI GpPpollmode(DWORD);
DWORD WINAPI GpStppoll(DWORD *, DWORD);
DWORD WINAPI GpExppoll(DWORD *);
DWORD WINAPI GpStwait(DWORD);
DWORD WINAPI GpWaittime(DWORD);
DWORD WINAPI GpReadbus(DWORD *);
DWORD WINAPI GpSfile(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpRfile(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpSdc(DWORD);
DWORD WINAPI GpDcl(void);
DWORD WINAPI GpGet(DWORD);
DWORD WINAPI GpGtl(DWORD);
DWORD WINAPI GpLlo(void);
DWORD WINAPI GpTct(DWORD);
DWORD WINAPI GpCrst(DWORD);
DWORD WINAPI GpCopc(DWORD);
DWORD WINAPI GpCwai(DWORD);
DWORD WINAPI GpCcls(DWORD);
DWORD WINAPI GpCtrg(DWORD);
DWORD WINAPI GpCpre(DWORD, DWORD);
DWORD WINAPI GpCese(DWORD, DWORD);
DWORD WINAPI GpCsre(DWORD, DWORD);
DWORD WINAPI GpQidn(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopt(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpud(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQrdt(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQcal(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlrn(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQtst(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopc(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQemc(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQgmc(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlmc(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQist(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpre(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQese(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQesr(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpsc(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQsre(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQstb(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQddt(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpTaLaBit(DWORD);
DWORD WINAPI GpBoardsts(DWORD, DWORD *);
DWORD WINAPI GpSrqEvent(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqEventEx(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqOn();
DWORD WINAPI GpDevFind(DWORD *);
DWORD WINAPI GpInpB(WORD);
DWORD WINAPI GpInpW(WORD);
DWORD WINAPI GpInpD(WORD);
DWORD WINAPI GpOutB(WORD, BYTE);
DWORD WINAPI GpOutW(WORD, WORD);
DWORD WINAPI GpOutD(WORD, DWORD);
DWORD WINAPI GpSetEvent(DWORD);
DWORD WINAPI GpSetEventSrq(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDet(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDec(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventIfc(HANDLE, WORD, DWORD);
DWORD WINAPI GpEnableNextEvent(void);
DWORD WINAPI GpSrqEx(DWORD, DWORD, DWORD);
DWORD WINAPI GpUpperCode(DWORD);
DWORD WINAPI GpCnvSettings(LPBYTE, LPBYTE, LPBYTE, DWORD);
DWORD WINAPI GpCnvSettingsToStr(DWORD, DWORD, DWORD);
DWORD WINAPI GpCnvStrToDbl(LPBYTE, double*);
DWORD WINAPI GpCnvStrToDblArray(LPBYTE, double*, LPDWORD);
DWORD WINAPI GpCnvStrToFlt(LPBYTE, float*);
DWORD WINAPI GpCnvStrToFltArray(LPBYTE, float*, LPDWORD);
DWORD WINAPI GpCnvDblToStr(LPBYTE, LPDWORD, double);
DWORD WINAPI GpCnvDblArrayToStr(LPBYTE, LPDWORD, double*, DWORD);
DWORD WINAPI GpCnvFltToStr(LPBYTE, LPDWORD, float);
DWORD WINAPI GpCnvFltArrayToStr(LPBYTE, LPDWORD, float*, DWORD);
DWORD WINAPI GpPollEx(LPDWORD, LPDWORD, LPDWORD);
DWORD WINAPI GpSlowMode(DWORD);
DWORD WINAPI GpCnvCvSettings(DWORD);
DWORD WINAPI GpCnvCvi(LPBYTE, short*);
DWORD WINAPI GpCnvCviArray(LPBYTE, short*, DWORD);
DWORD WINAPI GpCnvCvs(LPBYTE, float*);
DWORD WINAPI GpCnvCvsArray(LPBYTE, float*, DWORD);
DWORD WINAPI GpCnvCvd(LPBYTE, double*);
DWORD WINAPI GpCnvCvdArray(LPBYTE, double*, DWORD);
DWORD WINAPI GpTalkEx(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenEx(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpTalkAsync(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenAsync(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpCommandAsync(DWORD *);
DWORD WINAPI GpCheckAsync(DWORD, DWORD*);
DWORD WINAPI GpStopAsync(void);
DWORD WINAPI GpDevFindEx(short, short, short *);
DWORD WINAPI GpBoardstsEx(DWORD, DWORD, DWORD *);
DWORD WINAPI GpSmoothMode(DWORD);

DWORD WINAPI GpIni2(void);
DWORD WINAPI GpIfc2(DWORD);
DWORD WINAPI GpRen2(void);
DWORD WINAPI GpResetren2(void);
DWORD WINAPI GpTalk2(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpListen2(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpPoll2(DWORD *, DWORD *);
DWORD WINAPI GpSrq2(DWORD);
DWORD WINAPI GpStb2(DWORD);
DWORD WINAPI GpDelim2(DWORD, DWORD);
DWORD WINAPI GpTimeout2(DWORD);
DWORD WINAPI GpChkstb2(DWORD *, DWORD *);
DWORD WINAPI GpReadreg2(DWORD, DWORD *);
DWORD WINAPI GpDma2(DWORD, DWORD);
DWORD WINAPI GpExit2(void);
DWORD WINAPI GpComand2(DWORD *);
DWORD WINAPI GpDmainuse2(void);
DWORD WINAPI GpStstop2(DWORD);
DWORD WINAPI GpDmastop2(void);
DWORD WINAPI GpPpollmode2(DWORD);
DWORD WINAPI GpStppoll2(DWORD *, DWORD);
DWORD WINAPI GpExppoll2(DWORD *);
DWORD WINAPI GpStwait2(DWORD);
DWORD WINAPI GpWaittime2(DWORD);
DWORD WINAPI GpReadbus2(DWORD *);
DWORD WINAPI GpSfile2(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpRfile2(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpSdc2(DWORD);
DWORD WINAPI GpDcl2(void);
DWORD WINAPI GpGet2(DWORD);
DWORD WINAPI GpGtl2(DWORD);
DWORD WINAPI GpLlo2(void);
DWORD WINAPI GpTct2(DWORD);
DWORD WINAPI GpCrst2(DWORD);
DWORD WINAPI GpCopc2(DWORD);
DWORD WINAPI GpCwai2(DWORD);
DWORD WINAPI GpCcls2(DWORD);
DWORD WINAPI GpCtrg2(DWORD);
DWORD WINAPI GpCpre2(DWORD, DWORD);
DWORD WINAPI GpCese2(DWORD, DWORD);
DWORD WINAPI GpCsre2(DWORD, DWORD);
DWORD WINAPI GpQidn2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopt2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpud2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQrdt2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQcal2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlrn2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQtst2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopc2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQemc2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQgmc2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlmc2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQist2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpre2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQese2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQesr2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpsc2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQsre2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQstb2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQddt2(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpTaLaBit2(DWORD);
DWORD WINAPI GpBoardsts2(DWORD, DWORD *);
DWORD WINAPI GpSrqEvent2(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqEventEx2(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqOn2();
DWORD WINAPI GpDevFind2(DWORD *);
DWORD WINAPI GpInpB2(WORD);
DWORD WINAPI GpInpW2(WORD);
DWORD WINAPI GpInpD2(WORD);
DWORD WINAPI GpOutB2(WORD, BYTE);
DWORD WINAPI GpOutW2(WORD, WORD);
DWORD WINAPI GpOutD2(WORD, DWORD);
DWORD WINAPI GpSetEvent2(DWORD);
DWORD WINAPI GpSetEventSrq2(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDet2(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDec2(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventIfc2(HANDLE, WORD, DWORD);
DWORD WINAPI GpEnableNextEvent2(void);
DWORD WINAPI GpSrqEx2(DWORD, DWORD, DWORD);
DWORD WINAPI GpUpperCode2(DWORD);
DWORD WINAPI GpCnvSettings2(LPBYTE, LPBYTE, LPBYTE, DWORD);
DWORD WINAPI GpCnvSettingsToStr2(DWORD, DWORD, DWORD);
DWORD WINAPI GpCnvStrToDbl2(LPBYTE, double*);
DWORD WINAPI GpCnvStrToDblArray2(LPBYTE, double*, LPDWORD);
DWORD WINAPI GpCnvStrToFlt2(LPBYTE, float*);
DWORD WINAPI GpCnvStrToFltArray2(LPBYTE, float*, LPDWORD);
DWORD WINAPI GpCnvDblToStr2(LPBYTE, LPDWORD, double);
DWORD WINAPI GpCnvDblArrayToStr2(LPBYTE, LPDWORD, double*, DWORD);
DWORD WINAPI GpCnvFltToStr2(LPBYTE, LPDWORD, float);
DWORD WINAPI GpCnvFltArrayToStr2(LPBYTE, LPDWORD, float*, DWORD);
DWORD WINAPI GpPollEx2(LPDWORD, LPDWORD, LPDWORD);
DWORD WINAPI GpSlowMode2(DWORD);
DWORD WINAPI GpCnvCvSettings2(DWORD);
DWORD WINAPI GpCnvCvi2(LPBYTE, short*);
DWORD WINAPI GpCnvCviArray2(LPBYTE, short*, DWORD);
DWORD WINAPI GpCnvCvs2(LPBYTE, float*);
DWORD WINAPI GpCnvCvsArray2(LPBYTE, float*, DWORD);
DWORD WINAPI GpCnvCvd2(LPBYTE, double*);
DWORD WINAPI GpCnvCvdArray2(LPBYTE, double*, DWORD);
DWORD WINAPI GpTalkEx2(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenEx2(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpTalkAsync2(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenAsync2(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpCommandAsync2(DWORD *);
DWORD WINAPI GpCheckAsync2(DWORD, DWORD*);
DWORD WINAPI GpStopAsync2(void);
DWORD WINAPI GpDevFindEx2(short, short, short *);
DWORD WINAPI GpBoardstsEx2(DWORD, DWORD, DWORD *);
DWORD WINAPI GpSmoothMode2(DWORD);

DWORD WINAPI GpIni3(void);
DWORD WINAPI GpIfc3(DWORD);
DWORD WINAPI GpRen3(void);
DWORD WINAPI GpResetren3(void);
DWORD WINAPI GpTalk3(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpListen3(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpPoll3(DWORD *, DWORD *);
DWORD WINAPI GpSrq3(DWORD);
DWORD WINAPI GpStb3(DWORD);
DWORD WINAPI GpDelim3(DWORD, DWORD);
DWORD WINAPI GpTimeout3(DWORD);
DWORD WINAPI GpChkstb3(DWORD *, DWORD *);
DWORD WINAPI GpReadreg3(DWORD, DWORD *);
DWORD WINAPI GpDma3(DWORD, DWORD);
DWORD WINAPI GpExit3(void);
DWORD WINAPI GpComand3(DWORD *);
DWORD WINAPI GpDmainuse3(void);
DWORD WINAPI GpStstop3(DWORD);
DWORD WINAPI GpDmastop3(void);
DWORD WINAPI GpPpollmode3(DWORD);
DWORD WINAPI GpStppoll3(DWORD *, DWORD);
DWORD WINAPI GpExppoll3(DWORD *);
DWORD WINAPI GpStwait3(DWORD);
DWORD WINAPI GpWaittime3(DWORD);
DWORD WINAPI GpReadbus3(DWORD *);
DWORD WINAPI GpSfile3(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpRfile3(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpSdc3(DWORD);
DWORD WINAPI GpDcl3(void);
DWORD WINAPI GpGet3(DWORD);
DWORD WINAPI GpGtl3(DWORD);
DWORD WINAPI GpLlo3(void);
DWORD WINAPI GpTct3(DWORD);
DWORD WINAPI GpCrst3(DWORD);
DWORD WINAPI GpCopc3(DWORD);
DWORD WINAPI GpCwai3(DWORD);
DWORD WINAPI GpCcls3(DWORD);
DWORD WINAPI GpCtrg3(DWORD);
DWORD WINAPI GpCpre3(DWORD, DWORD);
DWORD WINAPI GpCese3(DWORD, DWORD);
DWORD WINAPI GpCsre3(DWORD, DWORD);
DWORD WINAPI GpQidn3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopt3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpud3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQrdt3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQcal3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlrn3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQtst3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopc3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQemc3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQgmc3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlmc3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQist3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpre3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQese3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQesr3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpsc3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQsre3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQstb3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQddt3(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpTaLaBit3(DWORD);
DWORD WINAPI GpBoardsts3(DWORD, DWORD *);
DWORD WINAPI GpSrqEvent3(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqEventEx3(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqOn3();
DWORD WINAPI GpDevFind3(DWORD *);
DWORD WINAPI GpInpB3(WORD);
DWORD WINAPI GpInpW3(WORD);
DWORD WINAPI GpInpD3(WORD);
DWORD WINAPI GpOutB3(WORD, BYTE);
DWORD WINAPI GpOutW3(WORD, WORD);
DWORD WINAPI GpOutD3(WORD, DWORD);
DWORD WINAPI GpSetEvent3(DWORD);
DWORD WINAPI GpSetEventSrq3(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDet3(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDec3(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventIfc3(HANDLE, WORD, DWORD);
DWORD WINAPI GpEnableNextEvent3(void);
DWORD WINAPI GpSrqEx3(DWORD, DWORD, DWORD);
DWORD WINAPI GpUpperCode3(DWORD);
DWORD WINAPI GpCnvSettings3(LPBYTE, LPBYTE, LPBYTE, DWORD);
DWORD WINAPI GpCnvSettingsToStr3(DWORD, DWORD, DWORD);
DWORD WINAPI GpCnvStrToDbl3(LPBYTE, double*);
DWORD WINAPI GpCnvStrToDblArray3(LPBYTE, double*, LPDWORD);
DWORD WINAPI GpCnvStrToFlt3(LPBYTE, float*);
DWORD WINAPI GpCnvStrToFltArray3(LPBYTE, float*, LPDWORD);
DWORD WINAPI GpCnvDblToStr3(LPBYTE, LPDWORD, double);
DWORD WINAPI GpCnvDblArrayToStr3(LPBYTE, LPDWORD, double*, DWORD);
DWORD WINAPI GpCnvFltToStr3(LPBYTE, LPDWORD, float);
DWORD WINAPI GpCnvFltArrayToStr3(LPBYTE, LPDWORD, float*, DWORD);
DWORD WINAPI GpPollEx3(LPDWORD, LPDWORD, LPDWORD);
DWORD WINAPI GpSlowMode3(DWORD);
DWORD WINAPI GpCnvCvSettings3(DWORD);
DWORD WINAPI GpCnvCvi3(LPBYTE, short*);
DWORD WINAPI GpCnvCviArray3(LPBYTE, short*, DWORD);
DWORD WINAPI GpCnvCvs3(LPBYTE, float*);
DWORD WINAPI GpCnvCvsArray3(LPBYTE, float*, DWORD);
DWORD WINAPI GpCnvCvd3(LPBYTE, double*);
DWORD WINAPI GpCnvCvdArray3(LPBYTE, double*, DWORD);
DWORD WINAPI GpTalkEx3(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenEx3(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpTalkAsync3(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenAsync3(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpCommandAsync3(DWORD *);
DWORD WINAPI GpCheckAsync3(DWORD, DWORD*);
DWORD WINAPI GpStopAsync3(void);
DWORD WINAPI GpDevFindEx3(short, short, short *);
DWORD WINAPI GpBoardstsEx3(DWORD, DWORD, DWORD *);
DWORD WINAPI GpSmoothMode3(DWORD);

DWORD WINAPI GpIni4(void);
DWORD WINAPI GpIfc4(DWORD);
DWORD WINAPI GpRen4(void);
DWORD WINAPI GpResetren4(void);
DWORD WINAPI GpTalk4(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpListen4(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpPoll4(DWORD *, DWORD *);
DWORD WINAPI GpSrq4(DWORD);
DWORD WINAPI GpStb4(DWORD);
DWORD WINAPI GpDelim4(DWORD, DWORD);
DWORD WINAPI GpTimeout4(DWORD);
DWORD WINAPI GpChkstb4(DWORD *, DWORD *);
DWORD WINAPI GpReadreg4(DWORD, DWORD *);
DWORD WINAPI GpDma4(DWORD, DWORD);
DWORD WINAPI GpExit4(void);
DWORD WINAPI GpComand4(DWORD *);
DWORD WINAPI GpDmainuse4(void);
DWORD WINAPI GpStstop4(DWORD);
DWORD WINAPI GpDmastop4(void);
DWORD WINAPI GpPpollmode4(DWORD);
DWORD WINAPI GpStppoll4(DWORD *, DWORD);
DWORD WINAPI GpExppoll4(DWORD *);
DWORD WINAPI GpStwait4(DWORD);
DWORD WINAPI GpWaittime4(DWORD);
DWORD WINAPI GpReadbus4(DWORD *);
DWORD WINAPI GpSfile4(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpRfile4(DWORD *, DWORD, BYTE *);
DWORD WINAPI GpSdc4(DWORD);
DWORD WINAPI GpDcl4(void);
DWORD WINAPI GpGet4(DWORD);
DWORD WINAPI GpGtl4(DWORD);
DWORD WINAPI GpLlo4(void);
DWORD WINAPI GpTct4(DWORD);
DWORD WINAPI GpCrst4(DWORD);
DWORD WINAPI GpCopc4(DWORD);
DWORD WINAPI GpCwai4(DWORD);
DWORD WINAPI GpCcls4(DWORD);
DWORD WINAPI GpCtrg4(DWORD);
DWORD WINAPI GpCpre4(DWORD, DWORD);
DWORD WINAPI GpCese4(DWORD, DWORD);
DWORD WINAPI GpCsre4(DWORD, DWORD);
DWORD WINAPI GpQidn4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopt4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpud4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQrdt4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQcal4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlrn4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQtst4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQopc4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQemc4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQgmc4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQlmc4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQist4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpre4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQese4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQesr4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQpsc4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQsre4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQstb4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpQddt4(DWORD, DWORD *, BYTE *);
DWORD WINAPI GpTaLaBit4(DWORD);
DWORD WINAPI GpBoardsts4(DWORD, DWORD *);
DWORD WINAPI GpSrqEvent4(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqEventEx4(HANDLE, WORD, DWORD);
DWORD WINAPI GpSrqOn4();
DWORD WINAPI GpDevFind4(DWORD *);
DWORD WINAPI GpInpB4(WORD);
DWORD WINAPI GpInpW4(WORD);
DWORD WINAPI GpInpD4(WORD);
DWORD WINAPI GpOutB4(WORD, BYTE);
DWORD WINAPI GpOutW4(WORD, WORD);
DWORD WINAPI GpOutD4(WORD, DWORD);
DWORD WINAPI GpSetEvent4(DWORD);
DWORD WINAPI GpSetEventSrq4(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDet4(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventDec4(HANDLE, WORD, DWORD);
DWORD WINAPI GpSetEventIfc4(HANDLE, WORD, DWORD);
DWORD WINAPI GpEnableNextEvent4(void);
DWORD WINAPI GpSrqEx4(DWORD, DWORD, DWORD);
DWORD WINAPI GpUpperCode4(DWORD);
DWORD WINAPI GpCnvSettings4(LPBYTE, LPBYTE, LPBYTE, DWORD);
DWORD WINAPI GpCnvSettingsToStr4(DWORD, DWORD, DWORD);
DWORD WINAPI GpCnvStrToDbl4(LPBYTE, double*);
DWORD WINAPI GpCnvStrToDblArray4(LPBYTE, double*, LPDWORD);
DWORD WINAPI GpCnvStrToFlt4(LPBYTE, float*);
DWORD WINAPI GpCnvStrToFltArray4(LPBYTE, float*, LPDWORD);
DWORD WINAPI GpCnvDblToStr4(LPBYTE, LPDWORD, double);
DWORD WINAPI GpCnvDblArrayToStr4(LPBYTE, LPDWORD, double*, DWORD);
DWORD WINAPI GpCnvFltToStr4(LPBYTE, LPDWORD, float);
DWORD WINAPI GpCnvFltArrayToStr4(LPBYTE, LPDWORD, float*, DWORD);
DWORD WINAPI GpPollEx4(LPDWORD, LPDWORD, LPDWORD);
DWORD WINAPI GpSlowMode4(DWORD);
DWORD WINAPI GpCnvCvSettings4(DWORD);
DWORD WINAPI GpCnvCvi4(LPBYTE, short*);
DWORD WINAPI GpCnvCviArray4(LPBYTE, short*, DWORD);
DWORD WINAPI GpCnvCvs4(LPBYTE, float*);
DWORD WINAPI GpCnvCvsArray4(LPBYTE, float*, DWORD);
DWORD WINAPI GpCnvCvd4(LPBYTE, double*);
DWORD WINAPI GpCnvCvdArray4(LPBYTE, double*, DWORD);
DWORD WINAPI GpTalkEx4(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenEx4(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpTalkAsync4(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpListenAsync4(DWORD *, DWORD *, BYTE *);
DWORD WINAPI GpCommandAsync4(DWORD *);
DWORD WINAPI GpCheckAsync4(DWORD, DWORD*);
DWORD WINAPI GpStopAsync4(void);
DWORD WINAPI GpDevFindEx4(short, short, short *);
DWORD WINAPI GpBoardstsEx4(DWORD, DWORD, DWORD *);
DWORD WINAPI GpSmoothMode4(DWORD);

#ifdef __cplusplus
}
#endif 


#define HLP_SAMPLES				274
#define HLP_SAMPLES_MASTER		275
#define HLP_SAMPLES_SLAVE		276
#define HLP_SAMPLES_MLTMETER	463
#define HLP_SAMPLES_DVS			278
#define HLP_SAMPLES_OSCILLO		279
#define HLP_SAMPLES_POLLING		382
#define HLP_SAMPLES_PARALLEL	381
#define HLP_SAMPLES_MLTLINE		383

#define ID_TIMER				1
#define ID_TIMER_TERMINATE		2
#define ID_TIMER_TRANSMISSION	3
#define ID_TIMER_RECEPTION		4
#define TIMERCOUNT				100
#define TIMERCOUNT_TERMINATE	3000

#define szHelpFileName95	"..\\..\\..\\..\\..\\Gpib5td.hlp"
#define szHelpFileNameNT	"..\\..\\..\\..\\..\\Gpibntd.hlp"