// Optorun AssistantDoc.cpp : implementation of the COptorunAssistantDoc class
//

#include "stdafx.h"
#include "Optorun Assistant.h"

#include "Optorun AssistantDoc.h"
#include "OACnfgDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COptorunAssistantDoc

IMPLEMENT_DYNCREATE(COptorunAssistantDoc, CDocument)

BEGIN_MESSAGE_MAP(COptorunAssistantDoc, CDocument)
END_MESSAGE_MAP()


// COptorunAssistantDoc construction/destruction

COptorunAssistantDoc::COptorunAssistantDoc()
{
	// TODO: add one-time construction code here

}

COptorunAssistantDoc::~COptorunAssistantDoc()
{

}

BOOL COptorunAssistantDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// COptorunAssistantDoc serialization

void COptorunAssistantDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// COptorunAssistantDoc diagnostics

#ifdef _DEBUG
void COptorunAssistantDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COptorunAssistantDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// COptorunAssistantDoc commands
BOOL COptorunAssistantDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	// TODO: Add your specialized code here and/or call the base class
	if(gi_stVacuumTestState.bTest || g_bTestHearthRotation)
	{
		//测试进行中时，不允许退出
		AfxMessageBox(g_strSystem[_OA_STR_SYSTEM_ERROR_TEST],MB_OK|MB_ICONSTOP);
		return false;
	}
	return CDocument::CanCloseFrame(pFrame);
}
