// marvinDriver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Aria.h"
#include "marvinDriver.h"


#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int REQ_WINSOCK_VER	= 2;
char *BCI_PORT				= "3817";
char *TABLET_PORT			= "27015";
char *SAVE_PORT				= "1211";
const int BUFFSIZE	= 128;

#define BCI_INDEX (0)
#define TABLET_INDEX (1)
#define SAVE_INDEX (2)
#define SOCKET_LIST_SIZE (5)
SocketStatus socketList[SOCKET_LIST_SIZE] = {FREE,FREE,FREE,FREE,FREE};
string socketListNames[] = {"EEG","Tablet","Save","AUX1","AUX2"};

bool lastCommandRead = true;
bool endProgram = false;
bool closeConnection = false;
bool robotReady = false;
bool frontBackIsActive = false;
bool leftRightIsActive = false;
bool debugOutput = false;


MyBuffer<SharedData<Command>> *frontBuffer = new MyBuffer<SharedData<Command>>;
MyBuffer<SharedData<Command>> *backBuffer = new MyBuffer<SharedData<Command>>;

DWORD sleepTime=1;  //in milliseconds
CRITICAL_SECTION cs;

void robotThread(void *params);
void handleTCPClient(void* params);
//void tabletThread(void* params);
SOCKET setupServerSocket(char *port);
int connectToClient(SOCKET s);
int parseCommands(char *buffer, int numbr, int clientSocket);
int swapBuffers(int commandCount);

int main(int argc, char *argv[])
{
	//initializations
	InitializeCriticalSection(&cs);
	char *bciPort = BCI_PORT;
	char *tabletPort = TABLET_PORT;
	char *savePort = SAVE_PORT;
	bool useBCI = true;
	bool useTablet = true;
	bool useSave = false;

	frontBuffer->read = true;
	backBuffer->read = true;

	ArgStruct *args=new ArgStruct;
	args->argc=&argc;
	args->argv=argv;
	cout << "starting robot thread\n";
	_beginthread(robotThread,0,(void *)(args));	


	while(!robotReady){Sleep(sleepTime);}
	

	//argument parser
	if(argc>1){
		for(int i=1;i<argc;i++){
			string str = argv[i];
			if(str.find("-nt")!=string::npos || str.find("-notablet")!=string::npos
				|| str.find("-NT")!=string::npos)
				useTablet = false;
			if(!strcmp(argv[i],"-to")||!strcmp(argv[i],"-teleop")||!strcmp(argv[i],"-TO"))
				useBCI = false;
			if(str=="-s" || str=="-save")
				useSave = true;
			if(str=="-d" || str=="-debug")
				debugOutput = true;
		}
	}

	if(debugOutput)
		printf("debug output on\n");

	int iResult = 1;
	WSADATA wsaData;

	//initialize winsock
	iResult = WSAStartup(MAKEWORD(2,0), &wsaData);
	if(iResult){
		cout << "WSAStartup failed with error " << iResult << endl;
		return iResult;
	}
	cout << "initialized\n";

	SOCKET servSock;
	if(useBCI){
		servSock = setupServerSocket(bciPort);
		if(servSock<0)
			return servSock;
	}
	SOCKET tabletServSock;
	if(useTablet){
		tabletServSock = setupServerSocket(tabletPort);
		if(tabletServSock<0)
			return tabletServSock;
	}
	SOCKET saveServSock;
	if(useSave){
		saveServSock = setupServerSocket(savePort);
		if(saveServSock<0)
			return saveServSock;
	}
	int BCISocket = SOCKET_ERROR;
	int tabletSocket = SOCKET_ERROR;
	EnterCriticalSection(&cs);
	bool myEndProgram = endProgram;
	bool myConnectionClosed = closeConnection;
	LeaveCriticalSection(&cs);
	do{
		myConnectionClosed = false;
		if(useBCI && socketList[BCI_INDEX]==FREE){
			if(debugOutput)
				cout << "opening EEG connection\n";
			MySockType *bci = new MySockType;
			bci->clientSock = BCISocket;
			bci->servSock = servSock;
			bci->index = BCI_INDEX;
			if(!useTablet){
				EnterCriticalSection(&cs);
				socketList[bci->index] = ACTIVE;
				LeaveCriticalSection(&cs);
			}
			_beginthread(handleTCPClient,0,(void *)(bci));		
		}

		if(useTablet && socketList[TABLET_INDEX]==FREE){
			if(debugOutput)
				cout << "opening tablet connection\n";			
			MySockType *tablet = new MySockType;
			tablet->clientSock = tabletSocket;
			tablet->servSock = tabletServSock;
			tablet->index = TABLET_INDEX;
			_beginthread(handleTCPClient,0,(void *)(tablet));
		}

		while(!myEndProgram && !myConnectionClosed){
			//cout << "checking for closed connections or end program\n";
			EnterCriticalSection(&cs);
			myEndProgram = endProgram;
			myConnectionClosed = closeConnection;
			LeaveCriticalSection(&cs);
			
			Sleep(sleepTime);
		}
		if(myConnectionClosed)
			cout << "connection closed\n";
		if(myEndProgram)
			cout << "ending program\n";
		
	}while(!myEndProgram);	
	//wait until all the connections have been closed
	bool quit = false;
	while(!quit){
		quit = true;
		cout << "waiting for all connections to be free\n";
		EnterCriticalSection(&cs);		
		for(int i=0;i<SOCKET_LIST_SIZE;i++){
			cout << SocketStatusNames[socketList[i]] << ", ";		
			if(socketList[i]!=FREE)
				quit = false;
			}
		cout << endl;
		LeaveCriticalSection(&cs);
		Sleep(sleepTime);
	}
	closesocket(servSock);
	closesocket(tabletServSock);
	delete frontBuffer;
	delete backBuffer;
	WSACleanup();
	cout << "program done.\n";
	Aria::exit(0);
}


//put thread function here

void robotThread(void *params){
	ArgStruct *args = reinterpret_cast<ArgStruct *>(params);
	Aria::init();  //initialize
	ArRobot robot; //declare robot object
	ArArgumentParser parser(args->argc, args->argv); //declare parser
	parser.loadDefaultArguments();
	
	//robotSetup returns 0 if it succeeded
	ArRobotConnector robotConnector(&parser, &robot);
	if(!robotConnector.connectRobot()){
        ArLog::log(ArLog::Terse, "marvinDriver: could not connect to robot.");
        if(parser.checkHelpAndWarnUnparsed()){
            //-help not given
            Aria::logOptions();
            Aria::exit(1);
        }
    }
    if(!Aria::parseArgs()){
        Aria::logOptions();
        Aria::shutdown();
        //return -1;
    }
	ArLog::log(ArLog::Normal, "connected!!");
	robot.runAsync(true);
	ArLog::log(ArLog::Normal, "locking robot");
	robot.lock();
	robot.comInt(ArCommands::ENABLE,1);
	robot.unlock();
	ArLog::log(ArLog::Normal, "Marvin Ready");
	/*
	if(robotSetup(&parser, &robot)!=0){		
		Aria::exit(1);
	}
	cout << "starting handler\n";
	/*
	
	*/
	MyTeleop teleop(&robot);
	teleop.activate();
	cout << "teleop activated\n";
	EnterCriticalSection(&cs);
		robotReady = true;
		int count = 0;
		bool myEndProgram = endProgram; //local version of this global resource
	LeaveCriticalSection(&cs);
	MyRobotTelemetry lastRT, curRT;
	lastRT.compass = robot.getCompass();
	lastRT.latVel = robot.getLatVel();
	lastRT.odometerDistance = robot.getOdometerDistance();
	lastRT.odometerTime = robot.getOdometerTime();
	lastRT.vel = robot.getVel();
	lastRT.voltage = robot.getBatteryVoltage();
	curRT = lastRT;
	while(!myEndProgram){
		count++;
		int frontDistance = robot.getClosestSonarRange(-70,70);
		int backDistance = robot.getClosestSonarRange(110,250);
		//unsigned int maxRatio = teleop.myKD->getMaxRatio();
		
		if(frontDistance<300 && robot.getVel()>0 || backDistance<300&&robot.getVel()<0){
			teleop.space();
			teleop.setThrottle(0);
			if(debugOutput)
				printf("something is too close to sonar, stopping\n");
			//robot.setTransVelMax(0);
			/*
			if(frontDistance<300)
				cout << "front distance is " << frontDistance << endl;
			if(backDistance<300)
				cout << "back distance is " << backDistance << endl;
				*/
			
		}		
		else if(frontDistance<500 && robot.getVel()>0 || backDistance<500 && robot.getVel()<0){
			if(debugOutput)
				printf("something is too close to sonar, setting throttle to 1/4th speed of %u\n",teleop.getMaxThrottle()/4);		
			teleop.setThrottle(teleop.getMaxThrottle()/4);
		}
			//robot.setTransVelMax(300);
		else if(frontDistance<700 && robot.getVel()>0 || backDistance<700 && robot.getVel()<0){
			if(debugOutput)
				printf("something is too close to sonar, setting throttle to 1/2 speed of %u\n", teleop.getMaxThrottle()/2);		
			teleop.setThrottle(teleop.getMaxThrottle()/2);
			//robot.setTransVelMax(600);
		}
		else
			teleop.setThrottle(teleop.getMaxThrottle());
			//robot.setTransVelMax(900);
		

		EnterCriticalSection(&cs);	
		//read command, mark it as read
		if(frontBuffer->read==false && frontBuffer->size>0){
			if(debugOutput)
				cout << "selecting command\n";
			for(int i=0;i<frontBuffer->size;i++){
				if(debugOutput)
					cout << CommandNames[frontBuffer->data[i].data] << endl;
				if(frontBuffer->data[i].data == FORWARDS && frontBackIsActive){
					teleop.up();				
				}
				else if(frontBuffer->data[i].data == BACKWARDS && frontBackIsActive){
					teleop.down();
				}
				else if(frontBuffer->data[i].data == LEFT && leftRightIsActive){
				
					teleop.left();
				}
				else if(frontBuffer->data[i].data == RIGHT && leftRightIsActive){
					teleop.right();
				}			
				else if(frontBuffer->data[i].data == STOP){
					teleop.space();				
				}
				else if(frontBuffer->data[i].data == CHANGE_SPEED){
					teleop.setMaxThrottle(frontBuffer->data[i].myInt);
					if(debugOutput)
						cout << "Max throttle set to: " << frontBuffer->data[i].myInt << endl;						
				}
				else if(frontBuffer->data[i].data == POLL_SONAR){
					cout << "number of sonar: " << robot.getNumSonar() << endl;
					for(int i=0;i<robot.getNumSonar();i++){
						cout << "sonar " << i << ": " << robot.getSonarReading(i)->getRange() << endl;
					}
				}
				else if(frontBuffer->data[i].data == POLL_TELEMETRY){
					cout << "sending telemetry data...";
					MyRobotTelemetry lastRT, curRT;
					curRT.compass = robot.getCompass();
					curRT.latVel = robot.getLatVel();
					curRT.odometerDistance = robot.getOdometerDistance();
					curRT.odometerTime = robot.getOdometerTime();
					curRT.vel = robot.getVel();
					curRT.voltage = robot.getBatteryVoltage();
					if(curRT==lastRT)
						send(frontBuffer->data[i].myInt,reinterpret_cast<char*>(&curRT),sizeof(MyRobotTelemetry),0);
					cout << "done.\n";
				}
				frontBuffer->data[i].data=NONE;
				//curCommand.ready=false;
			}//for the size of the front buffer
			frontBuffer->read = true;
			frontBuffer->size = 0;
			if(debugOutput)
				cout << "front buffer read\n";
		}//end if frontBuffer hasn't been read and its size is greater than 0
		/*
		if(debugOutput)
			cout << "checking for end of program in robotThread\n"; 
		*/
		myEndProgram = endProgram;
		LeaveCriticalSection(&cs);
		Sleep(sleepTime);	
	}//end main loop
	robot.waitForRunExit();
	cout << "leaving robot!\n";
	EnterCriticalSection(&cs);
	if(debugOutput)
			cout << "Leaving robotThread()\n"; 
	
	LeaveCriticalSection(&cs);
	//_endthread();
}
void handleTCPClient(void* params){
	MySockType *clientSocket = reinterpret_cast<MySockType *> (params);
	EnterCriticalSection(&cs);
	if(debugOutput){
		cout << "creating socket for " << socketListNames[clientSocket->index] 
			 << "...";
	}
	LeaveCriticalSection(&cs);
	clientSocket->clientSock = connectToClient(clientSocket->servSock);
	EnterCriticalSection(&cs);
	cout << socketListNames[clientSocket->index] << " socket is " 
		 << clientSocket->clientSock << endl;
	LeaveCriticalSection(&cs);
	if(clientSocket->clientSock<0 || clientSocket->clientSock == SOCKET_ERROR){
		cout << "could not create socket\n";
	}
	else{
		EnterCriticalSection(&cs);
		if(debugOutput)
			cout << "done creating socket\n";
		cout << socketListNames[clientSocket->index] << " is now connected\n";
		socketList[clientSocket->index]=INACTIVE;
		LeaveCriticalSection(&cs);
		
		char buffer[BUFFSIZE];

		bool disconnect=false;
		unsigned int numbr;
		numbr = recv(clientSocket->clientSock, buffer, BUFFSIZE, 0);
		while(numbr>0 && !disconnect){
			Sleep(sleepTime);
			if(_kbhit()){
				int ch = _getch();
				if(ch == 'Q' || ch == 'q')
					endProgram=true;
			}

			int nError = WSAGetLastError();
			//if there is an error and it is not a "would block" error
			if(nError!=WSAEWOULDBLOCK&&nError!=0){
				cout << "Winsock error code: "<<nError<<", disconnecting...\n";
				//endProgram = true;
				disconnect = true;
			}
			else if(nError!=WSAEWOULDBLOCK)
			{
				//if receive a command from the tablet,
				//deactivate EEG input
				if(clientSocket->index==TABLET_INDEX){
					EnterCriticalSection(&cs);
					if(socketList[TABLET_INDEX] != ACTIVE){
						cout << "received command from tablet, disabling EEG connection\n";
						if(socketList[BCI_INDEX] == ACTIVE)
							socketList[BCI_INDEX] = INACTIVE;
						socketList[TABLET_INDEX] = ACTIVE;
					}
					LeaveCriticalSection(&cs);
				}
				EnterCriticalSection(&cs);
				if(debugOutput)
					cout << "received " << numbr << " bytes\n";			
				for(unsigned int i=0;i<numbr;i++){
					if(buffer[i] == 32)
						cout << "\"space\"";
					cout << buffer[i];	
				}
				cout << endl;
				bool isActive = socketList[clientSocket->index]==ACTIVE;
				LeaveCriticalSection(&cs);
				
				if(isActive){
					bool myExit = false;
					int commandCount;
					
					//wait until the backBuffer has been read
					//then fill it up with new commands
					while(!myExit){
						EnterCriticalSection(&cs);
						if(debugOutput)
							cout << "checking if backBuffer has been read\n";
						if(backBuffer->read=true){
							commandCount = parseCommands(buffer,numbr,clientSocket->clientSock);					
							if(debugOutput)
								cout << "command count: " << commandCount << endl;
							if(commandCount<0){
								disconnect = true;					
								if(commandCount==-2){
									if(debugOutput)
										cout << "received a quit command, exiting...\n";
									endProgram = true;
								}
								//need to stop robot, then disconnect
								commandCount = 1;
								backBuffer->data[0].data = STOP;
							}
							swapBuffers(commandCount);
							myExit=true;
						}
						LeaveCriticalSection(&cs);
						Sleep(sleepTime);
					}
	
					
				}//end if this connection is active
			}//end if recv() would block
			else if(nError==WSAEWOULDBLOCK){
				/*
				EnterCriticalSection(&cs);
				if(debugOutput)
					cout << "read() would block\n";
				LeaveCriticalSection(&cs);
				*/
			}
			numbr = recv(clientSocket->clientSock, buffer, BUFFSIZE, 0);
			EnterCriticalSection(&cs);
				if(endProgram)	
					disconnect = true;
			LeaveCriticalSection(&cs);
		}//while receiving data and don't have to quit
		EnterCriticalSection(&cs);
		//cout << "exiting handleTCPClient()\n";	
		closeConnection = true;	
		closesocket(clientSocket->clientSock);
		socketList[clientSocket->index] = FREE;
		if(debugOutput)
			cout << socketListNames[clientSocket->index] << " socket closed\n";
		LeaveCriticalSection(&cs);
	}//end else valid socket was made
	EnterCriticalSection(&cs);
	if(debugOutput){
		cout << "reached end of handle client for " 
		     << socketListNames[clientSocket->index] << endl;
	}
	LeaveCriticalSection(&cs);
	delete clientSocket;
	//_endthread();
}

SOCKET setupServerSocket(char *port){
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s<0){
		cout << "could not create socket\n";
		WSACleanup();
		return s;
	}
	//if iMode!=0, socket is made non-blocking
	u_long iMode=1;
	ioctlsocket(s,FIONBIO,&iMode);
	struct sockaddr_in servAddr;
	ZeroMemory(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(atoi(port));
	
	//bind to local address
	int iResult = bind(s, reinterpret_cast<const sockaddr*> (&servAddr), sizeof(servAddr));
	if(iResult<0){
		cout << "bind failed\n";
		WSACleanup();
		return iResult;
	}
	
	//mark socket so it will listen!
	iResult = listen(s, SOMAXCONN);
	if(iResult < 0){
		cout << "listen failed with error " << WSAGetLastError() << endl;
		closesocket(s);
		WSACleanup();
		Aria::exit(iResult);
	}
	return s;
}
int connectToClient(SOCKET s){
	struct sockaddr_in clientAddr;
	ZeroMemory(&clientAddr, sizeof(clientAddr));
	socklen_t clientAddrLen = sizeof(clientAddr);
	

	//accept a client socket	
	EnterCriticalSection(&cs);
	if(debugOutput)
		cout << "using socket " << s << "...";
	cout << "accept()ing..." << endl;
	bool myEndProgram = endProgram; //local copy of shared resource
	LeaveCriticalSection(&cs);
	int clientSocket = SOCKET_ERROR;
	
	while(clientSocket == SOCKET_ERROR && !myEndProgram){
		clientSocket = accept(s, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
		if(_kbhit()){
			int ch = _getch();
			if(ch == 'Q' || ch == 'q')
				endProgram=true;
		}
		EnterCriticalSection(&cs);
		myEndProgram = endProgram;
		LeaveCriticalSection(&cs);
		Sleep(sleepTime);
	}
	if(!myEndProgram){
		if(clientSocket<0){
			cout << "accept() failed\n";
			closesocket(s);
			WSACleanup();
			return -1;
		}
		else{ 
			cout << "success! ";
			//if iMode!=0, socket is made non-blocking
			u_long iMode = 1;
			ioctlsocket(clientSocket,FIONBIO,&iMode);
		}
	}
	else{
		EnterCriticalSection(&cs);
		if(debugOutput)
			cout << "aborting from connectToClient()\n";
		LeaveCriticalSection(&cs);
		clientSocket = SOCKET_ERROR;
		return -1;
	}
	return clientSocket;
}

int parseCommands(char *buffer, int numbr, int clientSocket){
	int commandCount = 0;
	for(int i=0;i<numbr;i++){						
		if(buffer[i]=='f' || buffer[i] =='F')
			backBuffer->data[commandCount].data=FORWARDS;
		else if(buffer[i]=='b' || buffer[i]=='B')
			backBuffer->data[commandCount].data=BACKWARDS;
		else if(buffer[i]=='l' || buffer[i]=='L')
			backBuffer->data[commandCount].data=LEFT;
		else if(buffer[i]=='r' || buffer[i]=='R')
			backBuffer->data[commandCount].data=RIGHT;
		else if(buffer[i]==32) 
			backBuffer->data[commandCount].data=STOP;
		else if(buffer[i]=='s' || buffer[i]=='S'){
			backBuffer->data[commandCount].data=CHANGE_SPEED;
			i++;
			int numLength = 0;
			while(i<numbr && buffer[i]>='0' && buffer[i]<='9'){
				numLength++;
				i++;
			}
			if(numLength>0){
				char *memblock = new char[numLength];
				memcpy(memblock,buffer+i-numLength,numLength);
				backBuffer->data[commandCount].myInt = atoi(memblock);
				delete memblock;
			}
			else{
				backBuffer->data[commandCount].data = NONE;
				i--;
			}
		}
		else if(buffer[i]=='p' || buffer[i]=='P'){
			backBuffer->data[commandCount].data=POLL_SONAR;
		}
		else if(buffer[i]=='t' || buffer[i]=='T'){
			backBuffer->data[commandCount].data = POLL_TELEMETRY;
			backBuffer->data[commandCount].myInt = clientSocket;
		}
		else if(buffer[i]=='e' || buffer[i]=='E'){
			if(socketList[BCI_INDEX]==INACTIVE){
				socketList[BCI_INDEX] = ACTIVE;
				socketList[TABLET_INDEX] = INACTIVE;
				cout << "EEG now active\n";				
			}
			backBuffer->data[commandCount].data=NONE;
		}
		else if(buffer[i]=='x' || buffer[i]=='X'){
			backBuffer->data[commandCount].data=STOP;
			cout << "front/back deactivated\n";
			frontBackIsActive=false;
		}
		else if(buffer[i]=='w' || buffer[i]=='W'){
			backBuffer->data[commandCount].data=NONE;
			cout << "front/back activated\n";
			frontBackIsActive=true;
		}
		else if(buffer[i]=='z' || buffer[i]=='Z'){
			backBuffer->data[commandCount].data=STOP;
			cout << "left/right deactivated\n";
			leftRightIsActive=false;
		}
		else if(buffer[i]=='y' || buffer[i]=='Y'){
			backBuffer->data[commandCount].data=NONE;
			cout << "left/right activated\n";
			leftRightIsActive=true;
		}
		else if(buffer[i]=='d' || buffer[i]=='D'){
			backBuffer->data[commandCount].data=STOP;
			frontBackIsActive = false;
			leftRightIsActive = false;
			commandCount = -1;
			break;
		}
		else if(buffer[i]=='q' || buffer[i]=='Q'){
			cout << "endProgram set to true\n";
			backBuffer->data[commandCount].data=STOP;
			frontBackIsActive = false;
			leftRightIsActive = false;
			commandCount = -2;
			break;
		}
		else
			backBuffer->data[commandCount].data=NONE;
		commandCount++;
	}//end for numbr fill backBuffer
	return commandCount;
}

int swapBuffers(int commandCount){
	//this function is to be used inside of a critical section
	backBuffer->size = commandCount;
	backBuffer->read = false;
	//EnterCriticalSection(&cs);
	bool myEndProgram = endProgram;
	//LeaveCriticalSection(&cs);
	//Swap buffer section
	if(backBuffer->size>0){
		//wait until the front buffer has been read
		bool ready=false;
		while(!ready && !myEndProgram){
			//EnterCriticalSection(&cs);
				if(frontBuffer->read==true){
					ready=true;
					if(debugOutput)
						cout << "front buffer ready\n";
				}
				myEndProgram = endProgram;
			//LeaveCriticalSection(&cs);
			Sleep(sleepTime);
		}
		//then switch buffers after it has been read
		if(!myEndProgram){	
			//EnterCriticalSection(&cs);

			if(debugOutput)
				cout << "swapping buffers\n";
			MyBuffer<SharedData<Command>> *tempBuffer = frontBuffer;
			frontBuffer = backBuffer;
			backBuffer = frontBuffer;
			//LeaveCriticalSection(&cs);
		}
	}//end if backBuffer size is greater than 0
	return 0;
}
