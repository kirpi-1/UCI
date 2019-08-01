// ExperimentBase.cpp : Defines the entry point for the console application.
//
#include "ExperimentBase.h"
#include "subANT.h"

#include <process.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_SERVER_PORT "27015"
#define BUFFSIZE 512
//#include <stdio.h> 
//#include <stdlib.h> 
//#include <memory.h>
//#include <string.h>
//#include <conio.h>



int myMain(int argc, _TCHAR* argv[]);
SOCKET setupServerSocket(PCSTR port);
unsigned int __stdcall socketThread(void *params);
int handleTCPClient(MySockType *s);

extern subANT *sp;
HANDLE quitEvent, clientConnected;
HANDLE tcpThread;

int _tmain(int argc, _TCHAR* argv[])
{
	quitEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("QuitEvent"));
	ResetEvent(quitEvent);
	clientConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("Client Connected"));
	ResetEvent(clientConnected);

	HRESULT hr;
	InitCommonControls();
	CComModule	g_oATLModule;
	// Entering primary STA
	hr = CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hr));
	
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult !=0){
		printf("WSAStartup() failed: %d\n",iResult);
		return 1;
	}

	SOCKET server = setupServerSocket(DEFAULT_SERVER_PORT);


	// wait for client to connect
	SOCKET client = INVALID_SOCKET;

	// begin client thread
	// accept client
	MySockType myClient;
	tcpThread = (HANDLE)_beginthreadex(NULL,0,socketThread,(void *)&myClient,0,0);
	myMain(argc, argv);
	
	CoUninitialize();
	return 0;
}

int myMain(int argc, _TCHAR* argv[]){
	subANT myANT;
	myANT.InitAcquisitionObject();
	//sp = &myANT;

	myANT.cdfs.setSubNum(26);	
	myANT.cdfs.open("subANTtest.cdf");
	
	// wait for the client to connect or for the user to confirm
	while(true){
		if(_kbhit()){
			char c = _getch();				
			if(c=='d' || c=='D'){
				printf("using default amplifier values\n");
				break;
			}
		}
		DWORD res = WaitForSingleObject(clientConnected,0);
		if(res == WAIT_OBJECT_0)
			break;
	}

	// main loop starts here
	MSG msg;
	BOOL bRet;
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0){ 		
		//printf("bRet = %d\n",bRet);	
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
			myANT.handleConsoleInput();
		}
	}
	myANT.cdfs.close();
	return 0;
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



SOCKET setupServerSocket(PCSTR port){
	int iResult;

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//getaddrinfo
	iResult = getaddrinfo(NULL,port,&hints,&result);
	if(iResult!=0){
		printf("getaddrinfo failed %d\n",WSAGetLastError());
		return INVALID_SOCKET;
	}

	//setup socket
	SOCKET listenSocket = INVALID_SOCKET;
	listenSocket = socket(result->ai_family,result->ai_socktype, result->ai_protocol);

	if(listenSocket==INVALID_SOCKET){
		printf("failed to create listening socket(): %d\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return INVALID_SOCKET;
	}

	// set socket to non-blocking mode
	u_long iMode=1; //iMode = 1 for non-blocking socket
	ioctlsocket(listenSocket,FIONBIO,&iMode);


	//bind socket
	iResult = bind(listenSocket, result->ai_addr,(int)result->ai_addrlen);
	if(iResult == SOCKET_ERROR){
		printf("bind() failed with error: %d\n",WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);

	//mark socket for listening
	iResult = listen(listenSocket,SOMAXCONN);
	if(iResult == SOCKET_ERROR){
		printf("listen() failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	return listenSocket;
}


int connectToClient(SOCKET s){
	//struct sockaddr_in clientAddr;
	//ZeroMemory(&clientAddr,sizeof(clientAddr));
	//socklen_t clientAddrLen = sizeof(clientAddr);

	//accept a client socket
	int client = SOCKET_ERROR;
	printf("waiting for client to connect...\n");
	while(client == SOCKET_ERROR){
		client = accept(s, NULL, NULL);
		DWORD result = WaitForSingleObject(quitEvent,0);
		if(result == WAIT_OBJECT_0)
			return result;
		Sleep(1);
	}
	if(client < 0){
		printf("could not connect to client\n");
		closesocket(s);
		WSACleanup();
		return -1;
	}
	printf("connected to client!\n");
	u_long iMode = 1;// 1 for non-blocking socket
	ioctlsocket(client, FIONBIO, &iMode);
	
	return client;

}


int handleTCPClient(MySockType *s){	
	s->clientSocket = connectToClient(s->servSocket);
	if(s->clientSocket==SOCKET_ERROR || s->clientSocket == WAIT_OBJECT_0){
		printf("could not create socket, %d\n",s->clientSocket);		
		return s->clientSocket;
	}
	DWORD res = WaitForSingleObject(quitEvent,0);
	if(res == WAIT_OBJECT_0)
		return res;

	printf("created socket, using socket #%d\n", s->clientSocket);
	sp->socket.clientSocket = s->clientSocket;
	printf("sp socket set to %d\n",sp->socket.clientSocket);
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
				sp->socket.clientSocket = INVALID_SOCKET;
				closesocket(s->clientSocket);
				return nError;
			}
		}		
		else{
			//if received a command from the tablet
			//post those messages to the relevant window
			for(int i=0;i<numbr;i++)
				PostMessage(sp->hWnd(),WM_KEYDOWN,buffer[i],NULL);
		}
		
	}//
	ResetEvent(clientConnected);
	sp->socket.clientSocket = INVALID_SOCKET;
	closesocket(s->clientSocket);

}