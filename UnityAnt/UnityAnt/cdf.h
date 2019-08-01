#ifndef __CDF__
#define __CDF__


#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <Windows.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define MS_UNINITIALIZED_PTR 0xcccccccc
#define CDF_FILE_NOT_GOOD -1
#define CDF_FILE_OPEN_ERROR -2
#define CDF_FILE_READ_ERROR -3
#define CDF_RAW (0)
#define CDF_AMP (1)
#define CDF_FREQ (2)
#define CDF_FI_MAX_XLEN 128
#define CDF_FI_MAX_YLEN 512

namespace cdf{
	using namespace std;
	class OutOfBoundsError{};
	static OutOfBoundsError outOfBoundsError;
	class FileReadError{};
	static FileReadError fileReadError;
	class FileOpenError{};
	static FileOpenError fileOpenError;
	class CannotOverwriteHeaderError{};
	static CannotOverwriteHeaderError cannotOverwriteHeaderError;

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
	static char nameSlop[512] = {0};

	struct SessionInfo{		
		unsigned int numFrames;				//4 bytes
		unsigned int variableHeaderSize;	//4 bytes
		unsigned short  subjectNumber;		//2 bytes
		Date date;							//6 bytes
		unsigned short dataType;			//2 bytes
		unsigned short numElec;				//2 bytes
		unsigned short trialType;			//2 bytes
		char buffer[2];						//2 byte buffer to make this 24 bytes
	};

	SessionInfo defaultSession();
	
	const unsigned int sizeofFixedHeader = sizeof(SessionInfo);
	

	/* Frame data structured as follows:
		index number			(unsigned int, 4 bytes)
		variable header size	(unsigned int, 4 bytes)
		size in x dimension		(unsigned int, 4 bytes)
		size in y dimension		(unsigned int, 4 bytes)
		timestamp				(unsigned int, 4 bytes)
		data					(xdim * ydim floats, 4 bytes each)
		variable header			(header size number of bytes)


	*/
	class Frame{
	friend class CDFSession;
	public:
		Frame(){index=0;m_headerSize=0;m_xlen=0;m_ylen=0;m_allocatedSize=0;pointerSetup();};
		Frame(int x, int y){initialize(x,y);};
		Frame(int idx, int x, int y){initialize(idx,x,y);};
		Frame(int idx, int hs, int x, int y){initialize(idx,hs,x,y);};
		~Frame();
		Frame &operator=(Frame &f);

		void initialize(int idx, int hs, int x, int y);
		void initialize(int idx,int x, int y){initialize(idx,0,x,y);};
		void initialize(int x, int y){initialize(0,x,y);};
		void pointerSetup();
		void cleanup();
		bool setHeader(void *in, unsigned int size);
		void clearHeader();
		bool setFrameSize(unsigned int x, unsigned int y);
		bool setData(float *in, unsigned int x, unsigned int y=1);
		bool setTimeStamp(DWORD t);
		void setDefault();

		void print();

		char* header(){return m_header;};
		float*data(){return m_data;};
		unsigned int headerSize(){return m_headerSize;};
		unsigned int dataSize(){return m_xlen*m_ylen*sizeof(float);};
		unsigned int size(){return headerSize()+dataSize();};
		unsigned int xlen(){return m_xlen;};
		unsigned int ylen(){return m_ylen;};	
		unsigned int allocatedSize(){return m_allocatedSize;};
		DWORD timestamp(){return m_timestamp;};
		unsigned int index;
		
	protected:
		DWORD m_timestamp;
		unsigned int m_headerSize;
		unsigned int m_xlen, m_ylen, m_allocatedSize;
		char *m_header;
		float *m_data;

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
		CDFSession();
		CDFSession(unsigned int frameXLen, unsigned int frameYLen);
		~CDFSession();		
		void cleanup();
		void initialize();
		void setDefaults();

		
		void printCOI();
		void printActions();
		void printAsciiHeader();
		void printFrame(unsigned int i);
		void printFrameHeader();
		void printFreqs();
		
		void setSubNum(unsigned int sn);		
		bool setFrameData(float *in, unsigned int x, unsigned int y = 1);
		bool setFrameHeader(void *in, unsigned int size);
		void clearFrameHeader(){curFrame.clearHeader();};
		bool resetFrame();
		unsigned int getFrameSize();
		unsigned int getFrameHeaderSize();
		unsigned int getNumFrames();

		//void setSession(SessionInfo &si);
		void setSessionDate();
		void setSessionDate(struct tm *_tm);
		void setVarHeader(const void *vh, unsigned int size);
		void setVarHeader(const void *vh = NULL);
		void setFreqs(float *freqs, int numFreqs);

		char asciiHeader[ASCII_HEADER_SIZE];
		SessionInfo session;			
		char *varHeader;
		unsigned int varHeaderSize;
		
		int open(char *filename, bool overwrite = true);
		int append(char *filename);
		int close();

		int writeFrame();		
		int writeHeader();
		
		Frame *getFrames();

		bool isOpen();
		bool isGood();
	private:
		fstream file;
		Frame *frames; //for reading
		Frame curFrame; //for writing
		bool haveWrittenHeader;
		bool haveStartedSaving;

		void pointerSetup();

		unsigned int defaultX;
		unsigned int defaultY;

		void clearVarHeader();

		int writeASCIIHeader();
		int writeFixedHeader();
		int writeVariableHeader();
		int writeNumFrames();

		int readHeader();
		int readASCIIHeader();
		int readFixedHeader();
		int readVariableHeader();

		void readNextFrame();

		SYSTEMTIME st;
		DWORD startTime;
		DWORD curTime;
		DWORD offsetTime;
		

	};

	int getChannelIndex(const char *chName);
	std::string getChannelName(unsigned int i);

	//******************************************************************
	//************************ For Reading *****************************
	//******************************************************************
		
	void printFrame(Frame *frame);
	//string sprintFrame(Frame *frame);

	/*
	void setDefaultCOI(SessionInfo &si, VariableHeader &vh);
	void setDefaultAction(SessionInfo &si, VariableHeader &vh);
	*/
}

#endif //__CDF__



//todo
//finish getting rid of variable header from project
//remove actions and COIs from project