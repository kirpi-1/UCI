#include "stdafx.h"

#include "uANT.h"
#include <process.h>
#include <fstream>

#include <stdio.h> 
#include <stdlib.h> 
#include <memory.h>
#include <string.h>
#include <conio.h>
#include "UnityAnt.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

extern uANT *uap;

//unsigned __stdcall testThread(void *pArgs);

//for connecting to client, for whatever reason this needs to be here so that
//the stack doesn't collapse around it
//it used to be in handleTCPClient() but it was giving errors.  

//char clientBuffer[BUFFSIZE];

int main(int argc, char *argv[]){
	quitEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("QuitEvent"));
	ResetEvent(quitEvent);
	gotNewFrameHeaderData = CreateEvent(NULL, TRUE, FALSE, TEXT("Got new frame header data"));
	gotNewHMDData = CreateEvent(NULL,TRUE,FALSE,TEXT("Got new HMD data"));
	wroteFrameHeaderData = CreateEvent(NULL, TRUE, TRUE, TEXT("write frame header"));
	clientConnected = CreateEvent(NULL, TRUE, FALSE, TEXT("Client Connected"));
	frameBufferMutex = CreateMutex( 
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
	HMDDataMutex = CreateMutex( 
    NULL,              // default security attributes
    FALSE,             // initially not owned
    NULL);             // unnamed mutex
	//printf("testing inertia cubes\n");
	//DWORD result = WaitForSingleObject(cubeMutex,INFINITE);
	//ReleaseMutex(cubeMutex);

	ExperimentArguments args = parseArguments(argc, argv);
	int trialList[512] = {0};
	//UINT numTrials = loadTrialList(args.trialsFilename,trialList);

	HRESULT hr;
	InitCommonControls();
	CComModule	g_oATLModule;
	// Entering primary STA
	hr = CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hr));
	
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
	
	SOCKET udpServerSocket = setupUDPServer(UDP_PORT);
	if(udpServerSocket < 0)
		return udpServerSocket;
	
	SOCKET tabletSocket = SOCKET_ERROR;
	MySockType *tablet = new MySockType;
	tablet->clientSocket = tabletSocket;
	tablet->servSocket = servSocket;
	printf("sockets initialized\n");
	
	
	//zANT needs to destruct before calling CoUninitialize, hence the brackets. TODO: change this to be more elegant
	{
		//set up recording session
		uANT myzANT;	
		myzANT.subNum= args.subjectNumber;
		myzANT.trialNum = args.startTrial;
		myzANT.setFileName("default");
		myzANT.setTrialList(trialList,512);
		myzANT.toggleNullData(true);
		
		hr = myzANT.InitAcquisitionObject();
		//printf("hr = 0x%x\n",hr);
		//printf("hWnd %d\n", myzANT.hWnd());	

		tablet->hWnd = myzANT.hWnd();
		
		tcpThread = (HANDLE)_beginthreadex(NULL,0,socketThread,(void *)tablet,0,0);
		//udpThread = (HANDLE)_beginthreadex(NULL,0,handleUDPClient,(void *)&udpServerSocket,0,0);
		hmdThread = (HANDLE)_beginthreadex(NULL,0,HMDThread,(void *)&udpServerSocket,0,0);
		myzANT.socket.servSocket = servSocket;
		myzANT.socket.hWnd = myzANT.hWnd();
		
		uap = &myzANT;
		
		MSG msg;
		BOOL bRet;			
		char buff[256];

		printf("starting main loop\n");
		
		while((bRet = GetMessage( &msg,NULL, 0, 0 )) != 0)
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
					int numbp = sprintf_s(buff,256,"Trial finished!");					
					printf("%s\n",buff);
					uap->closeFile();
				}
			}
		}//end while
		SetEvent(quitEvent);
		printf("waiting for threads to end...");		
		//DeleteTimerQueueTimer(NULL,keyboardThread,INVALID_HANDLE_VALUE);
		if(tcpThread != NULL){
			printf("waiting on tcpThread...");
			WaitForSingleObject(tcpThread, INFINITE);
			printf("tcpthread done!\n");
		}
		if(hmdThread != NULL){
			printf("waiting on HMD thread...");
			WaitForSingleObject(hmdThread, INFINITE);
			printf("HMD thread done!\n");
		}
		printf("Program End\n");
	} //end kludgy destruct forcing
	//printf("outta there\n");
	CoUninitialize();
	CloseHandle(quitEvent);
	CloseHandle(clientConnected);
	delete tablet;
	WSACleanup();
	return 0;
}


ExperimentArguments parseArguments(int argc, char *argv[]){
	ExperimentArguments g;
	char buffer[256];
	int numbp;	
	g.subjectNumber = 26;
	g.startTrial = 0;
	strcpy_s(g.trialsFilename,".\\test.txt");	
	for(int i=1;i<argc;i++){
		char *c = argv[i];
		//if this is a designator
		if(c[0] == '-' && strlen(c)>1){			
			if((strstr(c,"s") || strstr(c, "subjectNumber")) && i<argc-1)
				g.subjectNumber = atoi(argv[++i]);			
			else if((strstr(c,"n") || strstr(c,"trialFileName")) && i<argc-1){
				i++;
				strcpy_s(g.trialsFilename,argv[i]);
			}
			else if((strstr(c,"t") || strstr(c,"startTrial")) && i<argc-1){
				g.startTrial = atoi(argv[++i]);
			}
		}
	}
	numbp = sprintf_s(buffer,"Sub%02d_trial\0",g.subjectNumber);
	strcpy_s(g.basefilename, buffer);	
	return g;
}

UINT loadTrialList(char *filename, int trialList[]){
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
	MySockType *in = reinterpret_cast<MySockType *>(params);
	MySockType *s;
	DWORD res = WaitForSingleObject(quitEvent,0);	
	int r=-1;
	while(true){
		s = new MySockType;
		s->servSocket = in->servSocket;
		s->hWnd = in->hWnd;
		s->clientSocket = connectToClient(s->servSocket);
		if(s->clientSocket==SOCKET_ERROR){
			printf("could not create socket, %d\n",s->clientSocket);		
		}
		else if(s->clientSocket == WAIT_OBJECT_0){
			printf("program ending, closing socket thread\n");
			break;
		}
		else{
			printf("created socket #%d\n", s->clientSocket);
			_beginthreadex(NULL,0,handleTCPClient,(void *)s,0,0);
		}
		//r = handleTCPClient(s);		
		//printf("socketThread() now checking quit signal\n");
		res = WaitForSingleObject(quitEvent,0);
		//if(r == WAIT_OBJECT_0 || res == WAIT_OBJECT_0)
		if(res == WAIT_OBJECT_0){
			printf("socketThread() leaving\n");
			break;			
		}
	}
	return 0;
}
unsigned int __stdcall handleTCPClient(void *params){
	MySockType *s = reinterpret_cast<MySockType *>(params);
	DWORD ret = -1;
	MessageType type = NONE;
	int messageSize = -1;
	unsigned int leftToRead = 0;
	unsigned int writeIdx = 0;
	BYTE *buffer = new BYTE[sizeof(float)*1024];
	bool messageReady = false;

	std::string clientMessage = "";
	SetEvent(clientConnected);	
	
	//int numbp = sprintf(buffer,"Connected to experiment");
	//numbp = send(s->clientSocket,buffer,numbp+1,0);
	int numbr;
	char clientBuffer[BUFFSIZE];
	//****************************** main loop ******************************
	bool quit = false;
	DWORD res;
	while((res = WaitForSingleObject(quitEvent,NULL)) != WAIT_OBJECT_0){

		numbr = recv(s->clientSocket,clientBuffer,BUFFSIZE,0);
		
		if(numbr==SOCKET_ERROR){
			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK && nError!=0){
				printf("Disconnecting due to winsock error code: %d\n",nError);				
				ResetEvent(clientConnected);
				uap->socket.clientSocket = INVALID_SOCKET;
				closesocket(s->clientSocket);
				return nError;
			}
		}
		else if(numbr==0){ //gracefully disconnected
			break;
		}
		else{ //received data from socket
			parseInput(clientBuffer, numbr, buffer, type, messageReady, messageSize, writeIdx);
		}//end else
		//quit if the program has terminated

	}//
	//printf("quitting handle client() 2\n");
	ResetEvent(clientConnected);
	uap->socket.clientSocket = INVALID_SOCKET;
	closesocket(s->clientSocket);
	printf("client disconnected\n");
	delete s;
	return ret;
}
SOCKET setupUDPServer(char *port){
	SOCKET s;	
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;

	getaddrinfo(NULL, port,&hints,&servinfo);
	int ret;
	for(p=servinfo;p!=NULL;p=p->ai_next){
		s = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
		if(s==-1)
			continue;
		ret = bind(s,p->ai_addr,p->ai_addrlen);
		if(ret == -1){
			closesocket(s);
			continue;
		}
		break;
	}

	if(p == NULL){
		DWORD err = WSAGetLastError();
		printf("coud not create UDP socket, error %d\n", err);
		WSACleanup();
		return -1;
	}
	freeaddrinfo(servinfo);

	u_long iMode=1; //iMode = 1 for non-blocking socket
	ioctlsocket(s,FIONBIO,&iMode);
	
	return s;
}
unsigned int __stdcall handleUDPClient(void *params){
	SOCKET *s = reinterpret_cast<SOCKET *>(params);
	DWORD ret = -1;
	MessageType type = NONE;
	int messageSize = -1;
	unsigned int leftToRead = 0;
	unsigned int writeIdx = 0;
	BYTE *buffer = new BYTE[sizeof(float)*1024];
	bool messageReady = false;
	struct sockaddr_storage clientAddr;
	memset(&clientAddr,0,sizeof(clientAddr));
	socklen_t addrlen = sizeof(clientAddr);

	int numbr;
	char clientBuffer[BUFFSIZE];
	memset(clientBuffer,0,BUFFSIZE);
	DWORD res;
	while((res = WaitForSingleObject(quitEvent,NULL)) != WAIT_OBJECT_0){
		numbr = recvfrom(*s,clientBuffer,BUFFSIZE,0,(struct sockaddr *)&clientAddr,&addrlen);
		if(numbr==SOCKET_ERROR){
			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK && nError!=0){
				printf("UDP Disconnecting due to winsock error code: %d\n",nError);				
				closesocket(*s);
				return nError;
			}
		}
		else{ //received data from socket
			printf("received %d bytes of data over UDP\n", numbr);
			float floatBuffer[5];
			memcpy(floatBuffer,clientBuffer+2+sizeof(unsigned int),5*sizeof(float));
			for(int i=0;i<5;i++)
				printf("\t%f\n",floatBuffer[i]);
			//parseInput(clientBuffer, numbr, buffer, type, messageReady, messageSize, writeIdx);
		}//end else
		//quit if the program has terminated

	}//
	printf("udp done\n");
	return 0;
}
unsigned int __stdcall HMDThread(void *params){
	SOCKET *s = reinterpret_cast<SOCKET *>(params);
	struct sockaddr_storage clientAddr;
	memset(&clientAddr,0,sizeof(clientAddr));
	socklen_t addrlen = sizeof(clientAddr);
	bool started = false;
	int numbr;
	char clientBuffer[BUFFSIZE];
	memset(clientBuffer,0,BUFFSIZE);
	DWORD res;
	while((res = WaitForSingleObject(quitEvent,NULL)) != WAIT_OBJECT_0){
		numbr = recvfrom(*s,clientBuffer,BUFFSIZE,0,(struct sockaddr *)&clientAddr,&addrlen);
		if(numbr==SOCKET_ERROR){
			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK && nError!=0){
				printf("UDP Disconnecting due to winsock error code: %d\n",nError);				
				closesocket(*s);
				return nError;
			}
		}
		else{ //received data from socket
			if(!started){
				printf("receiving data from HMD!\n");
				started = true;
			}
			float floatBuffer[HMD_DATA_SIZE];
			DWORD result = WaitForSingleObject(HMDDataMutex,NULL);
			if(result == WAIT_OBJECT_0){
				memcpy(HMDDataBuffer,clientBuffer,HMD_DATA_SIZE*sizeof(float));
				SetEvent(gotNewHMDData);
				ReleaseMutex(HMDDataMutex);
			}
		}//end else
		//quit if the program has terminated

	}//
	printf("HMD thread done\n");
	return 0;
}
unsigned int getMessageSize(char *buffer){
	char intSize[sizeof(unsigned int)];
	memcpy(intSize,buffer,sizeof(unsigned int));
	return *(reinterpret_cast<unsigned int *>(intSize));
}

unsigned int getMessageIntegers(int *out, char *buffer, unsigned int size){
	memcpy(out,buffer,sizeof(int)*size);
	return 0;
}

unsigned int parseInput(char clientBuffer[], unsigned int numbr, BYTE buffer[], MessageType &type, bool &messageReady, int &messageSize, unsigned int &writeIdx){
	unsigned int idx = 0;
	char *charBuffer = reinterpret_cast<char *>(buffer);
	int *intBuffer = reinterpret_cast<int *>(buffer);
	float *floatBuffer = reinterpret_cast<float *>(buffer);
	double *doubleBuffer = reinterpret_cast<double *>(buffer);


	while(idx<numbr){
		if(type == NONE){ //if don't have message type yet
			if(clientBuffer[idx] == '-'){
				//printf("received escape character...");
				type = UA_ESCAPE;
				//get datatype

			}//end if received escape character
			/*
			//skip null characters
			else if(clientBuffer[idx] != '\0'){
				//printf("posting message\n");
				PostMessage(s->hWnd,WM_KEYDOWN,clientBuffer[idx],NULL);					
			}
			*/
			idx++;
		}//end if type == NONE
		else if(type == UA_ESCAPE){
			if(clientBuffer[idx] == 't'){
					//text strings are -t<string length as unsigned int><character array>
					//printf("receiving text string!\n");
					type = UA_TEXT;
					idx++;
			}
			else if(clientBuffer[idx] == 'c'){
				type = UA_CHAR;
				idx++;
			}
			else if(clientBuffer[idx] == 'i'){
				//printf("receiving integers\n");
				type = UA_INT;
				idx++;
			}
			else if(clientBuffer[idx] == 'f'){
				printf("receiving floats\n");
				type = UA_FLOAT;
				idx++;
			}
			else if(clientBuffer[idx] == 'd'){
				//printf("receiving doubles\n");
				type = UA_DOUBLE;
				idx++;
			}
		}
		else if(type == UA_TEXT){					
			//receive until reach '\0'
			charBuffer[writeIdx] = clientBuffer[idx];
			if(clientBuffer[idx]=='\0'){
				messageReady = true;
				//printf("finished receiving the string\n");
			}
			writeIdx++;
			idx++;
		}
		else if(type == UA_CHAR){
			PostMessage(uap->hWnd(),WM_KEYDOWN,clientBuffer[idx],NULL);
			writeIdx = 0;
			messageSize = -1;
			type = NONE;
			messageReady = false;
			idx++;
		}
		else{
			//if don't have message size yet, get it
			if(messageSize < 0){
				messageSize = getMessageSize(clientBuffer+idx);
				idx+=sizeof(unsigned int);
			}
			else{							
				if(type == UA_INT){
					intBuffer[writeIdx] =*(reinterpret_cast<int *>(clientBuffer+idx));							
					//std::cout << "\t" << intBuffer[writeIdx] << std::endl;
					writeIdx++;
					idx+=sizeof(int);
				}
				else if(type == UA_FLOAT){
					floatBuffer[writeIdx] = *(reinterpret_cast<float *>(clientBuffer+idx));
					std::cout << "\t" << floatBuffer[writeIdx] << std::endl;
					writeIdx++;
					idx+=sizeof(float);
				}
				else if(type == UA_DOUBLE){
					doubleBuffer[writeIdx] = *(reinterpret_cast<double *>(clientBuffer+idx));
					//std::cout << "\t" << doubleBuffer[writeIdx] << std::endl;
					writeIdx++;
					idx+=sizeof(double);
				}
				//check if received full buffer
				if(writeIdx >= messageSize)							
					messageReady = true;
			}//end else you have message size

		}//end if type != NONE
		if(messageReady){
			unsigned int headerSize;
			void *src;
			if(type==UA_TEXT){
				headerSize = sizeof(char)*writeIdx;
				src = charBuffer;
				//printf("received: %s\n",charBuffer);
			}
			else if(type==UA_INT){
				headerSize = sizeof(int)*messageSize;
				src = intBuffer;
			}
			else if(type==UA_FLOAT){
				headerSize = sizeof(float)*messageSize;
				src = floatBuffer;
			}
			else if(type==UA_DOUBLE){
				headerSize = sizeof(double)*messageSize;
				src = doubleBuffer;
			}
			do{

				DWORD result = WaitForSingleObject(wroteFrameHeaderData,NULL);
				if(result == WAIT_OBJECT_0){
					if(frameBuffer != NULL)
						delete[] frameBuffer;
					frameBuffer = new char[headerSize];
					frameBufferSize = headerSize;
					memcpy(frameBuffer,src,headerSize);
					ResetEvent(wroteFrameHeaderData);
					SetEvent(gotNewFrameHeaderData);
					//printf("copied frame buffer\n");
					break;
				}
				result = WaitForSingleObject(quitEvent,NULL);
				if(result == WAIT_OBJECT_0){
					break;
				}
			}while(1);
			writeIdx = 0;
			messageSize = -1;
			type = NONE;
			messageReady = false;
		}


	}//end while still have buffer to analyze (idx < numbr)
	return 0;
}