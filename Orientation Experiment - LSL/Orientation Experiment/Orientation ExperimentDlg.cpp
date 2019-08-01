
// Orientation ExperimentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Orientation Experiment.h"
#include "Orientation ExperimentDlg.h"

#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WSLASTERROR;


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// COrientationExperimentDlg dialog




COrientationExperimentDlg::COrientationExperimentDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COrientationExperimentDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}



void COrientationExperimentDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COrientationExperimentDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PREV, &COrientationExperimentDlg::OnBnClickedPrev)
	ON_BN_CLICKED(IDC_NEXT, &COrientationExperimentDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_START_STOP, &COrientationExperimentDlg::OnBnClickedStartStop)
	//ON_BN_CLICKED(IDC_CONNECT_CUBES, &COrientationExperimentDlg::OnBnClickedConnectCubes)
	ON_BN_CLICKED(IDC_BORESIGHT, &COrientationExperimentDlg::OnBnClickedBoresight)
	ON_BN_CLICKED(IDOK, &COrientationExperimentDlg::OnBnClickedOk)
	ON_EN_CHANGE(IDC_SUB_NUM, &COrientationExperimentDlg::OnEnChangeSubNum)
	ON_BN_CLICKED(IDC_SWAP, &COrientationExperimentDlg::OnBnClickedSwap)
	ON_BN_CLICKED(IDC_PAUSE, &COrientationExperimentDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_RESET_CLOCK, &COrientationExperimentDlg::OnBnClickedResetClock)
	ON_BN_CLICKED(IDC_MARKER, &COrientationExperimentDlg::OnBnClickedMarker)
	ON_BN_CLICKED(IDC_TRIAL_MARKER, &COrientationExperimentDlg::OnBnClickedTrialMarker)
END_MESSAGE_MAP()


// COrientationExperimentDlg message handlers

BOOL COrientationExperimentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	curTrial = 0;
	markerCount = 0;
	subNum = 0;
	statusBox = GetDlgItem(IDC_STATUS);
	statusMessage[0] = '\0';
	running = false;
	pause = false;
	pauseTimer = true;
	openSuccess = 0;
	trackersConnected = false;
	time(&startTime);	
	runningTime = 0;
	
	VERIFY(myFont.CreateFont(
		32,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial")));                 // lpszFacename 
	
	statusBox->SetFont(&myFont);
	changeStatus();	
	GetDlgItem(IDC_START_STOP)->SetFont(&myFont);
	GetDlgItem(IDC_MARKER_COUNT)->SetFont(&myFont);
	GetDlgItem(IDC_CLOCK_TEXT)->SetFont(&myFont);
	setText(IDC_MARKER_COUNT," ");
	setClockBox();
	
	//TODO finish setting up CDF stuff
	cdfs.setDefaults();
	cdfs.session.subjectNumber = 1;
	//hmdMutex = CreateMutex(NULL,FALSE,TEXT("hmd mutex"));
	dataMutex = CreateMutex(NULL, FALSE,TEXT("dataMutex"));



	if(connectCubes()){
		headTracker->boresight();
		torsoTracker->boresight();
	}
	else{
		memset(trackers,0,sizeof(ICube)*2);
		torsoTracker = trackers;
		headTracker = trackers+1;
		setText(IDC_SYSTEM_MSG,"couldn't connect to cubes");
	}
	changeStatus();
	
	// sockets initialization
	int iret;
	WSADATA	wsaData;

	iret = WSAStartup(MAKEWORD(2,0), &wsaData);
	if(iret){
		setStatusBox("Startup failed");
		return iret;
	}
	//socket = setupUDPServer("27016");

	//if(socket != SOCKET_ERROR){
	//	//sprintf_s(statusMessage, BUFFSIZE, "Server at socket #%d",socket);//);
	//	//setStatusBox(statusMessage);
	//	//HMDThreadHandle = (HANDLE)_beginthreadex(NULL,0,HMDThread,this,0,0);
	//}
	//else{
	//	lastError = WSLASTERROR;
	//	sprintf_s(statusMessage, BUFFSIZE, "Error setting up socket: %d",lastError);
	//	setStatusBox(statusMessage);
	//}
	
	timerCubeUpdate = NULL;	
	////start threads
	if(openSuccess){
		CreateTimerQueueTimer(&timerCubeUpdate,		NULL, &cubeCallback,		 this, 0, UPDATE_INTERVAL,	NULL);		
	}
	CreateTimerQueueTimer(&timerDisplayUpdate,      NULL, &cubeDisplayCallback,  this, 0, WRITE_INTERVAL,	NULL);
	CreateTimerQueueTimer(&timerClockDisplayUpdate, NULL, &clockDisplayCallback, this, 0, CLOCK_INTERVAL,	NULL);
	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void COrientationExperimentDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COrientationExperimentDlg::OnPaint()
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
HCURSOR COrientationExperimentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COrientationExperimentDlg::changeStatus(){
	sprintf_s(statusMessage,BUFFSIZE,"Subject %u\nTrial: %u - %s",cdfs.session.subjectNumber,curTrial, TRIAL_TYPES[curTrial].c_str());	
	statusBox->SetWindowText(CA2W(statusMessage));
	//SetDlgItemText(IDC_STATUS, statusMessage);
}


void COrientationExperimentDlg::updateHeadBox(){
	if(pause){
		sprintf_s(torsoTrackerMessage,BUFFSIZE,"PAUSED PAUSED PAUSED PAUSED PAUSED PAUSED\n");
	}
	else{	
		sprintf_s(headTrackerMessage, BUFFSIZE, "Head\nYaw:   % 7.2f\nPitch: % 7.2f\nRoll:  % 7.2f\nYawV:   % 7.2f\nPitchV: % 7.2f\nRollV:  % 7.2f\nW: % 4.2f\nX: % 4.2f\nY: % 4.2f\nZ: % 4.2f",
								headTracker->m_data.YPR[0], headTracker->m_data.YPR[1],headTracker->m_data.YPR[2],
								headTracker->m_data.angVel[0],headTracker->m_data.angVel[1],headTracker->m_data.angVel[2],
								headTracker->m_data.WXYZ[0],headTracker->m_data.WXYZ[1],headTracker->m_data.WXYZ[2],headTracker->m_data.WXYZ[3]);
	}
	GetDlgItem(IDC_HEAD)->SetWindowText(CA2W(headTrackerMessage));	
}

void COrientationExperimentDlg::updateBodyBox(){
	if(pause){
		sprintf_s(torsoTrackerMessage,BUFFSIZE,"PAUSED PAUSED PAUSED PAUSED PAUSED PAUSED\n");
	}
	else{
		sprintf_s(torsoTrackerMessage,BUFFSIZE,"Torso\nYaw:   % 7.2f\nPitch: % 7.2f\nRoll:  % 7.2f\nYawV:   % 7.2f\nPitchV: % 7.2f\nRollV:  % 7.2f\nW: % 4.2f\nX: % 4.2f\nY: % 4.2f\nZ: % 4.2f",
								torsoTracker->m_data.YPR[0], torsoTracker->m_data.YPR[1],torsoTracker->m_data.YPR[2],
								torsoTracker->m_data.angVel[0],torsoTracker->m_data.angVel[1],torsoTracker->m_data.angVel[2],
								torsoTracker->m_data.WXYZ[0],torsoTracker->m_data.WXYZ[1],torsoTracker->m_data.WXYZ[2],torsoTracker->m_data.WXYZ[3]);
	}
	GetDlgItem(IDC_BODY)->SetWindowText(CA2W(torsoTrackerMessage));	

}

//void COrientationExperimentDlg::updateHMDBox(){
//	sprintf_s(HMDTrackerMessage, BUFFSIZE, "frame %d\nW: % 4.2f  X: % 5.2f\nX: % 4.2f  Y: % 5.2f\nY: % 4.2f  Z: % 5.2f\nZ: % 4.2f\nYawV:  % 7.2f\nPitchV:  % 7.2f\nRollV:  % 7.2f",	
//								hmdData.frame, hmdData.qw,	hmdData.px, hmdData.qx,	hmdData.py, hmdData.qy,	hmdData.pz, hmdData.qz, hmdData.vx, hmdData.vy, hmdData.vz);
//	GetDlgItem(IDC_HMD)->SetWindowText(CA2W(HMDTrackerMessage));	
//}

BOOL COrientationExperimentDlg::connectCubes(){
	//for ISD_OpenAllTrackers(), need the destination array to be of size ISD_MAX_TRACKERS or else
	//there is some nasty out-of-bounds memory access violation upon deconstruction
	sprintf_s(statusMessage, BUFFSIZE, "Conecting to trackers");
	setStatusBox(statusMessage);

	ISD_TRACKER_HANDLE	T[ISD_MAX_TRACKERS];
	openSuccess = ISD_OpenAllTrackers( (Hwnd) NULL, T, FALSE, FALSE);
	if(openSuccess < 1){
		sprintf_s(statusMessage, BUFFSIZE, "Could not connect to trackers");
		setStatusBox(statusMessage);
		return FALSE;
	}
	sprintf_s(statusMessage, BUFFSIZE, "Connected to trackers");
	setStatusBox(statusMessage);
	displayCubes = true;
	ISD_NumOpenTrackers(&numOpenTrackers);
	torsoTracker = trackers;
	headTracker = trackers+1;
	torsoTracker->initialize(T[0]);
	headTracker->initialize(T[1]);
	torsoTracker->p_info = new lsl::stream_info("Torso","ICube",sizeof(CubeData)/4);
	torsoTracker->p_outlet = new lsl::stream_outlet(*(torsoTracker->p_info));
	headTracker->p_info = new lsl::stream_info("Head","ICube",sizeof(CubeData)/4);
	headTracker->p_outlet = new lsl::stream_outlet(*(headTracker->p_info));
	p_info = new lsl::stream_info("Marker","Marker",1);
	p_outlet = new lsl::stream_outlet(*p_info);

	return TRUE;
}



void COrientationExperimentDlg::setStatusBox(LPCTSTR msg){
	GetDlgItem(IDC_SYSTEM_MSG)->SetWindowText(msg);
}

void COrientationExperimentDlg::setStatusBox(char* msg){
	GetDlgItem(IDC_SYSTEM_MSG)->SetWindowTextW(CA2W(msg));
}

void COrientationExperimentDlg::setText(UINT control,LPCTSTR msg){
	GetDlgItem(control)->SetWindowText(msg);
}

void COrientationExperimentDlg::setText(UINT control, char* msg){
	GetDlgItem(control)->SetWindowTextW(CA2W(msg));
}

void COrientationExperimentDlg::setClockBox(){
	int min = floor(runningTime)/60;
	int s = int(runningTime)%60;
	sprintf(timeDisplayBuffer,"%02d:%02d",min,s);	
	SetDlgItemText(IDC_CLOCK_TEXT, CA2W(timeDisplayBuffer));
	
}


VOID CALLBACK COrientationExperimentDlg::cubeCallback(PVOID obj, BOOLEAN TimerOrWaitFired){
	COrientationExperimentDlg *l = reinterpret_cast<COrientationExperimentDlg*>(obj);		
	//printf("started cubes,display cubes is %d\n",l->m_displayCubes);
	
	if(!l->openSuccess)
		return;
	DWORD waitResult;
	waitResult = WaitForSingleObject(l->dataMutex,UPDATE_INTERVAL);
	if(waitResult == WAIT_OBJECT_0){
		l->headTracker->update();
		l->torsoTracker->update();		
		CubeData h,t;
		memcpy(&h,&(l->headTracker->m_data),sizeof(CubeData));
		memcpy(&t,&(l->torsoTracker->m_data),sizeof(CubeData));
		if(!l->pause){
			l->headTracker->p_outlet->push_numeric_struct(h);
			l->torsoTracker->p_outlet->push_numeric_struct(t);
		}
		ReleaseMutex(l->dataMutex);
	}

}

VOID CALLBACK COrientationExperimentDlg::cubeDisplayCallback(PVOID obj, BOOLEAN TimerOrWaitFired){
	COrientationExperimentDlg *l = reinterpret_cast<COrientationExperimentDlg*>(obj);
	l->updateHeadBox();
	l->updateBodyBox();
	

	wchar_t buff[512];
	//swprintf_s(buff,512,L"Frame %d",l->cdfs.session.numFrames);
	//l->setStatusBox(buff);

	DWORD waitResult;
	waitResult = WaitForSingleObject(l->dataMutex,WRITE_INTERVAL);
	if(waitResult == WAIT_OBJECT_0){

		memcpy(l->cubeData[0].YPR, l->headTracker->m_data.YPR, 3*sizeof(float));
		memcpy(l->cubeData[0].WXYZ,l->headTracker->m_data.WXYZ,4*sizeof(float));
		memcpy(l->cubeData[0].angVel, l->headTracker->m_data.angVel,3*sizeof(float));
		memcpy(l->cubeData[1].YPR, l->torsoTracker->m_data.YPR, 3*sizeof(float));
		memcpy(l->cubeData[1].WXYZ,l->torsoTracker->m_data.WXYZ,4*sizeof(float));
		memcpy(l->cubeData[1].angVel, l->torsoTracker->m_data.angVel,3*sizeof(float));
		l->cdfs.setFrameData((float *)l->cubeData,sizeof(CubeData)/sizeof(float),2);
		
		//DWORD hmdWait;
		//hmdWait = WaitForSingleObject(l->hmdMutex,UPDATE_INTERVAL);
		//if(hmdWait == WAIT_OBJECT_0){
		//	l->cdfs.setFrameHeader(&(l->hmdData),HMD_DATA_SIZE);
		//	ReleaseMutex(l->hmdMutex);
		//}
		//if(l->saving)
		//	l->cdfs.writeFrame();

		ReleaseMutex(l->dataMutex);
	}
	
}

VOID CALLBACK COrientationExperimentDlg::clockDisplayCallback(PVOID obj, BOOLEAN TimerOrWaitFired){
	COrientationExperimentDlg *l = reinterpret_cast<COrientationExperimentDlg*>(obj);	
	if(!l->pauseTimer){		
		time_t curTime;
		time(&curTime);
		l->runningTime+= difftime(curTime,l->startTime);
		l->setClockBox();
		l->startTime = curTime;
	}
}

void COrientationExperimentDlg::OnEnChangeSubNum()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	wchar_t s[16];
	GetDlgItemText(IDC_SUB_NUM,s,16);
	cdfs.setSubNum(_wtoi(s));
	changeStatus();
}

//unsigned int __stdcall HMDThread(void *params){
//	COrientationExperimentDlg *l = reinterpret_cast<COrientationExperimentDlg*>(params);
//
//	struct sockaddr_storage clientAddr;
//	memset(&clientAddr,0,sizeof(clientAddr));
//	socklen_t addrlen = sizeof(clientAddr);
//	SOCKET s = l->socket;
//
//	int numbr;
//	char clientBuffer[BUFFSIZE];
//	memset(clientBuffer,0,BUFFSIZE);
//	DWORD res;
//	HMDData d;
//	char msg[BUFFSIZE];
//	//l->setStatusBox("starting HMD thread");
//	sprintf_s(msg,BUFFSIZE,"socket #%d",s);
//	l->GetDlgItem(IDC_HMD)->SetWindowText(CA2W(msg));	
//	while(true){
//		numbr = recvfrom(s,clientBuffer,BUFFSIZE,0,(struct sockaddr *)&clientAddr,&addrlen);
//		
//		if(numbr==SOCKET_ERROR){
//			int nError = WSAGetLastError();
//			//if there is an error and it is not a "would block" error
//			if(nError!=WSAEWOULDBLOCK && nError!=0){
//				sprintf_s(msg,BUFFSIZE,"UDP Disconnecting due to winsock error code: %d\n",nError);	
//				l->setStatusBox(msg);
//				closesocket(l->socket);
//				l->socket = INVALID_SOCKET;
//				WSACleanup();
//				return nError;
//			}
//		}
//		else{ //received data from socket
//			DWORD waitResult;
//			waitResult = WaitForSingleObject(l->hmdMutex,UPDATE_INTERVAL);
//			if(waitResult == WAIT_OBJECT_0){//received data from socket
//				memcpy(&(l->hmdData),clientBuffer,HMD_DATA_SIZE);
//				l->updateHMDBox();
//				ReleaseMutex(l->hmdMutex);
//			}
//		}//end else
//		//quit if the program has terminated
//
//	}//
//	return 0;
//}

SOCKET setupUDPServer(char *port){
	SOCKET s;	
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, port,&hints,&servinfo);
	int ret;
	for(p=servinfo;p!=NULL;p=p->ai_next){
		s = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(s==SOCKET_ERROR)
			continue;
		ret = bind(s,p->ai_addr,p->ai_addrlen);
		if(ret == SOCKET_ERROR){
			closesocket(s);
			continue;
		}
		break;
	}

	if(p == NULL){
		WSLASTERROR = GetLastError();
		return SOCKET_ERROR;
	}
	freeaddrinfo(servinfo);

	u_long iMode=1; //iMode = 1 for non-blocking socket
	ioctlsocket(s,FIONBIO,&iMode);
	
	return s;
}

void COrientationExperimentDlg::OnBnClickedOk()
{
	//TerminateThread(HMDThread,0);
	saving = false;
	
	DeleteTimerQueueTimer(NULL,timerDisplayUpdate,INVALID_HANDLE_VALUE);
	DeleteTimerQueueTimer(NULL,timerClockDisplayUpdate, INVALID_HANDLE_VALUE);
	if(timerCubeUpdate!=NULL)
		DeleteTimerQueueTimer(NULL,timerCubeUpdate,INVALID_HANDLE_VALUE);
	
	
	//CloseHandle(hmdMutex);
	CloseHandle(dataMutex);
	
	cdfs.close();
	closesocket(socket);
	WSACleanup();
	//default handler
	CDialogEx::OnOK();
}

void COrientationExperimentDlg::OnBnClickedPrev()
{
	if(curTrial>0)
		curTrial--;	
	changeStatus();
}

void COrientationExperimentDlg::OnBnClickedNext()
{	
	if(curTrial < MAX_NUM_TRIAL_TYPES)
		curTrial++;
	changeStatus();
}


void COrientationExperimentDlg::OnBnClickedStartStop()
{
	running = !running;
	if(running){//started running
		GetDlgItem(IDC_START_STOP)->SetWindowText(L"Stop Trial");
		cdfs.setupNextTrial();

		sprintf_s(filename,BUFFSIZE,"sub%02d_trial%02d_%2d_%2d_%2d_%2d.cdf",cdfs.session.subjectNumber,curTrial,cdfs.session.date.month,cdfs.session.date.day,cdfs.session.date.hour,
			cdfs.session.date.minute);
		cdfs.session.trialType = curTrial;
		cdfs.open(filename);
		saving = true;
		setStatusBox(filename);
	}
	else{//stopped running
		saving = false;
		cdfs.close();
		sprintf_s(statusMessage,BUFFSIZE,"%s saved",filename);
		setStatusBox(statusMessage);
		GetDlgItem(IDC_START_STOP)->SetWindowText(L"Start Trial");
		OnBnClickedNext();		
	}
}

//void COrientationExperimentDlg::OnBnClickedConnectCubes()
//{
//	if(openSuccess < 1)
//		connectCubes();
//	else{
//		sprintf_s(statusMessage, BUFFSIZE, "Can't connect to trackers");
//		setStatusBox(statusMessage);
//	}
//}


void COrientationExperimentDlg::OnBnClickedBoresight()
{
	if(openSuccess){
		BOOL success = (headTracker->boresight() | torsoTracker->boresight());
		if(!success)
			setStatusBox(L"Could not boresight");
		else
			setStatusBox(L"Trackers boresighted");

	}
}

void COrientationExperimentDlg::OnBnClickedSwap()
{
	ICube temp = trackers[0];
	trackers[0] = trackers[1];
	trackers[1] = temp;
}


void COrientationExperimentDlg::OnBnClickedPause()
{
	pause = !pause;
}


void COrientationExperimentDlg::OnBnClickedResetClock()
{	
	runningTime = 0;
}


void COrientationExperimentDlg::OnBnClickedMarker()
{
	char buffer[8];
	pauseTimer = !pauseTimer;
	if(!pauseTimer){
		sprintf_s(buffer,8,"Started");	
		time(&startTime); //get a new start time
		p_outlet->push_sample("1");	
	}
	else{
		sprintf_s(buffer,8,"Stopped");	
		p_outlet->push_sample("2");	
	}	
	markerCount++;
	setText(IDC_MARKER_COUNT,buffer);	
}


void COrientationExperimentDlg::OnBnClickedTrialMarker()
{	
	p_outlet->push_sample(&curTrial);
	if(pauseTimer == false){
		setText(IDC_MARKER_COUNT,"Stopped");
		pauseTimer = true;
	}
	runningTime = 0;
	setClockBox();
}
