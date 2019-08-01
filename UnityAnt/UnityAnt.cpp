// UnityAnt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "uANT.h"



#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

#define BUFFSIZE 512
const int REQ_WINSOCK_VER = 2;
char *DEFAULT_PORT = "27015";
char headerBuffer[512];
const int BUFFER_SIZE = 128;

extern uANT *uap;

//unsigned __stdcall testThread(void *pArgs);
HANDLE quitEvent;
HANDLE stopSavingEvent;
HANDLE tcpThread;
HANDLE clientConnected;
HANDLE headerMutex;
HANDLE hMET = NULL, hCUBE = NULL;


struct ExperimentArguments{
	char basefilename[256];
	char trialsFilename[256];
	UINT subjectNumber;
	UINT startTrial;
};


// **************** Function declarations ****************
SOCKET setupServerSocket(char *port);
int connectToClient(SOCKET s);
UINT loadTrialList(char *filename, UINT trialList[]);
int handleTCPClient(MySockType *s);
unsigned int __stdcall socketThread(void *params);
int parseInput(char *buffer, unsigned int buffsize, HWND hWnd);
// **************** Function declarations ****************


int main(int argc, char *argv[]){
	quitEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("QuitEvent"));
	ResetEvent(quitEvent);
	clientConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("Client Connected"));
	headerMutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
	//ReleaseMutex(cubeMutex);
	
	ExperimentArguments args;// = parseArguments(argc, argv);
	args.subjectNumber = 26;
	strcpy_s(args.basefilename, "sub26");
	args.startTrial = 0;

	/*
	UINT trialList[128];	
	UINT numTrials = loadTrialList(args.trialsFilename, trialList);
	printf("base file name: %s\ntrials file name: %s\nSubject Number: %d\n",args.basefilename,args.trialsFilename,args.subjectNumber);
	*/
	HRESULT hr;
	InitCommonControls();
	CComModule	g_oATLModule;

	// Entering primary STA
	hr = CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hr));

	//stopSavingEvent = metronome.m_metronomeDone;
	
	
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
		uANT myzANT;	
		myzANT.cdfs.session.subjectNumber = args.subjectNumber;
		myzANT.setFileName(args.basefilename);
		
		hr = myzANT.InitAcquisitionObject();
		//printf("hr = 0x%x\n",hr);
		//printf("hWnd %d\n", myzANT.hWnd());	
				
		tablet->hWnd = myzANT.hWnd();
		tcpThread = (HANDLE)_beginthreadex(NULL,0,socketThread,(void *)tablet,0,0);
		myzANT.socket.servSocket = tablet->servSocket;
		myzANT.socket.hWnd = tablet->hWnd;
		uap = &myzANT;
		
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
						int numbs = send(uap->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
						if(numbs==SOCKET_ERROR){
							printf("Error sending, %d\n",GetLastError());
						}
					}
					//stop saving, reset metronome
					//gp->toggleSaving(false);
					uap->closeFile();					
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
	uap->socket.clientSocket = s->clientSocket;
	printf("uap socket set to %d\n",uap->socket.clientSocket);
	SetEvent(clientConnected);	
	char *clientBuffer= new char[BUFFSIZE];
	int numbr;
	//int numbp = sprintf(buffer,"Connected to experiment");
	//numbp = send(s->clientSocket,buffer,numbp+1,0);
	
	//main loop
	while(true){
		//quit if the program has terminated
		res = WaitForSingleObject(quitEvent,0);
		if(res == WAIT_OBJECT_0){
			delete[] clientBuffer;
			return res;
		}
		numbr = recv(s->clientSocket,clientBuffer,BUFFSIZE,0);
		if(numbr<BUFFSIZE)
			clientBuffer[numbr]='\0';
		if(numbr==SOCKET_ERROR){
			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK && nError!=0){
				printf("Disconnecting due to winsock error code: %d\n",nError);				
				ResetEvent(clientConnected);
				uap->socket.clientSocket = INVALID_SOCKET;
				closesocket(s->clientSocket);
				delete[] clientBuffer;
				return nError;
			}
		}		
		else{
			PostMessage(s->hWnd,WM_KEYDOWN,clientBuffer[0],NULL);
			/*
			if(parseInput(buffer, numbr, s->hWnd)<0)
				break;
				*/
		}
		
	}
	printf("closing connection\n");
	ResetEvent(clientConnected);
	uap->socket.clientSocket = INVALID_SOCKET;
	closesocket(s->clientSocket);
	delete[] clientBuffer;
	return 0;
}

int parseInput(char *buffer, unsigned int buffsize, HWND hWnd){
	unsigned int idx = 0;
	char intSize[sizeof(unsigned int)];
	char message[BUFFSIZE];
	printf("buffer is %d bytes long\n",buffsize);
	printf("\"%s\"\n",buffer);
	while(idx < buffsize){
		DWORD res = WaitForSingleObject(quitEvent,0);
		if(res == WAIT_OBJECT_0)
			return res;
		if(buffer[idx] == '\0'){
			idx++;
			continue;
		}
		//- is an escape character
		if(buffer[idx] == '-'){
			printf("received escape character...");
			//get datatype
			idx++;
			//text
			if(buffer[idx] == 't'){
				//text strings are -t<string length as unsigned int><character array>
				printf("received a text string!\n");
				idx++;
				memcpy(intSize,buffer+idx,sizeof(unsigned int));
				idx+=sizeof(unsigned int);
				unsigned int messageLength = *(reinterpret_cast<unsigned int *>(intSize));
				//if message length is less than buffer, all is good, copy it
				if(messageLength<BUFFSIZE){
					memcpy(message,buffer+idx,sizeof(char)*messageLength);
					message[messageLength] = '\0'; //make sure last character is null					
				}
				//if message length is too long, take first BUFFSIZE(512)-3 characters, add a "..." to indicate there is more, throw out the rest of the message
				else{
					memcpy(message,buffer+idx,sizeof(char)*BUFFSIZE);
					memcpy(message+BUFFSIZE-3,"...",sizeof(char)*3);
					message[BUFFSIZE] = '\0'; //make sure last character is null										
				}
				idx+=messageLength;
				printf("%s\n",message);
			}
			
		}
		//if received a command from the tablet
		//post those messages to the relevant window
		else{
			printf("posting \"%c\" to window\n",buffer[idx]);
			char c = buffer[idx];
			PostMessage(hWnd,WM_KEYDOWN,c,NULL);			
			idx++;
		}		
	}//end while idx < buffsize
	return 0;
}