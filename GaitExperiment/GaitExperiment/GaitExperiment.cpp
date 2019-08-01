#include "stdafx.h"

#include "isense.h"
#include "icube.h"
#include "gExperiment.h"
#include <process.h>
#include "metronome.h"
#include <fstream>

#include <stdio.h> 
#include <stdlib.h> 
#include <memory.h>
#include <string.h>
#include <conio.h>


#pragma comment(lib, "Ws2_32.lib")

#define BUFFSIZE 128
const int REQ_WINSOCK_VER = 2;
char *DEFAULT_PORT = "27015";
const int BUFFER_SIZE = 128;
extern gExperiment *gp;


//unsigned __stdcall testThread(void *pArgs);
HANDLE quitEvent;
HANDLE stopSavingEvent;
HANDLE tcpThread;
HANDLE clientConnected;
HANDLE cubeMutex;
Metronome *mp;
HANDLE hMET = NULL, hCUBE = NULL;
LRCubes *lrc;

struct GaitArguments{
	char basefilename[256];
	char trialsFilename[256];
	UINT subjectNumber;
	UINT startTrial;
};

SOCKET setupServerSocket(char *port);
int connectToClient(SOCKET s);
GaitArguments parseArguments(int argc, char *argv[]);
UINT loadTrialList(char *filename, UINT trialList[]);
int handleTCPClient(MySockType *s);
unsigned int __stdcall socketThread(void *params);

int main(int argc, char *argv[]){
	quitEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("QuitEvent"));
	ResetEvent(quitEvent);
	clientConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("Client Connected"));
	cubeMutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
	//printf("testing inertia cubes\n");
	//DWORD result = WaitForSingleObject(cubeMutex,INFINITE);
	LRCubes icubes(quitEvent);
	lrc = &icubes;
	//ReleaseMutex(cubeMutex);
	GaitArguments args = parseArguments(argc, argv);
	UINT trialList[128];	
	UINT numTrials = loadTrialList(args.trialsFilename, trialList);
	printf("base file name: %s\ntrials file name: %s\nSubject Number: %d\n",args.basefilename,args.trialsFilename,args.subjectNumber);

	HRESULT hr;
	InitCommonControls();
	CComModule	g_oATLModule;
	// Entering primary STA
	hr = CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hr));

	Metronome metronome;
	mp = &metronome;
	metronome.setupExperiment(numTrials, trialList, quitEvent, args.startTrial);
	metronome.setupMetronome(60,25.0);
	stopSavingEvent = metronome.m_metronomeDone;
	
	
	//winsock setup
	int iResult = 1;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2,0),&wsaData);
	if(iResult){
		printf("WSAStartup() failed with error %d\n",iResult);
		return iResult;
	}
	
	SOCKET servSocket = setupServerSocket(DEFAULT_PORT);
	if(servSocket<0)
		return servSocket;
	SOCKET tabletSocket = SOCKET_ERROR;
	MySockType *tablet = new MySockType;
	tablet->clientSocket = tabletSocket;
	tablet->servSocket = servSocket;
	printf("sockets initialized\n");	
	
	//zANT needs to destruct before calling CoUninitialize, hence the brackets. TODO: change this to be more elegant
	{
		gExperiment myzANT;	
		myzANT.cdfs.session.subjectNumber = args.subjectNumber;
		myzANT.setFileName(args.basefilename);
		
		hr = myzANT.InitAcquisitionObject();
		//printf("hr = 0x%x\n",hr);
		//printf("hWnd %d\n", myzANT.hWnd());	
				
		tablet->hWnd = myzANT.hWnd();
		tcpThread = (HANDLE)_beginthreadex(NULL,0,socketThread,(void *)tablet,0,0);
		myzANT.socket.servSocket = tablet->servSocket;
		myzANT.socket.hWnd = tablet->hWnd;
		gp = &myzANT;
		
		MSG msg;
		BOOL bRet;			
		char buff[256];

		while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
		{ 
		
			if (bRet == -1)
			{
				// handle the error and possibly exit
				printf("error, bRet == -1\n");
				break;
			}
			else if(bRet == 0){
				printf("got quit message\n");
				break;
			}
			else
			{		
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
				int retval = myzANT.handleConsoleInput();
				if(retval==1)
					break;							
				DWORD result = WaitForSingleObject(stopSavingEvent,0);
				if(result == WAIT_OBJECT_0){
					int numbp = sprintf(buff,"Trial finished!");					
					printf("%s\n",buff);
					DWORD res = WaitForSingleObject(clientConnected, 0);
					if(res == WAIT_OBJECT_0){
						//printf("sending to tablet at socket #%d\n",gp->socket.clientSocket);
						int numbs = send(gp->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
						if(numbs==SOCKET_ERROR){
							printf("Error sending, %d\n",GetLastError());
						}
					}
					//stop saving, reset metronome
					//gp->toggleSaving(false);
					gp->closeFile();
					mp->m_currentTrial++;
					mp->reset();
				}
			}
		}		
		SetEvent(quitEvent);
		printf("waiting for threads to end...");		
		if(hMET != NULL)
			WaitForSingleObject(hMET,INFINITE);
		if(hCUBE != NULL)
			WaitForSingleObject(hCUBE, INFINITE);		
		if(tcpThread != NULL)
			WaitForSingleObject(tcpThread, INFINITE);
		/*/
		HANDLE h[2];
		h[0] = hMET;
		h[1] = hCUBE;
		//DWORD result = WaitForMultipleObjects(2, h, TRUE, INFINITE);
		/*/
		//WaitForSingleObject(h,INFINITE);
		printf("done!\n");
	} //end kludgy destruct forcing
	printf("outta there\n");
	CoUninitialize();
	CloseHandle(quitEvent);
	CloseHandle(clientConnected);
	delete tablet;
	WSACleanup();
	return 0;
}


GaitArguments parseArguments(int argc, char *argv[]){
	GaitArguments g;
	char buffer[256];
	int numbp;	
	g.subjectNumber = 26;
	g.startTrial = 0;
	strcpy_s(g.trialsFilename,"C:\\Users\\CNS Lab\\Documents\\Visual Studio 2010\\Projects\\GaitExperiment\\Release\\test.txt");	
	
	for(int i=1;i<argc;i++){
		char *c = argv[i];
		//if this is a designator
		if(c[0] == '-' && strlen(c)>1){			
			if((strstr(c,"s") || strstr(c, "subjectNumber")) && i<argc-1)
				g.subjectNumber = atoi(argv[++i]);			
			else if((strstr(c,"t") || strstr(c,"trialFileName")) && i<argc-1){
				i++;
				strcpy_s(g.trialsFilename,argv[i]);
			}
			else if((strstr(c,"n") || strstr(c,"startTrial")) && i<argc-1){
				g.startTrial = atoi(argv[++i]);
			}
		}
	}
	numbp = sprintf_s(buffer,"Sub%02dpart\0",g.subjectNumber);
	strcpy_s(g.basefilename, buffer);	
	return g;
}

UINT loadTrialList(char *filename, UINT trialList[]){
	std::ifstream fin;
	fin.open(filename);
	UINT numTrials;
	fin >> numTrials;
#ifdef DEBUG
	printf("numTrials: %d\n",numTrials);
#endif
	//*trialList = (UINT *)(malloc(numTrials*sizeof(UINT)));
	for(UINT i=0;i<numTrials;i++){
		fin >> trialList[i];	
#ifdef DEBUG
		printf("%d\n",trialList[i]);
#endif
	}
	return numTrials;
}

SOCKET setupServerSocket(char *port){
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s<0){
		printf("Could not create socket, ERROR: %d\n",WSAGetLastError());
		WSACleanup();
		return s;
	}
	u_long iMode=1; //iMode = 1 for non-blocking socket
	ioctlsocket(s,FIONBIO,&iMode);
	struct sockaddr_in servAddr;
	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(port));

	//bind to local address
	int iResult = bind(s,reinterpret_cast<const sockaddr*>(&servAddr),sizeof(servAddr));
	if(iResult < 0){
		printf("bind failed\n");
		WSACleanup();
		return iResult;
	}

	//mark socket so it will listen
	iResult = listen(s, SOMAXCONN);
	if(iResult < 0){
		printf("could not set socket to listen\n");
		closesocket(s);
		WSACleanup();
		return iResult;
	}

	return s;
}

int connectToClient(SOCKET s){
	struct sockaddr_in clientAddr;
	ZeroMemory(&clientAddr,sizeof(clientAddr));
	socklen_t clientAddrLen = sizeof(clientAddr);

	//accept a client socket
	int clientSocket = SOCKET_ERROR;
	printf("waiting for client to connect...\n");
	while(clientSocket == SOCKET_ERROR){
		clientSocket = accept(s, reinterpret_cast<sockaddr*>(&clientAddr),&clientAddrLen);
		DWORD result = WaitForSingleObject(quitEvent,0);
		if(result == WAIT_OBJECT_0)
			return result;
		Sleep(1);
	}
	if(clientSocket < 0){
		printf("could not connect to client\n");
		closesocket(s);
		WSACleanup();
		return -1;
	}
	printf("connected to client!\n");
	u_long iMode = 1;// 1 for non-blocking socket
	ioctlsocket(clientSocket, FIONBIO, &iMode);
	
	return clientSocket;

}

unsigned int __stdcall socketThread(void *params){
	MySockType *s = reinterpret_cast<MySockType *>(params);
	DWORD res = WaitForSingleObject(quitEvent,0);	
	while(true){		
		UINT r = handleTCPClient(s);		
		res = WaitForSingleObject(quitEvent,0);
		if(r == WAIT_OBJECT_0 || res == WAIT_OBJECT_0)
			break;			
	}
	return 0;
}
int handleTCPClient(MySockType *s){
	//MySockType *s = reinterpret_cast<MySockType *>(params);
	s->clientSocket = connectToClient(s->servSocket);
	if(s->clientSocket==SOCKET_ERROR || s->clientSocket == WAIT_OBJECT_0){
		printf("could not create socket, %d\n",s->clientSocket);		
		return s->clientSocket;
	}
	DWORD res = WaitForSingleObject(quitEvent,0);
	if(res == WAIT_OBJECT_0)
		return res;

	printf("created socket, using socket #%d\n", s->clientSocket);
	gp->socket.clientSocket = s->clientSocket;
	printf("gp socket set to %d\n",gp->socket.clientSocket);
	SetEvent(clientConnected);	
	char buffer[BUFFSIZE];
	//int numbp = sprintf(buffer,"Connected to experiment");
	//numbp = send(s->clientSocket,buffer,numbp+1,0);
	int numbr;
	//main loop
	
	while(true){
		numbr = recv(s->clientSocket,buffer,BUFFSIZE,0);
		if(numbr<BUFFSIZE)
			buffer[numbr]='\0';
		//quit if the program has terminated
		res = WaitForSingleObject(quitEvent,0);
		if(res == WAIT_OBJECT_0)
			return res;
		if(numbr==SOCKET_ERROR){
			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK && nError!=0){
				printf("Disconnecting due to winsock error code: %d\n",nError);				
				ResetEvent(clientConnected);
				gp->socket.clientSocket = INVALID_SOCKET;
				closesocket(s->clientSocket);
				return nError;
			}
		}		
		else{
			//if received a command from the tablet
			//post those messages to the relevant window
			for(int i=0;i<numbr;i++)
				PostMessage(s->hWnd,WM_KEYDOWN,buffer[i],NULL);
		}
		
	}//
	ResetEvent(clientConnected);
	gp->socket.clientSocket = INVALID_SOCKET;
	closesocket(s->clientSocket);

}