// ExperimentControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExperimentControl.h"
#include "ExperimentControlDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CExperimentControlDlg::CExperimentControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CExperimentControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CExperimentControlDlg::~CExperimentControlDlg(){
	//ZACK DO THIS REMOVE THE EVENTS
}
void CExperimentControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, ID_STATUS_BOX, m_Label);
}

BEGIN_MESSAGE_MAP(CExperimentControlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()	
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CExperimentControlDlg::OnBnClickedButtonConnect)		
	ON_BN_CLICKED(ID_BUTTON_START, &CExperimentControlDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(ID_BUTTON_STOP, &CExperimentControlDlg::OnBnClickedButtonStop)	
	ON_BN_CLICKED(ID_BUTTON_BORESIGHT, &CExperimentControlDlg::OnBnClickedButtonBoresight)
	ON_BN_CLICKED(ID_BUTTON_RESET_SERVOS, &CExperimentControlDlg::OnBnClickedButtonResetServos)
END_MESSAGE_MAP()


// CExperimentControlDlg message handlers

BOOL CExperimentControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	pStatusBox = GetDlgItem(ID_STATUS_BOX);
	SetDlgItemText(ID_STATUS_BOX,_T("READY!"));
	counter = 0;
	m_connected = CreateEvent(NULL,TRUE,FALSE,_T("Connected"));
	m_quit = CreateEvent(NULL,TRUE, FALSE, _T("QuitEvent"));
	m_exiting = CreateEvent(NULL,TRUE,FALSE,_T("ExitEvent"));

	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160,L"Arial Bold");
	m_Label = (CStatic *)GetDlgItem(ID_STATUS_BOX);
	m_Label->SetFont(m_Font1);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CExperimentControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CExperimentControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

afx_msg void CExperimentControlDlg::OnBnClickedButtonConnect(){
	if(OpenConnection()){		
		m_socketThread = (HANDLE)_beginthreadex(NULL,0,socketThread,this,0,0);
		m_bufferIdx = 0;
		SetEvent(m_connected);
		swprintf(m_msg,DEFAULT_BUFF_SIZE,L"Successfully connected", WSAGetLastError());
		SetDlgItemText(ID_STATUS_BOX,m_msg);
	}
	else
		SetDlgItemText(ID_STATUS_BOX,_T("Could not open a connection"));
}

unsigned int __stdcall CExperimentControlDlg::socketThread(void *obj){
	CExperimentControlDlg *cp = reinterpret_cast<CExperimentControlDlg *>(obj);
	char buffer[DEFAULT_BUFF_SIZE];
	
	DWORD res = WaitForSingleObject(cp->m_quit,0);
	while(res != WAIT_OBJECT_0){
		int numbr = recv(cp->m_socket,buffer,DEFAULT_BUFF_SIZE,0);
		if(numbr == SOCKET_ERROR){
			swprintf(cp->m_msg,DEFAULT_BUFF_SIZE,L"Error receving: %d",GetLastError());
			cp->SetDlgItemText(ID_STATUS_BOX,cp->m_msg);
		}
		else if(numbr>0){
			swprintf(cp->m_msg,DEFAULT_BUFF_SIZE,L"%s",buffer);
			cp->SetDlgItemText(ID_STATUS_BOX,cp->m_msg);
		}
		res = WaitForSingleObject(cp->m_quit,0);	
	}
	return 0;
}

BOOL CExperimentControlDlg::OpenConnection(){
	//check if already connected
	DWORD res = WaitForSingleObject(m_connected,0);
	if(res = WAIT_OBJECT_0){ //if already connected
		swprintf(m_msg,DEFAULT_BUFF_SIZE,L"Already connected");
		SetDlgItemText(ID_STATUS_BOX,m_msg);
		return FALSE;
	}
	int iResult;
	WSADATA wsaData;
	m_socket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;
	char *sendbuf = "ExperimentControlDlg";	
	char inetname[512];

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) 
	{
		swprintf(m_msg,DEFAULT_BUFF_SIZE,L"WSAStartup failed with error: %d", iResult);
		SetDlgItemText(ID_STATUS_BOX,m_msg);
        return FALSE;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_SERVER_NAME, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 )
	{
		swprintf(m_msg,DEFAULT_BUFF_SIZE,L"getaddrinfo failed with error: %d", iResult);
		SetDlgItemText(ID_STATUS_BOX,m_msg);
		WSACleanup();
        return FALSE;
    }

    // Attempt to connect to an address until one succeeds
	for (ptr=result; ptr != NULL ;ptr=ptr->ai_next) 
	{
		// Create a SOCKET for connecting to server
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, 
			ptr->ai_protocol);
		if (m_socket == INVALID_SOCKET) 
		{
			swprintf(m_msg,DEFAULT_BUFF_SIZE,L"socket failed with error: %ld", WSAGetLastError());
			SetDlgItemText(ID_STATUS_BOX,m_msg);
			WSACleanup();
			return FALSE;
        }

		// Connect to server.
		iResult = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) 
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

    if (m_socket == INVALID_SOCKET) 
	{
        swprintf(m_msg,DEFAULT_BUFF_SIZE,L"Unable to connect to %s, namely %s:%s",
			DEFAULT_SERVER_NAME,inetname,DEFAULT_PORT);
		SetDlgItemText(ID_STATUS_BOX,m_msg);
		WSACleanup();
		return FALSE;
    }
	return TRUE;
}

void CExperimentControlDlg::CloseConnection(){
	int iResult;
	
	DWORD res = WaitForSingleObject(m_connected,0);
	if(res != WAIT_OBJECT_0) //if not connected
		return;
	//else continue and close connection

    // shutdown the connection since no more data will be sent
	iResult = shutdown(m_socket, SD_SEND);
	if (iResult == SOCKET_ERROR) 
	{
		swprintf(m_msg,DEFAULT_BUFF_SIZE,L"shutdown failed with error: %d\n", WSAGetLastError());
		SetDlgItemText(ID_STATUS_BOX,m_msg);
	}
	closesocket(m_socket);	
	ResetEvent(m_connected);
}


void CExperimentControlDlg::OnBnClickedButtonQuit()
{	
	DWORD res = WaitForSingleObject(m_connected,0);
	if(res == WAIT_OBJECT_0){
		Send(MSG_EXIT);
		CloseConnection();
		WSACleanup();
	}	
	SetEvent(m_exiting);
}

void CExperimentControlDlg::OnBnClickedButtonStart()
{	
	Send(MSG_START);
}


void CExperimentControlDlg::OnBnClickedButtonStop()
{
	Send(MSG_STOP);
}


void CExperimentControlDlg::OnBnClickedButtonBoresight()
{
	Send(MSG_BORESIGHT);
}

void CExperimentControlDlg::Send(UINT msg, UINT size){
	if(m_bufferIdx<DEFAULT_BUFF_SIZE){
		m_buffer[m_bufferIdx] = msg;
		m_bufferIdx++;
	}
	DWORD res = WaitForSingleObject(m_connected,0);
	/*	
	swprintf(m_msg,128,L"start res: %x",res);
	SetDlgItemTextW(ID_STATUS_BOX,m_msg);
	*/
	if(res == WAIT_OBJECT_0){
		int numBytesSent = send(m_socket,(char *)m_buffer,m_bufferIdx,0);
		if(numBytesSent == SOCKET_ERROR){
			DWORD err = GetLastError();
			swprintf(m_msg,256,L"Socket Error: %d",err);
			SetDlgItemTextW(ID_STATUS_BOX,m_msg);
		}
	}
}

void CExperimentControlDlg::OnBnClickedButtonResetServos()
{
	Send(MSG_RESET_SERVOS);
}
