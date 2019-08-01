
// ExperimentControl.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ExperimentControl.h"
#include "ExperimentControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CExperimentControlApp

BEGIN_MESSAGE_MAP(CExperimentControlApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CExperimentControlApp construction

CExperimentControlApp::CExperimentControlApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CExperimentControlApp object

CExperimentControlApp theApp;


// CExperimentControlApp initialization

BOOL CExperimentControlApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	
	
	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CExperimentControlDlg dlg;
	m_pMainWnd = &dlg;
	m_app = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		WaitForSingleObject(dlg.m_exiting,1);
	}
	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

