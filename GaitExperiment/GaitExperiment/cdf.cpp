#include "stdafx.h"
#include "cdf.h"

using namespace std;

cdf::OutOfBoundsError outOfBoundsError;

cdf::SessionInfo cdf::defaultSession(){
	cdf::SessionInfo si;
	
	si.dataType = CDF_RAW;
	si.numFrames = 0;
	si.numActions = 0;
	si.numCOI = 64;
	si.numElec = 64;
	si.numFreq = 1;
	si.subjectNumber = 26;
	
	time_t curTime = time(NULL);
	struct tm *timeinfo;
	timeinfo = localtime(&curTime);	
	si.date.minute = timeinfo->tm_min;
	si.date.hour = timeinfo->tm_hour;
	si.date.day = timeinfo->tm_mday;
	si.date.month = timeinfo->tm_mon;
	si.date.year = timeinfo->tm_year+1900;

	si.trialType = 0;

	getVariableHeaderSize(si);

	return si;	
}


void cdf::Coi::initialize(int e){
	numElec = e;
	chanMat = new float[numElec];
	for(int i=0;i<numElec;i++)
		chanMat[i]=0;
}

cdf::Coi::~Coi(){
	cleanup();
}
void cdf::Coi::cleanup(){
	if(chanMat!=NULL)
		delete[] chanMat;
}
cdf::Coi& cdf::Coi::operator=(const cdf::Coi& c){
	if(this == &c)
		return *this;
	if(c.numElec<0)
		return *this;
	if(this->numElec!=c.numElec){
		this->cleanup();
		this->initialize(c.numElec);
	}
	for(int i=0;i<this->numElec;i++)
		this->chanMat[i]=c.chanMat[i];
	return *this;
}
void cdf::Action::initialize(int nc, int nf){
	numCOI = nc;
	numFreq = nf;
	if(numCOI>=0 && numFreq>=0){
		weights=new float[numCOI*numFreq];
		for(int i=0;i<numCOI*numFreq;i++) 
			weights[i]=0;
	}
}
void cdf::Action::cleanup(){
	if(weights!=NULL)
		delete[] weights;
}

cdf::Action::~Action(){
	cleanup();
}

cdf::Action& cdf::Action::operator=(const cdf::Action& a){
	if(this == &a)
		return *this;
	if(this->numCOI!=a.numCOI || this->numFreq!=a.numFreq){
		cleanup();
		this->initialize(a.numCOI,a.numFreq);
	}
	for(int i=0;i<this->numCOI*this->numFreq;i++){
		this->weights[i]=a.weights[i];
	}
	return *this;
}

// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// ************************************************** Variable Header Definitions **************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************

void cdf::VariableHeader::pointerSetup(){
	coiNames    = NULL;
	actionNames = NULL;
	cois		= NULL;
	actions		= NULL;
	freqs		= NULL;
}

cdf::VariableHeader::VariableHeader(SessionInfo &si){
	pointerSetup();
	initialize(si);
}

cdf::VariableHeader::~VariableHeader(){
	cleanup();
}

void cdf::VariableHeader::initializeCOIs(SessionInfo &si){
	if(coiNames!=NULL)
		delete[] coiNames;
	coiNames = new string[si.numCOI];
	if(cois!=NULL)
		delete[] cois;
	cois = new Coi[si.numCOI];
	for(int i=0;i<si.numCOI;i++)
		cois[i].initialize(si.numElec);
}

void cdf::VariableHeader::initializeActions(SessionInfo &si){
	if(actionNames!=NULL)
		delete[] actionNames;
	actionNames = new string[si.numActions];
	if(actions!=NULL)
		delete[] actions;
	actions = new Action[si.numActions];
	for(int i=0;i<si.numActions;i++)
		actions[i].initialize(si.numCOI,si.numFreq);
}
void cdf::VariableHeader::initialize(SessionInfo &si){
	this->session=&si;
	initializeCOIs(si);
	initializeActions(si);
	freqs = new float[si.numFreq];
}
void cdf::VariableHeader::cleanup(){	
	if(coiNames!=NULL){
		delete[] coiNames;
		coiNames = NULL;
	}
	if(actionNames!=NULL){
		delete[] actionNames;
		actionNames = NULL;
	}
	if(cois!=NULL){
		delete[] cois;
		cois = NULL;
	}
	if(actions!=NULL){
		delete[] actions;
		actions = NULL;
	}
	if(freqs!=NULL){
		delete[] freqs;
		freqs = NULL;
	}
}

cdf::VariableHeader& cdf::VariableHeader::operator=(const VariableHeader &vh){
	if(this==&vh)
		return *this;
 	this->cleanup();
	//si=*(vh.session);
	this->initialize(*(vh.session));
	this->coiFilename=vh.coiFilename;
	for(int i=0;i<vh.session->numCOI;i++){
		this->coiNames[i]=vh.coiNames[i];
		this->cois[i]=vh.cois[i];
	}
	this->actionFilename=vh.actionFilename;
	for(int i=0;i<vh.session->numActions;i++){
		this->actionNames[i]=vh.actionNames[i];
		this->actions[i]=vh.actions[i];
	}
	for(int i=0;i<this->session->numFreq;i++){
		this->freqs[i]=vh.freqs[i];
	}
	return *this;
}

// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// ***************************************************** FrameInfo Definitions *****************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************

void cdf::FrameInfo::initialize(int idx, int hs, int x, int y){
	index=idx;
	headerSize=hs;
	xlen=x;
	ylen=y;

	if(xlen*ylen>CDF_FI_MAX_XLEN*CDF_FI_MAX_YLEN){
		xlen = CDF_FI_MAX_XLEN;
		ylen = CDF_FI_MAX_YLEN;
		cerr << "size of frame too large, reverting to default size of " << xlen << "-by-" << ylen << endl;
	}
	m = ylen;
	n = xlen; 
	cleanup();
	pointerSetup();
	header = new char[headerSize];	
	for(unsigned int i=0;i<headerSize;i++)
		header[i]='\0';	
	for(unsigned int i=0;i<xlen*ylen;i++)
		data[i]=0;
}

void cdf::FrameInfo::pointerSetup(){
	header=NULL;
	//data=NULL;
}

cdf::FrameInfo::~FrameInfo(){
	cleanup();
}

void cdf::FrameInfo::cleanup(){
	if(header!=NULL)
		delete[] header;
	header = NULL;
	for(int i=0;i<CDF_FI_MAX_XLEN*CDF_FI_MAX_YLEN;i++)
		data[i] = 0;
	//memset(data,0,sizeof(float)*sizeof(data));
}

bool cdf::FrameInfo::setHeader(char *in, unsigned int size){
	if(this->header!=NULL)
		delete this->header;
	this->header = new char[size];
	this->headerSize = size;	
	memcpy(this->header,in,size);
	return true;
}

bool cdf::FrameInfo::setData(float *in, unsigned int x, unsigned int y){
	xlen = x;
	ylen = y;
	m = ylen;
	n = xlen;
	if(x*y>CDF_FI_MAX_XLEN*CDF_FI_MAX_YLEN){
		cerr << "buffer overflow, " << y << " by " << x << " is too big, cutting down size\n";
		memcpy(this->data,in,CDF_FI_MAX_XLEN*CDF_FI_MAX_YLEN*sizeof(float));
	}
	else
		memcpy(this->data,in,x*y*sizeof(float));
	return true;
}

void cdf::FrameInfo::setDefault(){
	xlen = n = 0;
	ylen = m = 0;
	headerSize = 0;
	cleanup();
	pointerSetup();

}

// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// **************************************************** CDF Session Definitions ****************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************
// *********************************************************************************************************************************

cdf::CDFSession::CDFSession(){
	pointerSetup();
	setDefaults();
}

cdf::CDFSession::CDFSession(SessionInfo& si, VariableHeader& vh){
	pointerSetup();
	setSession(si);
	setVarHeader(vh);
}

void cdf::CDFSession::pointerSetup(){
	frames=NULL;
	asciiHeader=NULL;
	varHeader.session=&(this->session);
}


void cdf::CDFSession::initialize(){
	pointerSetup();

}

cdf::CDFSession::~CDFSession(){
	cleanup();
}

void cdf::CDFSession::cleanup(){
	if(file.is_open()){
		writeNumFrames();
		cdf::writeASCIIHeader(file,session,varHeader);
		file.close();
	}
	if(frames!=NULL){
		delete[] frames;
		frames = NULL;
	}
	if(asciiHeader!=NULL){
		delete[] asciiHeader;
		asciiHeader = NULL;
	}
}

void cdf::CDFSession::printCOI(){
	printf("================\n");
	printf("      COIs      \n");
	printf("================\n");
	for(int i=0;i<session.numCOI;i++){
		printf("\t%s\n",varHeader.coiNames[i].c_str());
		for(int j=0;j<session.numElec;j++){
			if(varHeader.cois[i].chanMat[j]!=0)
				printf("%3s[%2d]:\t%.4f\n",cdf::channelNames[j].c_str(),j,varHeader.cois[i].chanMat[j]);				
		}
		cout << endl;
	}
}
	
void cdf::CDFSession::printActions(){
	printf("===================\n");
	printf("      Actions      \n");
	printf("===================\n");
	for(int i=0;i<session.numActions;i++){
		cout << varHeader.actionNames[i] << "\n";
		for(int j=0;j<session.numCOI;j++){
			for(int k=0;k<session.numFreq;k++){
				printf("%8.3f ",varHeader.actions[i].weights[j*session.numFreq+k]);
			}
			cout << endl;
		}
		cout << endl;
	}
}
void cdf::CDFSession::printAsciiHeader(){
	printf("%s\n",asciiHeader);
}
void cdf::CDFSession::printFrame(unsigned int idx){
	if(idx<session.numFrames && idx>=0){
		printf("=============================\n");
		printf("     Frame %d [index %d]     \n",frames[idx].index,idx);
		printf("=============================\n");
		printf("size: %d x %d\n",frames[idx].xlen,frames[idx].ylen);
		if(frames[idx].size()!=0){
			printf("===== Header =====\n%s\n",frames[idx].header);
			printf("==================\n");
		}
		else{
			printf("==== No Header ====\n");
		}

		for(unsigned int i=0;i<frames[idx].ylen;i++){
			for(unsigned int j=0;j<frames[idx].xlen;j++){
				printf("%8.3f ",frames[idx].data[i*frames[idx].xlen+j]);
			}
			printf("\n");
		}
		printf("\n");
	}
	else
		printf("Invalid frame: %d\n",idx);
}

void cdf::CDFSession::printFreqs(){
	printf("=====================\n");
	printf("     Frequencies     \n");
	printf("=====================\n");
	for(int i=0;i<session.numFreq;i++){
		printf("%5.2f\n",varHeader.freqs[i]);
	}
	printf("\n");
		
}

void cdf::CDFSession::setSubNum(unsigned int sn){
	session.subjectNumber = sn;	
}

bool cdf::CDFSession::setFrameData(float *in, unsigned int x, unsigned int y){
	return curFrame.setData(in,x,y);	
}

bool cdf::CDFSession::setFrameHeader(char *in, unsigned int size){
	return curFrame.setHeader(in, size);
}

void cdf::CDFSession::setSession(SessionInfo &si){
	this->session=si;
}

void cdf::CDFSession::setVarHeader(VariableHeader &vh){
	setSession(*(vh.session));	
	this->varHeader=vh;
	this->varHeader.session=&session;
}

//void cdf::CDFSession::setNumFrames(int nf){
//	this->session.numFrames=nf;
//}

void cdf::CDFSession::setFreqs(float *freqs, int numFreqs){
	delete[] varHeader.freqs;
	varHeader.freqs = new float[numFreqs];
	varHeader.session->numFreq = numFreqs;
	for(int i=0;i<numFreqs;i++){
		varHeader.freqs[i]=freqs[i];
	}
}

void cdf::CDFSession::setDefaults(){
	session = cdf::defaultSession();	
	varHeader.initialize(session);	
	setDefaultCOI();
	setDefaultAction();
	float f[1];
	f[0] = 1;
	setFreqs(f,1);
	// TODO, FIX INITIALIZATIONS OF FREQUENCY/CHANNEL NAMES, ETC
}

int cdf::CDFSession::close(){
	cleanup();
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
	asciiHeader = cdf::readASCIIHeader(file);
	session = cdf::readFixedHeader(file);
	try
	{
		cdf::readVariableHeader(file, session, varHeader);
	}
	catch(FileReadError f){
		cerr << "file read error of \"" << filename <<"\""<< endl;
		return CDF_FILE_READ_ERROR;
	}
	frames = cdf::getFrames(file, session);
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
		}
		else{//file already existed, so read its contents
			asciiHeader = new char[ASCII_HEADER_SIZE];
			file.read(asciiHeader,sizeof(char)*ASCII_HEADER_SIZE);
			session = cdf::readFixedHeader(file);
			cdf::readVariableHeader(file,session,varHeader);
			frames = cdf::getFrames(file, session);
		}
	}
	return !file.good();
}

cdf::FrameInfo* cdf::CDFSession::getFrames(){
	return cdf::getFrames(file,session);
}

int cdf::CDFSession::readCharCOI(char *filename){
	try{
		cdf::readCharCOI(filename,session,varHeader);
	}
	catch(FileReadError f){
		cerr << "file read error of \"" << filename << "\"\n";
		return -2;
	}
	getVariableHeaderSize(session);
	return 0;
}

int cdf::CDFSession::setDefaultCOI(){
	cdf::setDefaultCOI(session, varHeader);
	return 0;
}

int cdf::CDFSession::setDefaultAction(){
	cdf::setDefaultAction(session, varHeader);
	return 0;
}

int cdf::CDFSession::readCharActions(char *filename){
	try{
		cdf::readCharActions(filename,session,varHeader);
	}
	catch(FileReadError f){
		cerr << "file read error of \"" << filename << "\"\n";
		return -2;
	}
	getVariableHeaderSize(session);
	return 0;
}

int cdf::CDFSession::writeFrame(FrameInfo *fp){
	fp->index = session.numFrames;
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
	//header size			(unsigned int, 4 bytes)
	//size in x dimension	(unsigned int, 4 bytes)
	//size in y dimension	(unsigned int, 4 bytes)
	//variable header		(header size number of bytes)
	//data					(xdim * ydim floats, 4 bytes each)
	
	//write the data
	file.write(reinterpret_cast<char *>(&(fp->index)),sizeof(unsigned int));
	unsigned int size = fp->size();	
	file.write(reinterpret_cast<char *>(&(size)),sizeof(unsigned int));	
	file.write(reinterpret_cast<char *>(&(fp->xlen )),sizeof(unsigned int));	
	file.write(reinterpret_cast<char *>(&(fp->ylen )),sizeof(unsigned int));
	//cout << "header size " << frame->headerSize << endl << frame->header << endl;	
	file.write(fp->header,sizeof(char)*size);	
	file.write(reinterpret_cast<char *>(fp->data),sizeof(float)*fp->xlen*fp->ylen);
	if(file.fail()){
		cerr << "filestream failbit/badbit flag set after writing frame\n";
		return -1;
	}
	session.numFrames++;
	return session.numFrames;	
}

int cdf::CDFSession::writeFrame(){	
	return cdf::writeFrame(file,&curFrame);
}

int cdf::CDFSession::writeNumFrames(int numf){
	return cdf::writeNumFrames(this->file,numf);
}

int cdf::CDFSession::writeNumFrames(){
	return cdf::writeNumFrames(this->file,this->session);
}

int cdf::CDFSession::writeHeader(){
	return cdf::writeHeader(file,this->session,this->varHeader);
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

int cdf::writeASCIIHeader(fstream &fout, SessionInfo &si, VariableHeader &vh){	
	fout.seekp(0,ios::beg);
	if(fout.fail()){
		cerr << "filestream failbit/badbit falg set, did not write readable header\n";
		return -1;
	}
	char *str= new char[ASCII_HEADER_SIZE];
	for(int i=0;i<ASCII_HEADER_SIZE;i++)
		str[i]='*';
	char buffer[ASCII_HEADER_SIZE];
	//subject, date, number of frames
	sprintf(buffer,"Subject: %u\r\nDate: %u/%u/%u at %.2u:%.2u\r\nNumber of Frames: %d\r\n\0",
		si.subjectNumber,si.date.month,si.date.day,si.date.year,
		si.date.hour,si.date.minute,si.numFrames);
	string header="";
	header = buffer;
	//COI filename
	sprintf(buffer,"COI filename: %s\r\n",vh.coiFilename.c_str());
	header = header+buffer;
	//number of COIs
	sprintf(buffer,"Num COIs: %u\r\n",si.numCOI);
	header = header+buffer;
	//the COIs
	for(int i=0;i<si.numCOI;i++){
		sprintf(buffer,"%s\r\n",vh.coiNames[i].c_str());
		header = header+buffer;
	}
	//Action filename
	sprintf(buffer,"Action filename: %s\r\n",vh.actionFilename.c_str());
	header = header+buffer;
	//number of actions
	sprintf(buffer,"Num Actions: %u\r\n",si.numActions);
	header = header+buffer;
	//the actions
	for(int i=0;i<si.numActions;i++){
		sprintf(buffer,"%s\r\n",vh.actionNames[i].c_str());
		header = header+buffer;
	}
	//cout << header.length() << endl;
	int size;
	if(header.length()<=ASCII_HEADER_SIZE)
		size = header.length();
	else
		size = ASCII_HEADER_SIZE;
	memcpy(str,header.c_str(),size);
	str[ASCII_HEADER_SIZE-3]='\r';
	str[ASCII_HEADER_SIZE-2]='\n';
	str[ASCII_HEADER_SIZE-1]='\0';
	fout.write(str,ASCII_HEADER_SIZE);
	delete[] str;
	return 0;
}
int cdf::writeASCIIHeader(fstream &fout, SessionInfo &si, string *coiNames, string *actionNames){
	fout.seekp(0,ios::beg);
	if(fout.fail()){
		cerr << "filestream failbit/badbit falg set, did not write readable header\n";
		return -1;
	}
	char *str= new char[ASCII_HEADER_SIZE];
	for(int i=0;i<ASCII_HEADER_SIZE;i++)
		str[i]='*';
	char buffer[ASCII_HEADER_SIZE];
	//subject, date, number of frames
	sprintf(buffer,"Subject: %u\r\nDate: %u/%u/%u at %.2u:%.2u\r\nNumber of Frames: %d\r\n\0",
		si.subjectNumber,si.date.month,si.date.day,si.date.year,
		si.date.hour,si.date.minute,si.numFrames);
	string header="";
	header = buffer;
	//number of COIs
	sprintf(buffer,"Num COIs: %u\r\n",si.numCOI);
	header = header+buffer;
	//the COIs
	for(int i=0;i<si.numCOI;i++){
		sprintf(buffer,"%s\r\n",coiNames[i].c_str());
		header = header+buffer;
	}
	//number of actions
	sprintf(buffer,"Num Actions: %u\r\n",si.numActions);
	header = header+buffer;
	for(int i=0;i<si.numActions;i++){
		sprintf(buffer,"%s\r\n",actionNames[i].c_str());
		header = header+buffer;
	}
	//cout << header.length() << endl;
	int size;
	if(header.length()<=ASCII_HEADER_SIZE)
		size = header.length();
	else
		size = ASCII_HEADER_SIZE;
	memcpy(str,header.c_str(),size);
	str[ASCII_HEADER_SIZE-3]='\r';
	str[ASCII_HEADER_SIZE-2]='\n';
	str[ASCII_HEADER_SIZE-1]='\0';
	fout.write(str,ASCII_HEADER_SIZE);
	delete[] str;
	return 0;
}
void cdf::getVariableHeaderSize(SessionInfo &si){
	si.variableHeaderSize = si.numCOI*(sizeof(char)*NAME_SIZE+sizeof(float)*si.numElec) +
		si.numActions*(sizeof(float)*si.numCOI*si.numFreq+sizeof(char)*NAME_SIZE) +
		sizeof(char)*FILENAME_SIZE*2 + sizeof(float)*si.numFreq;
}

int cdf::writeFixedHeader(fstream &fout,SessionInfo &si, VariableHeader &vh){
	//variable header size is size of COIS + size of Actions
	//size of a COI is numElectrodes*float(data) + NAME_SIZE*char (name)
	//size of an Action is numFreq*numCOI*float(weights) + NAME_SIZE*char (name)+
	//numCOI*NAME_SIZE*char
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set, did not write fixed header\n";
		return -1;
	}
	getVariableHeaderSize(si);
	fout.write(reinterpret_cast<char *>(&(si)),sizeof(SessionInfo));
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set after writing fixed header\n";
		return -1;
	}
	return 0;
}

int cdf::writeVariableHeader(fstream &fout, char *coiFile, unsigned int coiFileSize, char *actionsFile, unsigned int actionsFileSize){
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set, did not write variable header\n";
		return -1;
	}
	fout.write(reinterpret_cast<char *>(&coiFileSize),sizeof(unsigned int));
	fout.write(reinterpret_cast<char *>(&actionsFileSize),sizeof(unsigned int));
	fout.write(coiFile,coiFileSize);
	fout.write(actionsFile,actionsFileSize);
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set after writing variable header\n";
		return -1;
	}
	return 0;
}

int cdf::writeVariableHeader(fstream &fout, SessionInfo &si, string *coiNames, string *actionNames, Coi *cois,  Action *actions, string filename){
	//sanity check
	//int pos = fout.tellp();
	//cout << pos << "\t" << sizeof(SessionInfo) << endl;
	fout.seekp(sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);

	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set, did not write variable header\n";
		return -1;
	}
	writeCOIActionNames(fout, si, coiNames, actionNames);
	/*
	//**************write the names***********
	//COIs
	std::string slop = "                              ";
	for(unsigned int i=0;i<si.numCOI;i++){			
		int l = coiNames[i].length();		
		if(l < NAME_SIZE){
			fout.write(coiNames[i].c_str(),sizeof(char)*l);			
			fout.write(nameSlop,sizeof(char)*(NAME_SIZE-l));
		}
		else
			fout.write(coiNames[i].c_str(),sizeof(char)*NAME_SIZE);
	}
	//Actions
	for(unsigned int i=0;i<si.numActions;i++){
		int l = actionNames[i].length();
		if(l < NAME_SIZE){
			fout.write(actionNames[i].c_str(),sizeof(char)*l);
			fout.write(nameSlop,sizeof(char)*(NAME_SIZE-l));
		}
		else
			fout.write(actionNames[i].c_str(),sizeof(char)*NAME_SIZE);
	}
	*/
	//******************data******************
	//COIs
	for(unsigned int i=0;i<si.numCOI;i++)
		fout.write(reinterpret_cast<char *>(cois[i].chanMat),sizeof(float)*si.numElec);
	//Actions
	for(unsigned int i=0;i<si.numActions;i++)
			fout.write(reinterpret_cast<char *>(actions[i].weights),sizeof(float)*si.numCOI*si.numFreq);
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set after writing variable header\n";
		return -1;
	}
	return 0;
}

// internal, do not use
int cdf::writeCOIActionNames(fstream &fout, SessionInfo &si, string *coiNames, string *actionNames){
	
	//**************write the names***********
	//COIs	
	for(unsigned int i=0;i<si.numCOI;i++){			
		int l = coiNames[i].length();		
		if(l < NAME_SIZE){
			fout.write(coiNames[i].c_str(),sizeof(char)*l);			
			fout.write(nameSlop,sizeof(char)*(NAME_SIZE-l));
		}
		else
			fout.write(coiNames[i].c_str(),sizeof(char)*NAME_SIZE);
	}
	//Actions
	for(unsigned int i=0;i<si.numActions;i++){
		int l = actionNames[i].length();
		if(l < NAME_SIZE){
			fout.write(actionNames[i].c_str(),sizeof(char)*l);
			fout.write(nameSlop,sizeof(char)*(NAME_SIZE-l));
		}
		else
			fout.write(actionNames[i].c_str(),sizeof(char)*NAME_SIZE);
	}
	return 0;
}

int cdf::writeCOIActionNames(fstream &fout, SessionInfo &si, VariableHeader &vh){
	return writeCOIActionNames(fout, si, vh.coiNames,vh.actionNames);
}
int cdf::writeVariableHeader(fstream &fout, SessionInfo &si, VariableHeader &vh){
	//format:
	//Frequencies
	//COI filenames
	//Action filenames
	//COI names
	//Action names
	//COIs
	//Actions
	fout.seekp(sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);

	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set, did not write variable header\n";
		return -1;
	}
	//write the frequencies first
	fout.write(reinterpret_cast<char *>(vh.freqs),sizeof(float)*si.numFreq);
	//write the filenames, first COI, then Action
	int l = vh.coiFilename.length();	
	if(l < FILENAME_SIZE){
		fout.write(vh.coiFilename.c_str(),sizeof(char)*l);
		fout.write(nameSlop,sizeof(char)*(FILENAME_SIZE - l));
	}
	else
		fout.write(vh.coiFilename.c_str(),sizeof(char)*FILENAME_SIZE);
	l = vh.actionFilename.length();
	if(l < FILENAME_SIZE){
		fout.write(vh.actionFilename.c_str(),sizeof(char)*l);
		fout.write(nameSlop,sizeof(char)*(FILENAME_SIZE - l));
	}
	else
		fout.write(vh.actionFilename.c_str(),sizeof(char)*FILENAME_SIZE);
	
	writeCOIActionNames(fout, si, vh);
	/*
	//**************write the names***********
	//COIs
	for(unsigned int i=0;i<si.numCOI;i++)	
		fout.write(vh.coiNames[i].c_str(),sizeof(char)*NAME_SIZE);
	//Actions
	for(unsigned int i=0;i<si.numActions;i++)
		fout.write(vh.actionNames[i].c_str(),sizeof(char)*NAME_SIZE);
	*/
	//******************data******************
	//COIs
	for(unsigned int i=0;i<si.numCOI;i++)
		fout.write(reinterpret_cast<char *>(vh.cois[i].chanMat),sizeof(float)*si.numElec);
	//Actions
	for(unsigned int i=0;i<si.numActions;i++)
			fout.write(reinterpret_cast<char *>(vh.actions[i].weights),sizeof(float)*si.numCOI*si.numFreq);
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set after writing variable header\n";
		return -1;
	}
	return 0;
}

int cdf::writeHeader(fstream &fout, SessionInfo &si,VariableHeader &vh){
	int res = cdf::writeASCIIHeader(fout, si, vh);
	if(res!=0)//failed
		return res;
	res = cdf::writeFixedHeader(fout,si,vh);
	if(res!=0)//failed
		return res;
	res = cdf::writeVariableHeader(fout,si,vh);
	return res;
}

int cdf::writeFrame(fstream &fout, FrameInfo *frame){
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set, did not write frame\n";
		return -1;
	}
	//make sure you are appending the data
	fout.seekp(0,ios::end);
	//cout << "frame " << frame->index << endl << frame->xlen << " by " << frame->ylen << endl;
	//byte order is:
	//index number			(unsigned int, 4 bytes)
	//header size			(unsigned int, 4 bytes)
	//size in x dimension	(unsigned int, 4 bytes)
	//size in y dimension	(unsigned int, 4 bytes)
	//variable header		(header size number of bytes)
	//data					(xdim * ydim floats, 4 bytes each)
	
	//write the data
	fout.write(reinterpret_cast<char *>(&(frame->index)),sizeof(unsigned int));
	unsigned int size = frame->size();	
	fout.write(reinterpret_cast<char *>(&(size)),sizeof(unsigned int));	
	fout.write(reinterpret_cast<char *>(&(frame->xlen )),sizeof(unsigned int));	
	fout.write(reinterpret_cast<char *>(&(frame->ylen )),sizeof(unsigned int));
	//cout << "header size " << frame->headerSize << endl << frame->header << endl;	
	fout.write(frame->header,sizeof(char)*size);	
	fout.write(reinterpret_cast<char *>(frame->data),sizeof(float)*frame->xlen*frame->ylen);
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set after writing frame\n";
		return -1;
	}
	return 0;
}

int cdf::writeNumFrames(fstream &fout, unsigned int size){
	fout.seekp(ASCII_HEADER_SIZE,ios::beg);
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set, did not write number of frames\n";
		return -1;
	}
	fout.write(reinterpret_cast<char *>(&(size)),sizeof(unsigned int));
	if(fout.fail()){
		cerr << "filestream failbit/badbit flag set after writing number of frames\n";
		return -1;
	}
	return 0;
}

int cdf::writeNumFrames(fstream &fout, SessionInfo &si){
	return writeNumFrames(fout,si.numFrames);
}

cdf::FrameInfo *cdf::readFrame(fstream &fin){	
	//read frame info
	char *buffer = new char[sizeof(unsigned int)];
	fin.read(buffer,sizeof(unsigned int));
	unsigned int index = *reinterpret_cast<int *>(buffer);
	fin.read(buffer,sizeof(unsigned int));
	unsigned int headerSize = *reinterpret_cast<unsigned int *>(buffer);
	fin.read(buffer,sizeof(unsigned int));
	unsigned int xlen = *reinterpret_cast<int *>(buffer);
	fin.read(buffer,sizeof(unsigned int));
	unsigned int ylen = *reinterpret_cast<int *>(buffer);
	
	FrameInfo *fi = new FrameInfo;	
	fi->initialize(index,headerSize,xlen,ylen);	
	
	delete[] buffer;
	//read header data
	buffer = new char[sizeof(char)*headerSize];
	fin.read(buffer,sizeof(char)*headerSize);
	memcpy(fi->header,buffer,sizeof(char)*headerSize);
	delete[] buffer;
	//read frame data
	buffer = new char[xlen*ylen*sizeof(float)];		
	fin.read(buffer,sizeof(float)*xlen*ylen);
	float *fdata = reinterpret_cast<float *>(buffer);
	for(unsigned int y=0;y<fi->ylen;y++){			
		for(unsigned int x=0;x<fi->xlen;x++)			
			fi->data[y*fi->xlen+x]=fdata[y*fi->xlen+x];
	}
	delete[] buffer;
	return fi;
}

void cdf::readFrame(fstream &fin, FrameInfo &frame){	
	//read frame info
	char *buffer = new char[sizeof(unsigned int)];
	fin.read(buffer,sizeof(unsigned int));
	unsigned int index = *reinterpret_cast<int *>(buffer);
	fin.read(buffer,sizeof(unsigned int));
	unsigned int headerSize = *reinterpret_cast<unsigned int *>(buffer);
	fin.read(buffer,sizeof(unsigned int));
	unsigned int xlen = *reinterpret_cast<int *>(buffer);
	fin.read(buffer,sizeof(unsigned int));
	unsigned int ylen = *reinterpret_cast<int *>(buffer);
		
	frame.initialize(index,headerSize,xlen,ylen);	
	
	delete[] buffer;
	//read header data
	buffer = new char[sizeof(char)*headerSize];
	fin.read(buffer,sizeof(char)*headerSize);
	memcpy(frame.header,buffer,sizeof(char)*headerSize);
	delete[] buffer;
	//read frame data
	buffer = new char[xlen*ylen*sizeof(float)];		
	fin.read(buffer,sizeof(float)*xlen*ylen);
	float *fdata = reinterpret_cast<float *>(buffer);
	for(unsigned int y=0;y<frame.ylen;y++){			
		for(unsigned int x=0;x<frame.xlen;x++)			
			frame.data[y*frame.xlen+x]=fdata[y*frame.xlen+x];
	}
	delete[] buffer;
}

cdf::FrameInfo* cdf::getFrames(fstream &fin, SessionInfo &si){
	FrameInfo *frames = new FrameInfo[si.numFrames];
	fin.seekg(ASCII_HEADER_SIZE+sizeof(SessionInfo)+si.variableHeaderSize,ios::beg);
	for(unsigned int i=0;i<si.numFrames;i++)
		readFrame(fin,frames[i]);
	return frames;
}

void cdf::getFrames(fstream &fin, SessionInfo &si, FrameInfo *frames){
	//TODO
}
cdf::FrameInfo *cdf::readFrameIndex(fstream &fin,SessionInfo &si,unsigned int index){
	FrameInfo *fi;
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

cdf::FrameInfo *cdf::readFrameIndex(fstream &fin, unsigned int index){
	FrameInfo *fi;	
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
void cdf::readCOIActionNames(fstream &fin, SessionInfo &si, string **coiNames, string **actionNames){
	*coiNames = new string[si.numCOI];
	*actionNames = new string[si.numActions];
	fin.seekg(sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);
	char *memblock = new char[NAME_SIZE];
	for(int i=0;i<si.numCOI;i++){
		fin.read(memblock,NAME_SIZE*sizeof(char));
		(*coiNames)[i] = memblock;
	}
	for(int i=0;i<si.numActions;i++){
		fin.read(memblock,NAME_SIZE*sizeof(char));
		(*actionNames)[i] = memblock;
	}
	delete[] memblock;
}
char* cdf::readASCIIHeader(fstream &fin){
	char *memblock = new char[sizeof(char)*ASCII_HEADER_SIZE];
	fin.seekg(0,ios::beg);
	fin.read(memblock,sizeof(char)*ASCII_HEADER_SIZE);
	return memblock;
}
cdf::SessionInfo cdf::readFixedHeader(fstream &fin){
	char *memblock=new char[sizeof(SessionInfo)]; //guaranteed to read the numframes, headerSize, and fixed header (4+4+20)
	fin.seekg(ASCII_HEADER_SIZE,ios::beg);
	fin.read(memblock,sizeof(SessionInfo));
	SessionInfo si = *reinterpret_cast<SessionInfo *>(memblock);			
	delete[] memblock;
	return si;
}
cdf::VariableHeader cdf::readVariableHeader(fstream &fin, SessionInfo &si){
	VariableHeader vh(si);
	fin.seekg(sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, could not seek to read variable header\n";
		throw fileReadError;
	}
	//read filenames
	char *buffer = new char[FILENAME_SIZE];
	fin.read(buffer,FILENAME_SIZE);
	vh.coiFilename = buffer;
	fin.read(buffer,FILENAME_SIZE);
	vh.actionFilename = buffer;
	delete[] buffer;
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, failed after reading filenames in variable header\n";
		throw fileReadError;
	}

	//read COI and Action names
	buffer = new char[NAME_SIZE];
	for(int i=0;i<si.numCOI;i++){
		fin.read(buffer,NAME_SIZE*sizeof(char));
		vh.coiNames[i] = buffer;
	}
	for(int i=0;i<si.numActions;i++){
		fin.read(buffer,NAME_SIZE*sizeof(char));
		vh.actionNames[i] = buffer;
	}
	delete[] buffer;

	//read COIs
	unsigned int size = si.numElec*sizeof(float);
	buffer = new char[size];
	for(unsigned int i=0;i<si.numCOI;i++){
		fin.read(buffer,size);	
		float *floatblock = reinterpret_cast<float *>(buffer);
		for(int j=0;j<si.numElec;j++){
			vh.cois[i].chanMat[j]=floatblock[j];
		}
	}
	delete[] buffer;
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, failed after reading COIs in variable header\n";
		throw fileReadError;
	}
	//read actions
	size = sizeof(float)*si.numFreq*si.numCOI;
	buffer = new char[size];
	for(unsigned int j=0;j<si.numActions;j++){
		fin.read(buffer,size);
		float *floatblock = reinterpret_cast<float *>(buffer);
		for(int i=0;i<si.numCOI*si.numFreq;i++)
			vh.actions[j].weights[i]=floatblock[i];
	}
	delete[] buffer;
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, failed after reading actions in variable header\n";
		throw fileReadError;
	}
	return vh;
}

void cdf::readVariableHeader(fstream &fin, SessionInfo &si, VariableHeader &vh){
	vh.initialize(si);
	fin.seekg(sizeof(SessionInfo)+ASCII_HEADER_SIZE,ios::beg);
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, could not seek to read variable header\n";
		throw fileReadError;
	}
	//read frequencies
	char *buffer = new char[sizeof(float)*si.numFreq];
	float *floatBuffer = reinterpret_cast<float *>(buffer);
	fin.read(buffer,sizeof(float)*si.numFreq);
	for(unsigned int i=0;i<si.numFreq;i++){
		vh.freqs[i]=floatBuffer[i];
	}
	delete[] buffer;	

	//read filenames
	buffer = new char[FILENAME_SIZE];
	fin.read(buffer,FILENAME_SIZE);
	vh.coiFilename = buffer;
	fin.read(buffer,FILENAME_SIZE);
	vh.actionFilename = buffer;
	delete[] buffer;
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, failed after reading filenames in variable header\n";
		throw fileReadError;
	}

	//read COI and Action names
	buffer = new char[NAME_SIZE];
	for(int i=0;i<si.numCOI;i++){
		fin.read(buffer,NAME_SIZE*sizeof(char));
		vh.coiNames[i] = buffer;
	}
	for(int i=0;i<si.numActions;i++){
		fin.read(buffer,NAME_SIZE*sizeof(char));
		vh.actionNames[i] = buffer;
	}
	delete[] buffer;

	//read COIs
	unsigned int size = si.numElec*sizeof(float);
	buffer = new char[size];
	for(unsigned int i=0;i<si.numCOI;i++){
		fin.read(buffer,size);	
		float *floatblock = reinterpret_cast<float *>(buffer);
		for(int j=0;j<si.numElec;j++){
			vh.cois[i].chanMat[j]=floatblock[j];
		}
	}
	delete[] buffer;
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, failed after reading COIs in variable header\n";
		throw fileReadError;
	}
	//read actions
	size = sizeof(float)*si.numFreq*si.numCOI;
	buffer = new char[size];
	for(unsigned int j=0;j<si.numActions;j++){
		fin.read(buffer,size);
		float *floatblock = reinterpret_cast<float *>(buffer);
		for(int i=0;i<si.numCOI*si.numFreq;i++)
			vh.actions[j].weights[i]=floatblock[i];
	}
	delete[] buffer;
	if(fin.fail()){
		cerr << "filestream failbit/badbit flag set, failed after reading actions in variable header\n";
		throw fileReadError;
	}
	getVariableHeaderSize(si);
}

cdf::Coi* cdf::readCOI(fstream &fin,SessionInfo &si){
	unsigned int size = si.numElec*sizeof(float);
	char *memblock = new char[size];
	Coi *cois = new Coi[si.numCOI];
	for(int i=0;i<si.numCOI;i++)
		cois[i].initialize(si.numElec);
	for(unsigned int i=0;i<si.numCOI;i++){
		fin.read(memblock,size);	
		float *floatblock = reinterpret_cast<float *>(memblock);
		for(int j=0;j<si.numElec;j++){
			cois[i].chanMat[j]=floatblock[j];
		}
	}
	delete[] memblock;
	return cois;
}
cdf::Action* cdf::readActions(fstream &fin, SessionInfo &si){
	unsigned int size = sizeof(float)*si.numFreq*si.numCOI;
	char *memblock = new char[size];
	Action* actions = new Action[si.numActions];
	for(unsigned int i=0;i<si.numActions;i++)
		actions[i].initialize(si.numCOI,si.numFreq);

	for(unsigned int j=0;j<si.numActions;j++){
		fin.read(memblock,size);
		float *floatblock = reinterpret_cast<float *>(memblock);
		for(int i=0;i<si.numCOI*si.numFreq;i++)
			actions[j].weights[i]=floatblock[i];
	}
	delete[] memblock;
	return actions;
}
void cdf::readCharCOI(string filename, SessionInfo &si, VariableHeader &vh){
	fstream fin;
	fin.open(filename.c_str());
	string name;
	fin >> name;
	fin >> si.numCOI;
	vh.initializeCOIs(si);
	vh.coiFilename = filename;
	if(name=="IDENTITY"){
		for(int i=0;i<si.numCOI;i++){						
			vh.coiNames[i]=channelNames[i];
			vh.cois[i].chanMat[i]=1;			
		}
	}
	else if(name=="IDENTITYPROJECTION"){
		int junk; //don't need first value of line
		float weight;
		string coiName;
		string chName;
		for(int i=0;i<si.numCOI;i++){
			fin >> junk	 >> coiName >> weight >> chName;
			vh.coiNames[i] = coiName;
			vh.cois[i].chanMat[getChannelIndex(chName.c_str())] = weight;
		}
	}
	else if(name=="LINCOM"){
		int numContribChan;
		for(int coi=0;coi<si.numCOI;coi++){
			fin >> numContribChan >> vh.coiNames[coi];
			float weight;
			string chName;
			for(int chan=0;chan<numContribChan;chan++){
				fin >> weight >> chName;
				vh.cois[coi].chanMat[getChannelIndex(chName.c_str())] = weight;
			}
		}
	}
	fin.close();
}

void cdf::setDefaultCOI(SessionInfo &si, VariableHeader &vh){
	vh.initializeCOIs(si);
	vh.coiFilename = "DEFAULT\0";
	for(int i=0;i<si.numCOI;i++){						
			vh.coiNames[i] = channelNames[i];
			vh.cois[i].chanMat[i]=1;			
	}
}

void cdf::readCharCOI(const char *filename, SessionInfo &si, Coi **cois, string **coiNames){
	fstream fin;
	fin.open(filename);
	string name;
	fin >> name;
	fin >> si.numCOI;	
	(*cois)=new Coi[si.numCOI];
	for(int i=0;i<si.numCOI;i++)
		(*cois)[i].initialize(si.numElec);
	(*coiNames) = new string[si.numCOI];
	if(name=="IDENTITY"){
		for(int i=0;i<si.numCOI;i++){						
			(*coiNames)[i]=channelNames[i];
			(*cois)[i].chanMat[i]=1;			
		}
	}
	else if(name=="IDENTITYPROJECTION"){
		int junk; //don't need first value of line
		float weight;
		string coiName;
		string chName;
		for(int i=0;i<si.numCOI;i++){
			fin >> junk	 >> coiName >> weight >> chName;
			(*coiNames)[i] = coiName;
			(*cois)[i].chanMat[getChannelIndex(chName.c_str())] = weight;
		}
	}
	else if(name=="LINCOM"){
		int numContribChan;
		for(int coi=0;coi<si.numCOI;coi++){
			fin >> numContribChan >> (*coiNames)[coi];
			float weight;
			string chName;
			for(int chan=0;chan<numContribChan;chan++){
				fin >> weight >> chName;
				(*cois)[coi].chanMat[getChannelIndex(chName.c_str())] = weight;
			}
		}
	}
	fin.close();
}

void cdf::setDefaultAction(SessionInfo &si, VariableHeader &vh){
	si.numActions = 0;
	vh.actionFilename = "DEFAULT\0";
	vh.initializeActions(si);
}
void cdf::readCharActions(string filename, SessionInfo &si, VariableHeader &vh){
	fstream fin;
	fin.open(filename.c_str());
	if(!fin.good()||!fin.is_open())
		throw fileReadError;
	fin >> si.numActions;
	vh.initializeActions(si);
	vh.actionFilename = filename;
	for(int action=0;action<si.numActions;action++){
		string cname;
		int numContrib;
		int freqIndex;
		float weight;
		fin >> vh.actionNames[action] >> numContrib;
		for(int coi=0;coi<numContrib;coi++){	
			fin >> cname >> freqIndex >> weight;
			int idx=0;
			while(idx<si.numCOI && cname.compare(vh.coiNames[idx]))
				idx++;
			if(idx==si.numCOI)
				throw outOfBoundsError;
			vh.actions[action].weights[idx*si.numFreq+freqIndex]=weight;			
		}	

	}
	fin.close();
}

void cdf::readCharActions(const char *filename, SessionInfo &si, string **coiNames, Action **actions, string **actionNames){
	fstream fin;
	fin.open(filename);
	if(!fin.good()||!fin.is_open())
		throw fileReadError;
	fin >> si.numActions;
	(*actions) = new Action[si.numActions];
	for(int i=0;i<si.numActions;i++)
		(*actions)[i].initialize(si.numCOI,si.numFreq);
	(*actionNames) = new string[si.numActions];
	for(int action=0;action<si.numActions;action++){
		string cname;
		int numContrib;
		int freqIndex;
		float weight;
		fin >> (*actionNames)[action] >> numContrib;
		for(int coi=0;coi<numContrib;coi++){	
			fin >> cname >> freqIndex >> weight;
			int idx=0;
			while(idx<si.numCOI && cname.compare((*coiNames)[idx]))
				idx++;
			if(idx==si.numCOI)
				throw outOfBoundsError;
			(*actions)[action].weights[idx*si.numFreq+freqIndex]=weight;			
		}	

	}
	fin.close();
}

void cdf::printCOI(SessionInfo &si,Coi *cois, string *coiNames){
	for(int i=0;i<si.numCOI;i++){
		printf("\t%s\n",coiNames[i].c_str());
		for(int j=0;j<si.numElec;j++){
			if(cois[i].chanMat[j]!=0)
				printf("%3s[%2d]:\t%.4f\n",cdf::channelNames[j].c_str(),j,cois[i].chanMat[j]);				
		}
		cout << endl;
	}
}

void cdf::printActions(SessionInfo &si, Action *actions, string *actionNames){
	for(int i=0;i<si.numActions;i++){
		cout << actionNames[i] << "\n";
		for(int j=0;j<si.numCOI;j++){
			for(int k=0;k<si.numFreq;k++){
				printf("%8.3f ",actions[i].weights[j*si.numFreq+k]);
			}
			cout << endl;
		}
		cout << endl;
	}
}
void cdf::printFrame(FrameInfo *frame){
	printf("\tFrame %d\n",frame->index);
	printf("%d x %d\n",frame->xlen,frame->ylen);
	if(frame->size()!=0){
		printf("===== Header =====\n%s\n",frame->header);
		printf("==================\n");
	}
	else{
		printf("==== No Header ====\n");
	}

	for(unsigned int i=0;i<frame->ylen;i++){
		for(unsigned int j=0;j<frame->xlen;j++){
			printf("%8.3f ",frame->data[i*frame->xlen+j]);
		}
		printf("\n");
	}
}