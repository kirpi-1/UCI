#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define DEFAULT_SERV_NAME "localhost"//"localhost"//"
#define DEFAULT_PORT_TCP "27015"
#define DEFAULT_PORT_UDP "27016"
#define sleepTime 1

SOCKET setupUDPServer(char *port);
unsigned int handleUDPClient(SOCKET s);

int main(int argc, char* argv[]){
	
	SYSTEMTIME st;
	GetLocalTime(&st);
	printf("year:\t%d\nmonth:\t%d\nday:\t%d\nhour:\t%d\nmin:\t%d\nsec:\t%d\nms:\t%d\n",
		st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);

	
	
	
	
	
	int iret = 1;
	WSADATA	wsaData;

	iret = WSAStartup(MAKEWORD(2,0), &wsaData);
	if(iret){
		cout << "Startup failed\n";
		return iret;
	}
	cout << "initialized, iret = "<< iret << endl;
	SOCKET udpServer = setupUDPServer(DEFAULT_PORT_UDP);
	printf("socket #%d\n",udpServer);
	handleUDPClient(udpServer);
	return 0;
}

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
unsigned int handleUDPClient(SOCKET s){

	struct sockaddr_storage clientAddr;
	memset(&clientAddr,0,sizeof(clientAddr));
	socklen_t addrlen = sizeof(clientAddr);

	int numbr;
	char clientBuffer[512];
	memset(clientBuffer,0,512);
	DWORD res;
	printf("Waiting for client\n");
	while(true){
		numbr = recvfrom(s,clientBuffer,512,0,(struct sockaddr *)&clientAddr,&addrlen);
		if(numbr==SOCKET_ERROR){
			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK && nError!=0){
				printf("UDP Disconnecting due to winsock error code: %d\n",nError);				
				closesocket(s);
				WSACleanup();
				return nError;
			}
		}
		else{ //received data from socket
			printf("received frame %d\n",reinterpret_cast<int*>(clientBuffer)[0]);
			
			
		}//end else
		//quit if the program has terminated

	}//
	printf("udp done\n");
	return 0;
}