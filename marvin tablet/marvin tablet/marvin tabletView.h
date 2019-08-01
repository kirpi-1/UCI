
// marvin tabletView.h : interface of the CmarvintabletView class
//

#pragma once


class CmarvintabletView : public CView
{
protected: // create from serialization only
	CmarvintabletView();
	DECLARE_DYNCREATE(CmarvintabletView)

// Attributes
public:
	CmarvintabletDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CmarvintabletView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in marvin tabletView.cpp
inline CmarvintabletDoc* CmarvintabletView::GetDocument() const
   { return reinterpret_cast<CmarvintabletDoc*>(m_pDocument); }
#endif

