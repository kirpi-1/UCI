
// ExperimentControlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#define DEFAULT_SERVER_NAME "sager02"
#define DEFAULT_PORT "27015"

#define DEFAULT_BUFF_SIZE		256

// CExperimentControlDlg dialog
class CExperimentControlDlg : public CDialogEx
{
// Construction
public:
	CExperimentControlDlg(CWnd* pParent = NULL);	// standard constructor
	~CExperimentControlDlg();
// Dialog Data
	enum { IDD = IDD_EXPERIMENTCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	SOCKET m_socket;	
	wchar_t m_msg[DEFAULT_BUFF_SIZE];
	BYTE m_buffer[DEFAULT_BUFF_SIZE];
	UINT m_bufferIdx;
	int counter;
	HANDLE m_hWnd;
	HANDLE m_socketThread;
	HANDLE m_connected;
	HANDLE m_quit;

	static unsigned int __stdcall socketThread(void *obj);
public:
	
	afx_msg void OnBnClickedButtonConnect();
	afx_msg static unsigned int __stdcall OnBnClickedButtonConnectAsync(void *obj);
	afx_msg void OnStnClickedStaticStatus();
	afx_msg void OnBnClickedButtonQuit();	
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();	
	afx_msg void OnBnClickedButtonBoresight();

	BOOL OpenConnection();
	void CloseConnection(); 
	void Send(UINT msg, UINT size = 1);

	CWnd *pStatusBox;

	HANDLE m_exiting;
	CStatic *m_Label;

	
	afx_msg void OnBnClickedButtonResetServos();
};
