#include "stdafx.h"
#include "cdf.h"

using namespace std;

//cdf::OutOfBoundsError outOfBoundsError;

// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// ***************************************************** Frame Definitions *****************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************

void cdf::Frame::initialize(int idx, int hs, int x, int y){
	cleanup();
	pointerSetup();
	index=idx;
	m_headerSize=hs;
	m_header = new char[m_headerSize];		
	setFrameSize(x, y);
	for(unsigned int i=0;i<m_headerSize;i++)
		m_header[i]='\0';	
	for(unsigned int i=0;i<m_xlen*m_ylen;i++)
		m_data[i]=0;
}

void cdf::Frame::pointerSetup(){
	m_header=NULL;
	m_data=NULL;
}

cdf::Frame::~Frame(){
	cleanup();
}

void cdf::Frame::cleanup(){
	clearHeader();
	if(m_data!=NULL)
		delete[] m_data;
	m_data = NULL;
	m_allocatedSize = 0;
}

bool cdf::Frame::setHeader(void *in, unsigned int size){
	clearHeader();
	m_header = new char[size];
	m_headerSize = size;	
	memcpy(m_header,in,size);
	return true;
}

void cdf::Frame::clearHeader(){
	if(m_header!=NULL){
		delete[] m_header;
		m_header = NULL;
	}
	m_headerSize = 0;
}

bool cdf::Frame::setFrameSize(unsigned int x, unsigned int y){
	if(x*y > m_allocatedSize){		
		if(m_data != NULL)
			delete[] m_data;					
		m_data = new float[x*y];
		m_allocatedSize = x*y;
	}	
	m_xlen = x;
	m_ylen = y;
	return true;
}
bool cdf::Frame::setTimeStamp(DWORD t){
	m_timestamp = t;
	return true;
}
bool cdf::Frame::setData(float *in, unsigned int x, unsigned int y){
	setFrameSize(x,y);	
	memcpy(m_data,in,x*y*sizeof(float));
	return true;
}

void cdf::Frame::setDefault(){
	m_xlen = 0;
	m_ylen = 0;
	m_headerSize = 0;
	cleanup();
	pointerSetup();
}

cdf::Frame& cdf::Frame::operator=(Frame &f){
	if(this == &f)
		return *this;		
	setHeader(f.header(),f.headerSize());
	setData(f.data(),f.xlen(),f.ylen());
	return *this;
}

void cdf::Frame::print(){
	printf("\tFrame %d\n",index);
	printf("%d x %d\n",m_xlen,m_ylen);
	if(size()!=0){
		printf("===== Header =====\n%s\n",m_header);
		printf("==================\n");
	}
	else{
		printf("==== No Header ====\n");
	}

	for(unsigned int i=0;i<m_ylen;i++){
		for(unsigned int j=0;j<m_xlen;j++){
			printf("%8.3f ",m_data[i*m_xlen+j]);
		}
		printf("\n");
	}	
}

// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// **************************************************** CDF Session Definitions ****************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************

cdf::CDFSession::CDFSession(){
	initialize();
}

cdf::CDFSession::~CDFSession(){
	cleanup();
}

void cdf::CDFSession::cleanup(){
	if(file.is_open()){
		writeHeader();		
		file.close();
	}
	if(frames!=NULL){
		delete[] frames;
		frames = NULL;
	}
	if(varHeader != NULL){
		delete[] varHeader;
		varHeader = NULL;
	}

}

void cdf::CDFSession::clearVarHeader(){
	if(varHeader != NULL){
		delete[] varHeader;
		varHeader = NULL;		
		session.variableHeaderSize = 0;
	}
}

void cdf::CDFSession::pointerSetup(){
	frames = NULL;
	varHeader = NULL;
}


void cdf::CDFSession::initialize(){
	haveWrittenHeader = false;
	haveStartedSaving = false;

	pointerSetup();
	setDefaults();	
}

void cdf::CDFSession::setDefaults(){
	setSessionDate();
	session.numElec = 64;
	session.numFrames = 0;
	session.subjectNumber = 0;
	session.trialType = 0;
	session.variableHeaderSize = 0;
	session.dataType = CDF_RAW;
	varHeaderSize = 0;
}

void cdf::CDFSession::printAsciiHeader(){
	printf("%s\n",asciiHeader);
}
void cdf::CDFSession::printFrame(unsigned int idx){
	if(idx<session.numFrames && idx>=0){
		printf("=============================\n");
		printf("     Frame %d [index %d]     \n",frames[idx].index,idx);
		printf("=============================\n");
		printf("size: %d x %d\n",frames[idx].xlen(),frames[idx].ylen());
		if(frames[idx].size()!=0){
			printf("===== Header =====\n%s\n",frames[idx].header());
			printf("==================\n");
		}
		else{
			printf("==== No Header ====\n");
		}

		for(unsigned int i=0;i<frames[idx].ylen();i++){
			for(unsigned int j=0;j<frames[idx].xlen();j++){
				printf("%8.3f ",frames[idx].m_data[i*frames[idx].xlen()+j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	else
		printf("Invalid frame: %d\n",idx);
}

void cdf::CDFSession::printFrameHeader(){
	printf("===== Header =====\n");
	for(int i=0;i<curFrame.headerSize();i++)
		printf("%d ",curFrame.header()[i]);
	printf("==================\n");
}

void cdf::CDFSession::setSubNum(unsigned int sn){
	session.subjectNumber = sn;	
}

bool cdf::CDFSession::setFrameData(float *in, unsigned int x, unsigned int y){
	return curFrame.setData(in,x,y);	
}

bool cdf::CDFSession::setFrameHeader(void *in, unsigned int size){
	return curFrame.setHeader(in, size);
}

unsigned int cdf::CDFSession::getFrameSize(){
	return curFrame.dataSize();
}
unsigned int cdf::CDFSession::getFrameHeaderSize(){
	return curFrame.headerSize();
}
unsigned int cdf::CDFSession::getNumFrames(){
	return session.numFrames;
}
bool cdf::CDFSession::resetFrame(){
	float *f;
	clearFrameHeader();
	return setFrameData(f,0,0);
}

void cdf::CDFSession::setSessionDate(){	
	GetLocalTime(&st);
	startTime = st.wMilliseconds+1000*(st.wSecond+60*(st.wMinute+60*st.wHour));
	session.date.minute = st.wMinute;
	session.date.hour = st.wHour;
	session.date.day = st.wDay;
	session.date.month = st.wMonth;
	session.date.year = st.wYear;
}

void cdf::CDFSession::setVarHeader(const void *vh, unsigned int size){	
	if(haveStartedSaving){
		cerr << "Cannot change header once you've started writing frames, it's too dangerous\n";
		throw cannotOverwriteHeaderError;
	}
	clearVarHeader();
	if(vh!=NULL){
		varHeader = new char[size];
	}
	session.variableHeaderSize = size;
	memcpy(varHeader,vh,size);	
	varHeaderSize = size;
	if(haveWrittenHeader)
		writeHeader();
}

void cdf::CDFSession::setVarHeader(const void *vh){
	setVarHeader(NULL,0);
}

int cdf::CDFSession::close(){
	writeNumFrames();
	if(file.is_open())
		file.close();
	return 0;
}

bool cdf::CDFSession::isOpen(){
	return file.is_open();
}

bool cdf::CDFSession::isGood(){
	return file.good();
}

int cdf::CDFSession::append(char *filename){
	if(file.is_open()){
		cerr << "cannot open file again\n";
		return CDF_FILE_OPEN_ERROR;
	}
	file.open(filename,ios::in|ios::out|ios::binary|ios::app);
	if(!file.good())
		return CDF_FILE_NOT_GOOD;
	readASCIIHeader();
	readFixedHeader();
	try
	{			
		readVariableHeader();
	}
	catch(FileReadError f){
		cerr << "file read error of \"" << filename <<"\""<< endl;
		return CDF_FILE_READ_ERROR;
	}
	frames = getFrames();
	return 0;
}

int cdf::CDFSession::open(char *filename,bool overwrite){
	if(file.is_open()){
		cerr << "cannot open a file, already have one open\n";
		return -2;
	}
	//if the overwrite flag is set, just open a new file with truncate flag	
	if(overwrite){
		file.open(filename,ios::in|ios::out|ios::trunc|ios::binary);
		session.numFrames = 0;
		writeHeader();
		//cout << "!good, opening with trunc\n";
	}
	else{
		//otherwise, try opening the file without truncation
		file.open(filename,ios::in|ios::out|ios::binary);
		//if it doesn't exist, it will fail, so try opening again with trunc flag set
		//which will make a new file if it doesn't exist
		if(!file.good()){
			file.clear();
			file.open(filename,ios::in|ios::out|ios::trunc|ios::binary);
			cerr << "File does not exist, opening new file\n";
			session.numFrames = 0;
			writeHeader();
		}
		else{//file already existed, so read its contents
			readHeader();
			getFrames();
		}
	}
	return !file.good();
}

int cdf::CDFSession::writeFrame(){
	haveStartedSaving = true;
	curFrame.index = session.numFrames;
	//cdf::printFrame(&fi);	
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set, did not write frame\n";
		return -1;
	}
	//make sure you are appending the data
	file.seekp(0,ios::end);
	//cout << "frame " << frame->index << endl << frame->xlen << " by " << frame->ylen << endl;
	//byte order is:
	//index number			(unsigned int, 4 bytes)
	//variable header size	(unsigned int, 4 bytes)
	//size in x dimension	(unsigned int, 4 bytes)
	//size in y dimension	(unsigned int, 4 bytes)
	//timestamp				(unsigned int, 4 bytes)
	//data					(xdim * ydim floats, 4 bytes each)
	//variable header		(header size number of bytes)	
	GetLocalTime(&st);
	curTime = st.wMilliseconds+1000*(st.wSecond+60*(st.wMinute+60*st.wHour));
	curFrame.setTimeStamp(curTime);
	//write the data
	unsigned int w = curFrame.index;
	file.write(reinterpret_cast<char *>(&w),sizeof(unsigned int));
	w = curFrame.headerSize();	
	file.write(reinterpret_cast<char *>(&w),sizeof(unsigned int));	
	w = curFrame.xlen();
	file.write(reinterpret_cast<char *>(&w),sizeof(unsigned int));	
	w = curFrame.ylen();
	file.write(reinterpret_cast<char *>(&w),sizeof(unsigned int));
	w = curFrame.timestamp();
	file.write(reinterpret_cast<char *>(&w),sizeof(unsigned int));
	file.write(reinterpret_cast<char *>(curFrame.data()),sizeof(float)*curFrame.xlen()*curFrame.ylen());
	file.write(curFrame.header(),sizeof(char)*curFrame.headerSize());
	
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set after writing frame\n";
		return -1;
	}
	session.numFrames++;
	resetFrame();
	return session.numFrames;	
}

int cdf::CDFSession::writeNumFrames(){
	//first bytes after ascii header are the number of frame bytes
	file.seekp(ASCII_HEADER_SIZE,ios::beg);
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set, did not write number of frames\n";
		return -1;
	}
	file.write(reinterpret_cast<char *>(&(session.numFrames)),sizeof(unsigned int));
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set after writing number of frames\n";
		return -1;
	}
	return 0;
}

int cdf::CDFSession::writeHeader(){
	int res = writeASCIIHeader();
	if(res!=0)//failed
		return res;
	res = writeFixedHeader();
	if(res!=0)//failed
		return res;
	res = writeVariableHeader();
	haveWrittenHeader = true;
	return res;
} 

int cdf::CDFSession::writeASCIIHeader(){	
	file.seekp(0,ios::beg);
	if(file.fail()){
		cerr << "filestream failbit/badbit falg set, did not write readable header\n";
		return -1;
	}
	char *str= new char[ASCII_HEADER_SIZE];
	for(int i=0;i<ASCII_HEADER_SIZE;i++)
		str[i]='*';
	char buffer[ASCII_HEADER_SIZE];
	//subject, date, number of frames
	sprintf_s(buffer,ASCII_HEADER_SIZE,"Subject: %u\r\nDate: %u/%u/%u at %.2u:%.2u\r\nNumber of Frames: %d\r\n\0",
		session.subjectNumber,session.date.month,session.date.day,session.date.year,
		session.date.hour,session.date.minute,session.numFrames);
	string header="";
	header = buffer;
	int size;
	if(header.length()<=ASCII_HEADER_SIZE)
		size = header.length();
	else
		size = ASCII_HEADER_SIZE;
	memcpy(str,header.c_str(),size);
	str[ASCII_HEADER_SIZE-3]='\r';
	str[ASCII_HEADER_SIZE-2]='\n';
	str[ASCII_HEADER_SIZE-1]='\0';
	file.write(str,ASCII_HEADER_SIZE);
	delete[] str;
	return 0;
}

int cdf::CDFSession::writeFixedHeader(){
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set, did not write fixed header\n";
		return -1;
	}
	//getVariableHeaderSize(si);
	file.seekp(ASCII_HEADER_SIZE,ios::beg);
	file.write(reinterpret_cast<char *>(&(session)),sizeof(SessionInfo));
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set after writing fixed header\n";
		return -1;
	}
	return 0;
}

int cdf::CDFSession::writeVariableHeader(){
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set, did not write variable header\n";
		return -1;
	}
	//get to the right spot in the file
	file.seekp(sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);
	file.write(varHeader,session.variableHeaderSize);
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set after writing variable header\n";
		return -1;
	}
	return 0;
}

cdf::Frame* cdf::CDFSession::getFrames(){
	if(frames != NULL)
		delete[] frames;
	Frame *frames = new Frame[session.numFrames];
	file.seekg(ASCII_HEADER_SIZE+sizeof(SessionInfo)+session.variableHeaderSize,ios::beg);
	for(unsigned int i=0;i<session.numFrames;i++){
		readNextFrame();
		frames[i] = curFrame;
	}
	return frames;
}

void cdf::CDFSession::readNextFrame(){
//read frame info
	char *buffer = new char[sizeof(unsigned int)];
	file.read(buffer,sizeof(unsigned int));
	unsigned int index = *reinterpret_cast<int *>(buffer);
	file.read(buffer,sizeof(unsigned int));
	unsigned int headerSize = *reinterpret_cast<unsigned int *>(buffer);
	file.read(buffer,sizeof(unsigned int));
	unsigned int xlen = *reinterpret_cast<int *>(buffer);
	file.read(buffer,sizeof(unsigned int));
	unsigned int ylen = *reinterpret_cast<int *>(buffer);
		
	curFrame.initialize(index,headerSize,xlen,ylen);	
	
	delete[] buffer;
	//read header data
	buffer = new char[sizeof(char)*headerSize];
	file.read(buffer,sizeof(char)*headerSize);
	curFrame.setHeader(buffer,headerSize);	
	delete[] buffer;
	//read frame data
	buffer = new char[xlen*ylen*sizeof(float)];		
	file.read(buffer,sizeof(float)*xlen*ylen);
	float *fdata = reinterpret_cast<float *>(buffer);
	curFrame.setData(fdata,xlen, ylen);
	delete[] buffer;
}

int cdf::CDFSession::readHeader(){
	readASCIIHeader();
	readFixedHeader();
	readVariableHeader();
	return 0;
}

int cdf::CDFSession::readASCIIHeader(){
	file.seekp(0,ios::beg);	
	file.read(asciiHeader,ASCII_HEADER_SIZE);	
	return 0;
}

int cdf::CDFSession::readFixedHeader(){
	file.seekp(ASCII_HEADER_SIZE,ios::beg);
	char buffer[sizeof(SessionInfo)];
	file.read(buffer,sizeof(SessionInfo));
	SessionInfo *s = reinterpret_cast<SessionInfo *>(buffer);
	session = *s;
	return 0;
}

int cdf::CDFSession::readVariableHeader(){
	clearVarHeader();
	file.seekp(ASCII_HEADER_SIZE+sizeof(SessionInfo),ios::beg);	
	char *buffer = new char[session.variableHeaderSize];
	setVarHeader(buffer, session.variableHeaderSize);
	delete[] buffer;
	return 0;
}

// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// ******************************************************** CDF Definitions ********************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************

int cdf::getChannelIndex(const char *chName){
	int idx = -1;
	int i=0;
	while(i<128&&strcmp(chName,channelNames[i++].c_str()));
	return i-1;
}

std::string cdf::getChannelName(unsigned int i){
	return cdf::channelNames[i];
}
/*
cdf::Frame* cdf::getFrames(fstream &fin, SessionInfo &si){
	Frame *frames = new Frame[si.numFrames];
	fin.seekg(ASCII_HEADER_SIZE+sizeof(SessionInfo)+si.variableHeaderSize,ios::beg);
	for(unsigned int i=0;i<si.numFrames;i++)
		readFrame(fin,frames[i]);
	return frames;
}

void cdf::getFrames(fstream &fin, SessionInfo &si, Frame *frames){
	//TODO
}
cdf::Frame *cdf::readFrameIndex(fstream &fin,SessionInfo &si,unsigned int index){
	Frame *fi;
	if(si.numFrames<index)
		throw outOfBoundsError;
	fin.seekg(si.variableHeaderSize+sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);
	fi = readFrame(fin);
	while(!fin.eof() && fi->index!=index){
		delete fi;
		fi = readFrame(fin);
	}
	return fi;
}

cdf::Frame *cdf::readFrameIndex(fstream &fin, unsigned int index){
	Frame *fi;	
	char *buffer = new char[32];
	fin.seekg(ASCII_HEADER_SIZE,ios::beg);
	fin.read(buffer,sizeof(SessionInfo));
	SessionInfo si = *reinterpret_cast<SessionInfo *>(buffer);
	
	if(si.numFrames<index)
		throw outOfBoundsError;
	fin.seekg(si.variableHeaderSize+sizeof(SessionInfo),ios::beg);
	fi = readFrame(fin);
	while(!fin.eof() && fi->index!=index){
		delete fi;
		fi = readFrame(fin);
	}
	return fi;
}
*/

void cdf::printFrame(Frame *frame){
	printf("\tFrame %d\n",frame->index);
	printf("%d x %d\n",frame->xlen(),frame->ylen());
	if(frame->size()!=0){
		printf("===== Header =====\n%s\n",frame->header());
		printf("==================\n");
	}
	else{
		printf("==== No Header ====\n");
	}

	for(unsigned int i=0;i<frame->ylen();i++){
		for(unsigned int j=0;j<frame->xlen();j++){
			printf("%8.3f ",frame->data()[i*frame->xlen()+j]);
		}
		printf("\n");
	}
}