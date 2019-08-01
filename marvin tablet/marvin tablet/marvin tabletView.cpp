
// marvin tabletView.cpp : implementation of the CmarvintabletView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "marvin tablet.h"
#endif

#include "marvin tabletDoc.h"
#include "marvin tabletView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmarvintabletView

IMPLEMENT_DYNCREATE(CmarvintabletView, CView)

BEGIN_MESSAGE_MAP(CmarvintabletView, CView)
END_MESSAGE_MAP()

// CmarvintabletView construction/destruction

CmarvintabletView::CmarvintabletView()
{
	// TODO: add construction code here

}

CmarvintabletView::~CmarvintabletView()
{
}

BOOL CmarvintabletView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CmarvintabletView drawing

void CmarvintabletView::OnDraw(CDC* /*pDC*/)
{
	CmarvintabletDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CmarvintabletView diagnostics

#ifdef _DEBUG
void CmarvintabletView::AssertValid() const
{
	CView::AssertValid();
}

void CmarvintabletView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CmarvintabletDoc* CmarvintabletView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CmarvintabletDoc)));
	return (CmarvintabletDoc*)m_pDocument;
}
#endif //_DEBUG


// CmarvintabletView message handlers
