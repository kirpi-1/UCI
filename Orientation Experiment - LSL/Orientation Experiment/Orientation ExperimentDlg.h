
// Orientation ExperimentDlg.h : header file
//

#pragma once

#define UPDATE_INTERVAL 1
#define WRITE_INTERVAL 10
#define CLOCK_INTERVAL 1000
#define HMD_RECEIVE_INTERVAL 10
#define HEAD_INDEX 0
#define TORSO_INDEX 1

#include "cdf.h"
#include "isense.h"
#include "icube.h"
#include "lsl_cpp.h"
#include <time.h>


#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "liblsl32.lib")
#pragma comment(lib, "Winmm.lib")

const std::string TRIAL_TYPES[] = {"NULL","Walking", "Jogging", "Walk Look Right", "Walk Look Left", "Stair Ascent/Descent", "NULL", "Treadmill Walking", "Treadmill Jogging"};
#define BUFFSIZE 512
#define HMD_DATA_NUM 12
#define HMD_DATA_SIZE HMD_DATA_NUM*sizeof(float)
#define MAX_NUM_TRIAL_TYPES 9

struct HMDData{
	DWORD frame;
	DWORD timestamp;
	float qw;
	float qx;
	float qy;
	float qz;
	float px;
	float py;
	float pz;
	float vx;
	float vy;
	float vz;
};



// COrientationExperimentDlg dialog
class COrientationExperimentDlg : public CDialogEx
{
// Construction
public:
	COrientationExperimentDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ORIENTATIONEXPERIMENT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	int				curTrial;
	unsigned int	trialList[100];
	unsigned int	subNum;
	bool			running,pause,pauseTimer;
	double			runningTime;
	

	CWnd *			statusBox;	
	CFont			myFont;
	char			statusMessage[BUFFSIZE];
	char			headTrackerMessage[BUFFSIZE];
	char			torsoTrackerMessage[BUFFSIZE];
	char			HMDTrackerMessage[BUFFSIZE];
	
	WORD			numOpenTrackers;
	DWORD			openSuccess;
	bool			trackersConnected;
	bool			displayCubes;
	cdf::CDFSession	cdfs;
	ICube			trackers[2], *headTracker, *torsoTracker;
	CubeData		cubeData[2];

	time_t			startTime;
	char			timeDisplayBuffer[16];

	lsl::stream_info	*p_info;
	lsl::stream_outlet	*p_outlet;
	
	unsigned int	markerCount;
	

public:
	HANDLE			timerCubeUpdate, timerDisplayUpdate, timerClockDisplayUpdate, HMDThreadHandle;
	HANDLE			hmdMutex, dataMutex;
	LARGE_INTEGER	startLI;
	LARGE_INTEGER	performanceFrequency;
	FILETIME		now;

	CAsyncSocket	csock;
	SOCKET			socket;
	DWORD			lastError;
	float			HMDDataBuffer[HMD_DATA_SIZE];
	HMDData			hmdData;
	
	

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void changeStatus();
	void updateHeadBox();
	void updateBodyBox();
	//void updateHMDBox();
	BOOL connectCubes();
	char filename[BUFFSIZE];
	bool saving;
	LARGE_INTEGER lip;

public:
	afx_msg void OnBnClickedPrev();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedStartStop();
	//afx_msg void OnBnClickedConnectCubes();
	afx_msg void OnBnClickedBoresight();
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeSubNum();
	afx_msg void OnBnClickedSwap();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedResetClock();
	afx_msg void OnBnClickedMarker();
	afx_msg void OnBnClickedTrialMarker();

	void setStatusBox(LPCTSTR msg);	
	void setStatusBox(char *msg);
	void setClockBox();
	void setText(UINT control, LPCTSTR msg);
	void setText(UINT control, char* msg);

private:
	static VOID CALLBACK cubeCallback(PVOID obj, BOOLEAN TimerOrWaitFired);
	static VOID CALLBACK cubeDisplayCallback(PVOID obj, BOOLEAN TimerOrWaitFired);
	//static VOID CALLBACK HMDUpdateCallback(PVOID obj, BOOLEAN TimerOrWaitFired);
	static VOID CALLBACK lslStreamCallback(PVOID obj, BOOLEAN TimerOrWaitFired);
	static VOID CALLBACK clockDisplayCallback(PVOID obj, BOOLEAN TimerOrWaitFired);


};

unsigned int __stdcall HMDThread(void *params);
SOCKET setupUDPServer(char *port);
