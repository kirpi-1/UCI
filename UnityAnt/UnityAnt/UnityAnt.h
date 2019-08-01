#pragma once
#define BUFFSIZE 512
#define KEYBOARD_CHECK_INTERVAL 100
enum MessageType{NONE, UA_TEXT, UA_INT, UA_DOUBLE, UA_FLOAT, UA_CHAR, UA_ESCAPE};

HANDLE quitEvent;
HANDLE stopSavingEvent;
HANDLE tcpThread, udpThread, hmdThread;
HANDLE clientConnected;
HANDLE keyboardThread;
HANDLE gotNewFrameHeaderData, gotNewHMDData;
HANDLE HMDDataMutex;
HANDLE wroteFrameHeaderData;
HANDLE frameBufferMutex;
void *frameBuffer;
unsigned int frameBufferSize;
const int REQ_WINSOCK_VER = 2;
char *DEFAULT_PORT = "27015";
char *UDP_PORT = "27016";
float HMDDataBuffer[16] = { 0,0,0,0,
							0,0,0,0,
							0,0,0,0,
							0,0,0,0};
struct ExperimentArguments{
	char basefilename[256];
	char trialsFilename[256];
	UINT subjectNumber;
	UINT startTrial;
};

struct ClientMessage{
	char buffer[BUFFSIZE];
	unsigned int size;
	MessageType type;
};


SOCKET setupServerSocket(char *port);
SOCKET setupUDPServer(char *port);
int connectToClient(SOCKET s);
UINT loadTrialList(char *filename, int trialList[]);
unsigned int __stdcall handleTCPClient(void *params);
unsigned int __stdcall handleUDPClient(void *params);
unsigned int __stdcall HMDThread(void *params);

unsigned int __stdcall socketThread(void *params);
ExperimentArguments parseArguments(int argc, char *argv[]);
unsigned int getMessageSize(char *buffer);
std::string  getMessageString(char *buffer,unsigned int size);
unsigned int getMessageIntegers(int *out, char *buffer, unsigned int size);

unsigned int parseInput(char clientBuffer[], unsigned int numbr, BYTE buffer[], MessageType &type, bool &messageReady, int &messageSize, unsigned int &writeIdx);