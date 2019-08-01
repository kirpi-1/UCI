#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string>
#include "marvinTestClient.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main(int argc, char* argv[]){
	char *servName = DEFAULT_SERV_NAME;
	char *port = DEFAULT_PORT;
	bool constantDrive = false;
	bool turboMode = false;
	for(int i=1;i<argc;i++){
		cout << "arg " << i << endl;
		if(!strcmp(argv[i],"-a") || !strcmp(argv[i],"-address")){
			if(i+1<argc)
				servName = argv[i+1];
			else{
				cout << "Need a server address with -a/-address flag\n";
				return -1;
			}
		}
		else if(!strcmp(argv[i],"-p") || !strcmp(argv[i],"-port")){
			if(i+1<argc)
				port = argv[i+1];
			else{
				cout << "Need a port number with -p/-port flag\n";
				return -1;
			}
		}
		else if(!strcmp(argv[i],"-f") || !strcmp(argv[i],"-forward"))
			constantDrive = true;
		else if(!strcmp(argv[i],"-t") || !strcmp(argv[i],"-turbo"))
			turboMode = true;
	}

	int iret = 1;
	WSADATA	wsaData;

	iret = WSAStartup(MAKEWORD(2,0), &wsaData);
	if(iret){
		cout << "Startup failed\n";
		return iret;
	}	
	cout << "initialized\n";

	SOCKET hSocket = INVALID_SOCKET;
	//char   buff[BUFFSIZE];
	
	//create socket
	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(hSocket < 0){
		cout << "could not create socket\n";
		WSACleanup();
		return 1;
	}
	
	//create server address
	
	struct addrinfo *result;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints)); //VERY IMPORTANT
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;


	iret = getaddrinfo(servName, port, &hints, &result);
	if(iret!=0){
		cout << "getaddrinfo() failed\n";
		cout << "error " << iret << endl;
		WSACleanup();
		return iret;
	}
	/*
	sockaddr_in sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(port);	
	//sockAddr.sin_addr.S_un.S_addr = result->ai_addr
	
	HOSTENT *pHostent;
	pHostent = gethostbyname(servName);
	if(!pHostent){
		cout << "could not resolve hostname\n";
		return -1;
	}
	
	//extract primary IP address from hostent structure
	if(pHostent->h_addr_list && pHostent->h_addr_list[0])
		sockAddr.sin_addr.S_un.S_addr = *reinterpret_cast<unsigned long*>(pHostent->h_addr_list[0]);
	*/
	//using this, servName has to be in "x.x.x.x" format
	/*
	iret = InetPton(AF_INET,servName, &sockAddr.sin_addr.S_un.S_addr);
	if(iret==0){
		cout << "invalid address string\n";
		return iret;
	}
	else if(iret<0){
		cout << "error with InetPton()\n";
		return iret;
	}
	*/
	
	
	cout << "connecting...\n";
	//connect to socket
	if(connect(hSocket, result->ai_addr, result->ai_addrlen) < 0){
		cout << "connect() failed\n";
		WSACleanup();
		return 1;
	}
	cout << "connected\n";
	//char *echoMessage;
	string base = "";	
	//string input;
	//cin >> input;
	char input;
	bool gotInput = false;
	if(constantDrive || turboMode){
		if(constantDrive)
			base = "FFF";
		if(_kbhit()){
			gotInput = true;
			input = char(_getch());
			base = base + input;
		}
	}
	else{
		cin >> base;
		gotInput = true;
	}
	//echoMessage = &base;
	//int i=0;
	cout << base << endl;
	bool receiving = false;
	MyRobotTelemetry myRT;
	char *inputBuffer=new char[128];
	int numBytes = 0;
	while(true){
		/*
		echoMessage="w";
		if(i==10)
			echoMessage="x";
		i++;
		*/
		//unsigned int echoStringLen = strlen(echoMessage);
		/*
		if(input=="stop")
			input = " ";
		unsigned int len = input.length();
		*/
		if(constantDrive || gotInput){
			gotInput = false;
			cout << base << " is length " << base.length() << endl;
			numBytes = send(hSocket, (base.c_str()), base.length(), 0);
			if(numBytes<0){
				cout << "send failed\n";
				WSACleanup();
				return numBytes;
			}
			else if(numBytes != base.length()){
				cout << "sent an unexpected number of bytes\n";
				WSACleanup();
				return numBytes-base.length();
			}
			cout << "sent " << base << " successfully\n";
			if(input=='d' || input=='D' || input=='q' || input=='Q' ||
				base.find("d")!=string::npos || base.find("D")!=string::npos || 
				base.find("Q")!=string::npos || base.find("q")!=string::npos)			
				break;
			if(input=='t' || input=='T' ||
				base.find("t")!=string::npos ||
				base.find("T")!=string::npos){
				int numbr = recv(hSocket,inputBuffer,sizeof(MyRobotTelemetry),0);
				if(numbr!=sizeof(MyRobotTelemetry))
					cout << "wtf!\n";
				myRT = *(reinterpret_cast<MyRobotTelemetry *>(inputBuffer));
				cout << "vel:\t" << myRT.vel << endl
					 << "rotVel:\t"<<myRT.rotVel << endl
					 << "voltage:\t"<<myRT.voltage << endl
					 << "frontSonar:\t"<<myRT.frontSonar << endl
					 << "backSonar:\t"<<myRT.backSonar << endl;
			}

			//Sleep(sleepTime);			
		}
		//cout << "waiting...\n";
		gotInput = false;
		base = "";
		input = '\0';
		if(constantDrive){
			base = "FFF";
			if(_kbhit()){
				gotInput = true;
				input = char(_getch());
				if(input == '5')
					base = base + "s50";
				else if(input == '0')
					base = base + "s100";
				else if(input == '1')
					base = base + "s10";
				else
					base = base + input;
			}
		}
		else{
			cin >> base;
			gotInput = true;
		}
		Sleep(sleepTime);

		
		
		
		//cin >> input;
		//input = char(_getch());
		//cout << input << endl;
		//printf("%c\n",input);
		//echoMessage = &input;
	}
	cout << "closing socket\n";
	closesocket(hSocket);
	
	//clean up and end
	iret = WSACleanup();
	if(iret){
		cout << "cleanup failed\n";
		return iret;
	}
	delete[] inputBuffer;
	cout << "done\n";
	system("PAUSE");
	return 0;
}