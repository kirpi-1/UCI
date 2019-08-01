#include "stdafx.h"
#include "uANT.h"


#define OUTPUT_BUFFER_SIZE 512*10
float outputBuffer[OUTPUT_BUFFER_SIZE];
unsigned int outputChannel = 0;
unsigned int bufferCount = 0;

uANT::uANT(){
	printf("uANT setup\n");
	invariantSetup();
	registerMessageWindow();
	variantSetup();
}
uANT::~uANT(){
	if(biopacRunning)
		toggleBiopac();
}

void uANT::variantSetup(){
	if(zp!=NULL){
		printf("you possibly just lost your last zp object...\n");
		free(consoleInput);
	}
	biopacWindow = hWnd();
	memset(hmdData,0,5*sizeof(float));
	headerBuffer = new BYTE[INITIAL_HEADER_BUFFER_SIZE];
	memset(headerBuffer,0,INITIAL_HEADER_BUFFER_SIZE);
	headerBufferSize = sizeof(float)*HMD_DATA_SIZE;
	headerAllocatedSize = INITIAL_HEADER_BUFFER_SIZE;
	subNum = 0;
	trialNum = 0;	
	memset(trialList,0,BUFFSIZE*sizeof(int));
	foundBiopacWindow = false;
	biopacRunning = false;

	zp = this;
	uap = this;

	
}
void uANT::setTrialList(int in[], UINT s){
	memcpy(trialList,in,s*sizeof(int));
}
//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Process windows messages.
//-----------------------------------------------------------------------------
LRESULT CALLBACK uANT::m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{		
	
	int numbp = 0;
	//printf("uANT uMsg = 0x%x\n",uMsg);
	if(uap==NULL){		
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYUP:
		*(uap->getbKeyPtr(wParam)) = FALSE;
		break;

	case WM_KEYDOWN:
		*(uap->getbKeyPtr(wParam)) = TRUE;
		//printf("keydown %c\n", wParam);

		if(wParam == 'q' || wParam == 'Q' || wParam == MSG_EXIT){
			printf("Posting quit message\n");
			PostQuitMessage(0);
			return 0;
		}
		else if(wParam == 's' || wParam == 'S'){
			setFileName("test");
			if(!uap->isFileOpen())
				uap->startNewFile();
			uap->toggleSaving();
			numbp = sprintf_s(m_msg,MSG_SIZE, "Saving set to %d", uap->saving());
			verboseMessage();
			
		}
		else if(wParam == '1'){
			uap->toggleNullData();
		}
		else if(wParam == '2'){
			uap->toggleTestData();
		}
		else if(wParam == '3'){
			uap->toggleAmpData();
			
		}
		else if(wParam == 'h' || wParam == 'H'){
			std::cout << "	Q		Quit" << std::endl;
			std::cout << "	S		Toggle saving" << std::endl;
			std::cout << "	H		Print help" << std::endl;
			std::cout << "	I		Print session info" << std::endl;
			std::cout << "	1		Toggle null data" << std::endl;
			std::cout << "	2		Toggle test data" << std::endl;
			std::cout << "	3		Toggle amplifier data" << std::endl;
			std::cout << "	P		Start/Stop trial" << std::endl;

		}
		else if(wParam == 'i' || wParam == 'I'){
			std::cout << "========== Session Info ==========" << std::endl;
			std::cout << "Subject:  " << subNum <<std:: endl;
			std::cout << "Filename: " << this->m_filename << std::endl;
			std::cout << "Date: ";
			printf("%04d-%02d-%02d %02d:%02d\n",cdfs.session.date.year,	cdfs.session.date.month,
																		cdfs.session.date.day,
																		cdfs.session.date.hour,
																		cdfs.session.date.minute);
			std::cout << "Trial: " << trialNum << std::endl;
			
			std::cout << "Using ";
			int t = getDataType();
			if(t == 1)
				printf("null data\n");
			else if(t == 2)
				printf("test data\n");
			else if(t == 3)
				printf("amplifier data\n");
			else
				printf("no data\n");
			std::cout << "==================================" << std::endl;

		}
		else if(wParam == 'p' || wParam == 'P'){
			// if haven't found the biopac window, find it
			if(!foundBiopacWindow){
				LPARAM lp = LPARAM(this);
				EnumWindows(printWindowName, lp);
				//if still haven't found it, don't go any further
				if(!foundBiopacWindow){
					printf("couldn't find AcqKnowledge window\n");
					break;
				}
			}
			if(saving()){
				//if was already saving
				//stop saving, close the file, increase trial number
				printf("End of trial, stopping recording\n");
				toggleSaving(false);
				closeFile();
				trialNum++;
				
			}
			else{
				//if wasn't saving, start new file
				//set up defaults
				printf("Starting new recording\n");
				cdfs.setDefaults();
				if(subNum == 0)
					printf("Did you remember to set subject number?\n");
				cdfs.setSubNum(subNum);
				cdfs.session.trialType = trialNum;
				
				//setup filename
				char c[BUFFSIZE];
				sprintf_s(c,BUFFSIZE,"sub%02d_trial%02d_%04d_%02d_%02d_%02d_%02d",subNum,trialNum,cdfs.session.date.year,
																						cdfs.session.date.month,
																						cdfs.session.date.day,
																						cdfs.session.date.hour,
																						cdfs.session.date.minute);
				setFileName(c);

				if(!uap->startNewFile()){
					printf("could not open new file, aborting\n");
					throw(cdf::fileReadError);
				}
				toggleSaving(true);				
			}
			//in either case, send a stop/start signal to biopac window		
			toggleBiopac();
			
	
		}
		else{
			zANT::m_MsgProc(hWnd,uMsg,wParam,lParam);		
		}
		break;
	}

	if (uMsg == uap->getMSG_ONNEWDATA())
		return uap->OnNewData();


	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool uANT::toggleBiopac(){
	_beginthreadex(NULL,0,toggleBiopacHelper,biopacWindow,0,0);
	return biopacRunning = !biopacRunning;	
}

unsigned int __stdcall toggleBiopacHelper(void *param){
	// send CTRL_SPACE to biopac window
	HWND biopacWindow = (HWND)(param);
	HWND currentWindow = GetForegroundWindow();
	INPUT p[4];
	memset(p,0,sizeof(INPUT)*4);
	for(int i=0;i<4;i++)
		p[i].type = INPUT_KEYBOARD;			
	p[0].ki.wVk = VK_CONTROL;
	p[1].ki.wVk = VK_SPACE;
	p[2].ki.wVk = VK_SPACE;
	p[3].ki.wVk = VK_CONTROL;
	p[2].ki.dwFlags = KEYEVENTF_KEYUP;
	p[3].ki.dwFlags = KEYEVENTF_KEYUP;
	SetForegroundWindow(biopacWindow);	
	SendInput(4,p,sizeof(INPUT));
	Sleep(50); //need a sleep so SendInput has time to process
	SetForegroundWindow(currentWindow);
	return 0;
}
HRESULT uANT::OnNewData()
{	
	//printf("OnNewData() in zANT\n");
	//get new data		
	//if we are doing test data, then get the made up the data
	DWORD result;
	//Check for new HMD data
	result = WaitForSingleObject(gotNewHMDData,NULL);
	if(result == WAIT_OBJECT_0){
		WaitForSingleObject(HMDDataMutex,INFINITE);
		memcpy(headerBuffer,HMDDataBuffer,sizeof(float)*HMD_DATA_SIZE);
		headerBufferSize = HMD_DATA_SIZE*sizeof(float);
		if(verbose()){
			printf("HMD Data\n");
			printf("\t%u\n",reinterpret_cast<unsigned int *>(headerBuffer[0]));
			for(int i=1;i<HMD_DATA_SIZE;i++)
				printf("\t%f\n",reinterpret_cast<float *>(headerBuffer)[i]);
		}
		ResetEvent(gotNewHMDData);
		ReleaseMutex(HMDDataMutex);
	}

	result = WaitForSingleObject(gotNewFrameHeaderData,NULL);
	if(result == WAIT_OBJECT_0){
		unsigned int newSize = frameBufferSize + sizeof(float)*HMD_DATA_SIZE;
		if(newSize>headerAllocatedSize){
			delete[] headerBuffer;
			headerBuffer = new BYTE[newSize];
			headerAllocatedSize = newSize;
		}
		headerBufferSize = newSize;
		memcpy(headerBuffer+sizeof(float)*HMD_DATA_SIZE,frameBuffer,frameBufferSize);
		ResetEvent(gotNewFrameHeaderData);
		SetEvent(wroteFrameHeaderData);
	}
	if(headerBuffer != NULL)
		cdfs.setFrameHeader(headerBuffer,headerBufferSize);	
	return zANT::OnNewData();
}

BOOL CALLBACK printWindowName(__in HWND hwnd,__in LPARAM lParam){
	int length = GetWindowTextLength(hwnd);
	uANT *u = reinterpret_cast<uANT *>(lParam);
	if(length==0)
		return true;
	TCHAR *buffer;
	buffer = new TCHAR[length+1];
	memset(buffer,0,(length+1)*sizeof(TCHAR));
	DWORD res = GetWindowText(hwnd,buffer,length+1); 
	std::string s = buffer;
	//std::cout << "name of window: " << s << std::endl;
	if(s.find(TEXT("AcqKnowledge"))!=std::string::npos && s.find(TEXT(".acq"))!=std::string::npos){
		u->biopacWindow = hwnd;
		u->foundBiopacWindow = true;
		char msg[BUFFSIZE] = {0};
		sprintf_s(msg,BUFFSIZE,"found the BioPac Window (AcqKnowledge:\n\thWnd: %d\n\tnName: %s\n",hwnd,s.c_str());
		u->verboseMessage();
		
	}	
	return true;
}