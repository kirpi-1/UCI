#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string>

#define TCP
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define DEFAULT_SERV_NAME "SAGER02"
#define DEFAULT_PORT "27016"
#define sleepTime 1

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
	//create server address	
	struct addrinfo *result,*p;
	struct addrinfo hints;

	//UDP
	//ZeroMemory(&hints, sizeof(hints)); //VERY IMPORTANT


	//hints.ai_family = AF_INET;	
	//hints.ai_socktype = SOCK_DGRAM;

	//if(getaddrinfo(servName, port,&hints, &result)!=0){		
	//	cout << "getaddrinfo() failed\n";
	//	cout << "error " << iret << endl;
	//	WSACleanup();
	//	return iret;
	//}
	//SOCKET udpSocket;
	//for(p = result;p!=NULL;p=p->ai_next){
	//	udpSocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
	//	if(udpSocket == -1)
	//		continue;
	//	break;
	//}
	//struct addrinfo server;
	//ZeroMemory(&server, sizeof(server));
	//sockaddr sa;		
	//memcpy(sa.sa_data,p->ai_addr->sa_data,sizeof(char)*14);
	//sa.sa_family = p->ai_addr->sa_family;	
	//
	//server.ai_addr = &sa;	
	//size_t addrlen = p->ai_addrlen;
	
	
	
	// TCP
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; //force IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
		
	iret = getaddrinfo(servName, port, &hints, &result);
	if(iret!=0){
		cout << "getaddrinfo() failed\n";
		cout << "error " << iret << endl;
		WSACleanup();
		return iret;
	}
	
	//create socket
	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(hSocket < 0){
		cout << "could not create socket\n";
		WSACleanup();
		return 1;
	}

	cout << "connecting...\n";
	//connect to socket
	if(connect(hSocket, result->ai_addr, result->ai_addrlen) < 0){
		cout << "connect() failed\n";
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);
	
	
	cout << "connected\n";
	//char *echoMessage;
	string base = "";	
	//string input;
	//cin >> input;
	char input;
	bool gotInput = false;

	//echoMessage = &base;
	//int i=0;
	cout << base << endl;
	bool receiving = false;
	char *inputBuffer=new char[128];
	char buffer[512];
	unsigned int bufferSize = 0;
	int numBytes = 0;
	int numbs;
	int t;
	int ta[] = {1,2,3,4,5};
	float f;
	float fa[] ={1.111,2.222,3.333,4.444,5.555};
	double d;
	double da[] = {1.123456789,2.123456789,3.123456789,4.123456789,5.123456789};
	bool quit = false;
	while(!quit){
		int selection;
		cout << "1. \"Hello World\" together"<< endl
			 << "2. \"Hello World\" separate"<< endl
			 << "3. 32" << endl
			 << "4. 3.141" << endl
			 << "5. 2.718281828" << endl
			 << "6. [1 2 3 4 5]" << endl
			 << "7. quit" << endl;
		cin >> selection;
		switch(selection){
			case 1:
				memcpy(buffer,"-t",sizeof(char)*2);
				strcpy(buffer+2,"Hello World");				
				bufferSize = 2+12;
				break;
			case 2:
				memcpy(buffer,"-t",sizeof(char)*2);
				numbs = send(hSocket, buffer, 2, 0);
				strcpy(buffer,"Hello World");
				bufferSize = 12;
				break;
			case 3:
				memcpy(buffer,"-i",sizeof(char)*2);
				t = 1;
				memcpy(buffer+2,reinterpret_cast<char *>(&t),sizeof(int));
				t = 32;
				memcpy(buffer+2+sizeof(int),reinterpret_cast<char *>(&t),sizeof(int));
				bufferSize = 2+2*sizeof(int);
				break;
			case 4:
				memcpy(buffer,"-f", sizeof(char)*2);
				t = 1;
				memcpy(buffer+2,reinterpret_cast<char *>(&t),sizeof(int));
				f = 3.141;
				memcpy(buffer+2+sizeof(int),reinterpret_cast<char *>(&f),sizeof(float));
				bufferSize = 2+sizeof(int)+sizeof(float);
				break;
			case 5:
				memcpy(buffer,"-d", sizeof(char)*2);
				t = 1;
				d = 2.718281828;
				memcpy(buffer+2,reinterpret_cast<char *>(&t),sizeof(int));				
				memcpy(buffer+2+sizeof(int),reinterpret_cast<char *>(&d),sizeof(double));
				bufferSize = 2+sizeof(int)+sizeof(double);
				break;
			case 6:
				//memcpy(buffer,"-i",sizeof(char)*2);				
				//t = 5;
				//memcpy(buffer+2,reinterpret_cast<char *>(&t),sizeof(int));				
				//memcpy(buffer+2+sizeof(int),reinterpret_cast<char *>(ta),sizeof(int)*5);
				//bufferSize = 2+sizeof(int)+sizeof(int)*5;
				memcpy(buffer,reinterpret_cast<char *>(ta),sizeof(int)*5);
				bufferSize = sizeof(int)*5;
				break;
			case 7:
				quit = true;
				bufferSize = 0;
				break;
		}
		if(bufferSize > 0){
			numbs = send(hSocket, buffer, bufferSize, 0);
			//numbs = sendto(udpSocket,buffer,bufferSize,0,server.ai_addr,server.ai_addrlen);
			//numbs = sendto(udpSocket,buffer,bufferSize,0,p->ai_addr,p->ai_addrlen);
			bufferSize = 0;
		}

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
	//system("PAUSE");
	return 0;
}