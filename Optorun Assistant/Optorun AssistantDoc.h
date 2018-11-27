// Optorun AssistantDoc.h : interface of the COptorunAssistantDoc class
//

#pragma once

class COptorunAssistantDoc : public CDocument
{
protected: // create from serialization only
	COptorunAssistantDoc();
	DECLARE_DYNCREATE(COptorunAssistantDoc)

// Attributes
public:


// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~COptorunAssistantDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()


public:
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);

};


