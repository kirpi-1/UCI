// HMDBeamer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"



//d3dcompiler.lib
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#include <conio.h>
#include <WinBase.h>
//#include <windows.h>
#include <MMSystem.h>

#include "HMDBeamer.h"
//surface pro is C13120201SPRO
//SAGER02
#define DEFAULT_SERVER "C13120201SPRO"
#define HMD_PORT "27016"
#define SEND_PERIOD 10

VOID CALLBACK HMDSendCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

#define HMDSENDBUFF_NUM_FLOATS 12
typedef struct _hmdsendbuf
{
	// frame counter
	unsigned int framecount;
	// relative timestamp (msec) via timeGetTime()
	DWORD timestamp;
	// hmd headpose thepose orientation
	float hp_qw;
	float hp_qx;
	float hp_qy;
	float hp_qz;
	// hmd headpose thepose position
	float hp_px;
	float hp_py;
	float hp_pz;
	// hmd headpose angular velocity
	float hp_vx;
	float hp_vy;
	float hp_vz;
} HMDSendBuf;

DWORD lasttime;
unsigned long framecounter;
ovrHmd Hmd;
HMDSendBuf hmdsendbuf;
ovrFrameTiming HmdFrameTiming;
WSADATA wsaData;
SOCKET udpSocket;
SYSTEMTIME st;
float data[HMDSENDBUFF_NUM_FLOATS];
addrinfo *socketInfo;



int sendFloatsUDP(SOCKET s,struct addrinfo *server, float *f, unsigned int size){
	char *b;	
	b = reinterpret_cast<char *>(f);
	return sendto(s,b,size*sizeof(float),0,server->ai_addr,server->ai_addrlen);
}

int LoadSendBufTrackState(HMDSendBuf *hsb,ovrTrackingState *ts)
{
	hsb->hp_qw = ts->HeadPose.ThePose.Orientation.w;
	hsb->hp_qx = ts->HeadPose.ThePose.Orientation.x;
	hsb->hp_qy = ts->HeadPose.ThePose.Orientation.y;
	hsb->hp_qz = ts->HeadPose.ThePose.Orientation.z;
	hsb->hp_px = ts->HeadPose.ThePose.Position.x;
	hsb->hp_py = ts->HeadPose.ThePose.Position.y;
	hsb->hp_pz = ts->HeadPose.ThePose.Position.z;
	hsb->hp_vx = ts->HeadPose.AngularVelocity.x;
	hsb->hp_vy = ts->HeadPose.AngularVelocity.y;
	hsb->hp_vz = ts->HeadPose.AngularVelocity.z;

	return 1;
}
int PrintSendBufTrackState(HMDSendBuf *hsb)
{
	printf("framecount %u timestamp %u\n",hsb->framecount,hsb->timestamp);
	printf("HeadPose Or %g\t%g\t%g\t%g\n",hsb->hp_qw,hsb->hp_qx,hsb->hp_qy,hsb->hp_qz);
	printf("HeadPose Po %g\t%g\t%g\n",hsb->hp_px,hsb->hp_py,hsb->hp_pz);
	printf("HeadPose Ve %g\t%g\t%g\n\n",hsb->hp_vx,hsb->hp_vy,hsb->hp_vz);
	return 1;
}
int main()
{	
	int nbytestosend = sizeof(HMDSendBuf);
	lasttime=0;
	framecounter=0;
	ovr_Initialize();
	printf("detected %d headests\n", ovrHmd_Detect());
	printf("initializing headset...\n");
	
	Hmd = ovrHmd_Create(0);
	if (!Hmd)
	{
		printf("Failure to initialize OVR and Rift HMD\n");
		Beep(128,500);
		return 1;
	}
	ovrHmd_RecenterPose(Hmd);
	printf("initializing winsock...\n");
	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 0;
	}
	printf("creating socket...\n");

	struct addrinfo *result,*p;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints)); //VERY IMPORTANT
	hints.ai_family = AF_INET;	
	hints.ai_socktype = SOCK_DGRAM;
	DWORD iret;
	if((iret = getaddrinfo(DEFAULT_SERVER, HMD_PORT,&hints, &result))!=0){		
		iret = WSAGetLastError();
		printf("getaddrinfo error, %d\n", iret);
		WSACleanup();
		return iret;
	}
	for(p = result;p!=NULL;p=p->ai_next){
		udpSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
		if(udpSocket == -1 || result->ai_family != AF_INET)
			continue;
		break;
	}
	if(p == NULL){
		printf("couldn't create socket");
		return 5;
	}
	socketInfo = p;
	//configure HMD
	ovrHmd_ConfigureTracking(Hmd, ovrTrackingCap_Orientation |
								  ovrTrackingCap_MagYawCorrection |
								  ovrTrackingCap_Position,0);


	HmdFrameTiming.ScanoutMidpointSeconds = 0.0;
	ovrTrackingState trackState = ovrHmd_GetTrackingState(Hmd,HmdFrameTiming.ScanoutMidpointSeconds);	
	
	LoadSendBufTrackState(&hmdsendbuf,&trackState);
	PrintSendBufTrackState(&hmdsendbuf);

	printf("ready!\n");
	
	
	HANDLE timer;
	CreateTimerQueueTimer(&timer,NULL,&HMDSendCallback,NULL,0,SEND_PERIOD,NULL);
			
	while(1){
		if(_kbhit()){
			int key = _getch();
			if(key=='q')
				break;	
			else if(key=='r')
				ovrHmd_RecenterPose(Hmd);
		}

	}
	DeleteTimerQueueTimer(NULL,timer,INVALID_HANDLE_VALUE);
	iResult = shutdown(udpSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(udpSocket);
		WSACleanup();
		Beep(128,500);Beep(256,500);Beep(512,500);Beep(1024,500);Beep(2048,500);Beep(4096,500);
		return 1;
	}

	freeaddrinfo(result);
	return 0;
}



VOID CALLBACK HMDSendCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired){
	if(udpSocket==INVALID_SOCKET)
		return;
	ovrTrackingState trackState = ovrHmd_GetTrackingState(Hmd,HmdFrameTiming.ScanoutMidpointSeconds);
	
	hmdsendbuf.framecount = framecounter++;	
	GetLocalTime(&st);
	hmdsendbuf.timestamp = st.wMilliseconds+1000*(st.wSecond+60*(st.wMinute+60*st.wHour));
	LoadSendBufTrackState(&hmdsendbuf,&trackState);
	memcpy(data,&hmdsendbuf,sizeof(hmdsendbuf));

	
	DWORD iResult = sendFloatsUDP(udpSocket,socketInfo,data,HMDSENDBUFF_NUM_FLOATS);



	PrintSendBufTrackState(&hmdsendbuf);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(udpSocket);
		udpSocket = INVALID_SOCKET;
		WSACleanup();
		Beep(128,500);Beep(256,500);Beep(512,500);Beep(1024,500);Beep(2048,500);
		return;
	}


}

