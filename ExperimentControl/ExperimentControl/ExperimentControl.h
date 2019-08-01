
// ExperimentControl.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "ExperimentControlDlg.h"
// CExperimentControlApp:
// See ExperimentControl.cpp for the implementation of this class
//

class CExperimentControlApp : public CWinApp
{
public:
	CExperimentControlApp();

	CExperimentControlDlg *m_app;
// Overrides
public:
	virtual BOOL InitInstance();
	//virtual int OnIdle();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CExperimentControlApp theApp;