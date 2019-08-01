#include "stdafx.h"
#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
namespace cdf{
#define MS_UNINITIALIZED_PTR 0xcccccccc
	using namespace std;
	class OutOfBoundsError{};
	static OutOfBoundsError outOfBoundsError;
	class FileReadError{};
	static FileReadError fileReadError;

	struct Date{
		unsigned char minute;	//1 byte
		unsigned char hour;		//1 byte
		unsigned char day;		//1 byte
		unsigned char month;	//1 byte
		unsigned short year;	//2 bytes
	};
	
	const unsigned int NAME_SIZE = 16;
	const unsigned int FILENAME_SIZE = 64;
	const unsigned int ASCII_HEADER_SIZE = 512;

	class Coi{
	public:
		Coi(){chanMat=NULL;numElec=-1;};
		Coi(int e){initialize(e);};
		~Coi();
		void initialize(int e);
		void cleanup();
		Coi &operator=(const Coi &c);
		float *chanMat;
		int numElec;
	};

	class Action{
	public:
		Action(){weights=NULL;numCOI=numFreq=-1;};
		Action(int numCOI, int numFreq){initialize(numCOI, numFreq);};
		~Action();
		void initialize(int numCOI, int numFreq);
		void cleanup();
		Action &operator=(const Action &a);
		float *weights;
		int numCOI;
		int numFreq;
	};

	struct SessionInfo{		
		unsigned int numFrames;				//4 bytes
		unsigned int variableHeaderSize;	//4 bytes
		unsigned short  subjectNumber;		//2 bytes
		Date date;							//6 bytes
		unsigned short dataType;			//2 bytes
		unsigned short numElec;				//2 bytes
		unsigned short numFreq;				//2 bytes
		unsigned short numCOI;				//2 bytes
		unsigned short numActions;			//2 bytes
		char padding[2];					//2 bytes
	};										//28 total

	class VariableHeader{
	public:
		VariableHeader(){pointerSetup();};
		VariableHeader(SessionInfo &si);
		//VariableHeader(const VariableHeader &vh);
		~VariableHeader();
		void initialize(SessionInfo &si);
		void initializeCOIs(SessionInfo &si);
		void initializeActions(SessionInfo &si);
		void cleanup();
		void pointerSetup();
		VariableHeader &operator=(const VariableHeader &vh);
		float *freqs;
		std::string coiFilename;
		std::string actionFilename;
		std::string *coiNames;
		std::string *actionNames;
		Coi *cois;
		Action *actions;		
		SessionInfo *session;
	};

	const unsigned int sizeofFixedHeader = sizeof(SessionInfo);
	
	class FrameInfo{
	public:
		FrameInfo(){index=0;headerSize=0;xlen=0;ylen=0;pointerSetup();};
		FrameInfo(int x, int y){initialize(x,y);};
		FrameInfo(int idx, int x, int y){initialize(idx,x,y);};
		FrameInfo(int idx, int hs, int x, int y){initialize(idx,hs,x,y);};
		~FrameInfo();
		void initialize(int idx, int hs, int x, int y);
		void initialize(int idx,int x, int y){initialize(idx,0,x,y);};
		void initialize(int x, int y){initialize(-1,x,y);};
		void pointerSetup();
		void cleanup();
		bool setHeader(char *in, unsigned int size);
		bool setData(float *in, unsigned int xlen, unsigned int ylen=1);
		unsigned int size(){return headerSize;};
		unsigned int index;		
		unsigned int xlen;
		unsigned int ylen;
		char *header;
		float *data;
	private:
		unsigned int headerSize;
	};
	static std::string channelNames[] = {//total 128 names
		"FP1", "FPZ", "FP2", "F7", "F3", "FZ", "F4", "F8", "FC5", "FC1",
		"FC2", "FC6",  "M1", "T7", "C3", "CZ", "C4", "T8",  "M2", "CP5",
		"CP1", "CP2", "CP6", "P7", "P3", "PZ", "P4", "P8", "POZ",  "O1",
		 "OZ",  "O2", "AF7","AF3","AF4","AF8", "F5", "F1",	"F2",  "F6",
		"FC3", "FCZ", "FC4", "C5", "C1", "C2", "C6","CP3", "CPZ", "CP4",
		 "P5",  "P1",  "P2", "P6","PO5","PO3","PO4","PO6", "FT7", "FT8",
		"TP7", "TP8", "PO7","PO8","AUX1","AUX2","AUX3","AUX4","AUX5","AUX6",
		 "AUX7", "AUX8", "AUX9","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10",
		"AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10",
		"AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10",
		"AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10",
		"AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10",
		"AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10","AUX10"
	};
	//this class is used to automatically manage memory when reading
	//a CDF file
	class CDFSession{
	public:
		CDFSession(){pointerSetup();};
		CDFSession(SessionInfo& si, VariableHeader& vh);
		~CDFSession();
		void cleanup();
		SessionInfo session;
		VariableHeader varHeader;
		FrameInfo *frames;
		char *asciiHeader;
		void printCOI();
		void printActions();
		void printAsciiHeader();
		void printFrame(int i);
		void printFreqs();
		void setNumFrames(int nf);
		void setSession(SessionInfo &si);
		void setVarHeader(VariableHeader &vh);
		void setFreqs(float *freqs, int numFreqs);
		
		FrameInfo *getFrames();

		int close();

		int open(char *filename, bool overwrite = false);
		int append(char *filename);

		int readCharCOI(char *filename);
		int readCharActions(char *filename);

		int writeFrame(FrameInfo &fi); //appends the frame to the end of the file
		int writeNumFrames(int numf); //writes the number of frames directly to file in the appropriate part of the header
		int writeNumFrames();
		int writeHeader();	//writes header automatically at beginning of file
	private:
		fstream file;
		void pointerSetup();
	};

	int getChannelIndex(const char *chName);
	std::string getChannelName(unsigned int i);
	int writeASCIIHeader(fstream &fout, SessionInfo &si, string *coiNames, string *actionNames);
	int writeASCIIHeader(fstream &fout, SessionInfo &si, VariableHeader &vh);
	int writeFixedHeader(fstream &fout, SessionInfo &si, VariableHeader &vh);
	int writeVariableHeader(fstream &fout, char *coiFile, unsigned int coiFileSize, char *actionsFile, unsigned int actionsFileSize);
	int writeVariableHeader(fstream &fout, SessionInfo &si, string *coiNames, string *actionNames, Coi *cois,  Action *actions, string filename="");
	int writeVariableHeader(fstream &fout, SessionInfo &si, VariableHeader &vh);
	//int writeHeader(fstream &fout, SessionInfo &si, string *coiNames, string *actionNames, Coi *cois, Action *actions);
	int writeHeader(fstream &fout, SessionInfo &si, VariableHeader &vh);
	int writeFrame(fstream &fout, FrameInfo *frame);
	int writeNumFrames(fstream &fout, SessionInfo &si);
	int writeNumFrames(fstream &fout, unsigned int size);

	void getVariableHeaderSize(SessionInfo &si);
	
	
	char *readASCIIHeader(fstream &fin);
	SessionInfo readFixedHeader(fstream &fin);
	VariableHeader readVariableHeader(fstream &fin, SessionInfo &si);
	void readVariableHeader(fstream &fin, SessionInfo &si, VariableHeader &vh);
	
	void readCOI(fstream &fin, SessionInfo &si, VariableHeader &vh);
	Coi *readCOI(fstream &fin, SessionInfo &si);
	Action *readActions(fstream &fin, SessionInfo &si);
	
	FrameInfo *readFrame(fstream &fin);
	void readFrame(fstream &fin, FrameInfo &frame);
	FrameInfo *readFrameIndex(fstream &fin, SessionInfo &si, unsigned int index);
	FrameInfo *readFrameIndex(fstream &fin, unsigned int index);
	FrameInfo *getFrames(fstream &fin, SessionInfo &si);
	void getFrames(fstream &fin, SessionInfo &si, FrameInfo *frames);
	//void readFrame(fstream &fin, FrameInfo &frame);
	//void readFrameIndex(fstream &fin, SessionInfo &si, FrameInfo &frame, unsigned int index);
	
	void readCOIActionNames(fstream &fin, SessionInfo &si, string **coiNames, string **actionNames);
	void readCharCOI(const char *filename, SessionInfo &si, Coi **cois, string **coiNames);
	void readCharCOI(string filename, SessionInfo &si, VariableHeader &vh);
	void readCharActions(const char *filename, SessionInfo &si, string **coiNames, Action **actions, string **actionNames);
	void readCharActions(string filename, SessionInfo &si, VariableHeader &vh);
	void printCOI(SessionInfo &si,Coi *cois, string *coiNames);
	//string sprintCoi(SessionInfo &si, Coi *cois, string *coiNames);
	void printActions(SessionInfo &si, Action *actions, string *actionNames);
	//string sprintCoi(SessionInfo &si, Action *actions, string *actionNames);
	void printFrame(FrameInfo *frame);
	//string sprintFrame(FrameInfo *frame);
}