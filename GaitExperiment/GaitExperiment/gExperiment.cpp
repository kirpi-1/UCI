#include "stdafx.h"
#include "isense.h"
#include "gExperiment.h"
#include "metronome.h"


extern gExperiment *gp;
extern LRCubes *lrc;
extern Metronome *mp;
extern HANDLE hMET, hCUBE, clientConnected, cubeMutex;
extern LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#define OUTPUT_BUFFER_SIZE 512*10
float outputBuffer[OUTPUT_BUFFER_SIZE];
unsigned int outputChannel = 0;
unsigned int bufferCount = 0;

gExperiment::gExperiment(){
	printf("gExperiment setup\n");
	invariantSetup();
	registerMessageWindow();
	variantSetup();
}

void gExperiment::variantSetup(){
	if(zp!=NULL){
		printf("you possibly just lost your last zp object...\n");
		free(consoleInput);
	}
	m_useCubes = false;
	m_displayCubes = false;

	zp = this;
	gp = this;
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Process windows messages.
//-----------------------------------------------------------------------------
LRESULT CALLBACK gExperiment::m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{		
	
	char buff[256];
	int numbp = 0;
	//	printf("gexperiment uMsg = 0x%x\n",uMsg);
	if(gp==NULL){		
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYUP:
		*(gp->getbKeyPtr(wParam)) = FALSE;
		break;

	case WM_KEYDOWN:
		*(gp->getbKeyPtr(wParam)) = TRUE;
		//printf("keydown %c\n", wParam);

		if(wParam == 'b' || wParam == 'B'){
			lrc->boresight();
			numbp = sprintf(buff,"Boresight complete");
			DWORD res = WaitForSingleObject(clientConnected, 0);
			if(res == WAIT_OBJECT_0){
				send(gp->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
			}
		}
		else if(wParam == 'c' || wParam == 'C'){
			gp->toggleCubes();
			if(gp->useCubes()){
				//(cubeMutex,INFINITE);
				hCUBE = lrc->start();
				//ReleaseMutex(cubeMutex);
			}
			else{
				//WaitForSingleObject(cubeMutex,INFINITE);
				lrc->stop();
				//ReleaseMutex(cubeMutex);
			}
		}
		else if(wParam == 'd' || wParam == 'D'){
			//WaitForSingleObject(cubeMutex,INFINITE);
			printf("display cubes callback\n");
			lrc->m_displayCubes = !lrc->m_displayCubes;
			//ReleaseMutex(cubeMutex);
		}

		else if(wParam == 'm' || wParam == 'M' || wParam == MSG_START){
			//start the experiment
			//need to make sure cubes are on, have another metronome to use,
			//start a new file, and start saving
			DWORD finished = WaitForSingleObject(hMET,0);
			if(finished == WAIT_TIMEOUT)
				break;
			if(mp->loadMetronome()){
				printf("ran out of trials\n");
				numbp = sprintf(buff,"ran out of trials\n");
				DWORD res = WaitForSingleObject(clientConnected, 0);
				if(res == WAIT_OBJECT_0){					
					int numbs = send(gp->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
					if(numbs==SOCKET_ERROR){
						printf("Error sending, %d\n",GetLastError());
					}
				}
				break;
			}			
			//stop the cubes first, just in case
			lrc->stop();
			printf("starting next metronome\n");
			printf("Current type: %d\t%d bpm %.0f seconds\n",mp->curTrialType(),mp->bpm(),mp->time());
			numbp = sprintf(buff,"Trial %d of %d",mp->curTrial(),mp->numTrials()-1);
			printf("%s\n",buff);
			DWORD res = WaitForSingleObject(clientConnected, 0);
			if(res == WAIT_OBJECT_0){
				//printf("sending to tablet at socket #%d\n",gp->socket.clientSocket);
				int numbs = send(gp->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
				if(numbs==SOCKET_ERROR){
					printf("Error sending, %d\n",GetLastError());
				}
			}
			gp->m_filePartIdx = mp->curTrial();
			gp->cdfs.session.trialType = mp->curTrialType();
			gp->startNewFile();
			gp->toggleCubes(true);
			lrc->boresight();
			lrc->start();						
			gp->toggleSaving(true);						
			hMET = mp->start();
			
			break;
		}
		else if(wParam == 'n' || wParam == 'N'){
			if(!gp->startNewFile()){
				printf("could not open new file, aborting\n");
				throw(cdf::fileReadError);
			}						
		}
		else if(wParam == 'o' || wParam == 'O' || wParam == MSG_STOP){
			gp->toggleSaving(false);
			printf("stopping metronome\n");	
			BOOL b = mp->stop();					
			break;
		}
		else if(wParam == 'q' || wParam == 'Q' || wParam == MSG_EXIT){
			printf("Posting quit message\n");
			PostQuitMessage(0);
			return 0;
		}
		else if(wParam == 'w' || wParam == 'W'){
			printf("starting next RHI trial\n");
			gp->toggleCubes(false);
			gp->startNewFile();
		}
		else if(wParam == 's' || wParam == 'S'){
			gp->toggleSaving();
			numbp = sprintf(m_msg,"Saving set to %d", gp->saving());
			verboseMessage();
			DWORD res = WaitForSingleObject(clientConnected, 0);
			if(res == WAIT_OBJECT_0){
				send(gp->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
			}
			
		}
		else if(wParam == 't' || wParam == 'T'){
			gp->toggleTestData();
			numbp = sprintf(m_msg,"Saving set to %d", gp->saving());
			verboseMessage();
			DWORD res = WaitForSingleObject(clientConnected, 0);
			if(res == WAIT_OBJECT_0){
				send(gp->socket.clientSocket,buff,numbp+1,0); //+1 for the null character at the end
			}
		}
		else if(wParam == '['){
			
			if(outputChannel > 0)
				outputChannel--;
			std::cout << "output channel " << outputChannel << ", " << cdf::channelNames[outputChannel] << std::endl;
		}
		else if(wParam == ']'){
			outputChannel++;
			if(outputChannel>63)
				outputChannel = 63;
			std::cout << "output channel " << outputChannel << ", " << cdf::channelNames[outputChannel] << std::endl;			
		}
		else{
			zANT::m_MsgProc(hWnd,uMsg,wParam,lParam);		
		}
		break;
	}

	if (uMsg == gp->getMSG_ONNEWDATA())
		return gp->OnNewData();


	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HRESULT gExperiment::OnNewData()
{
	
	//printf("OnNewData() in zANT\n");
	//get new data		
	//if we are doing test data, then get the made up the data
	if(m_useTestData){
		cdfs.setFrameData(testData+testDataOffset*DEFAULT_NSAMPLES*DEFAULT_NCHANNELS, DEFAULT_NCHANNELS, DEFAULT_NSAMPLES);		
		testDataOffset++;
		if(testDataOffset>=16)
			testDataOffset = 0;
	}	
	else{
		//the data is in nchan x nsamp, i.e. horizontal with time going left to right
		cdfs.setFrameData(m_oAcquisition.m_buffer,m_oAcquisition.m_nsamp,m_oAcquisition.m_nchan);		
		/*
		int i;
		//move old data back
		for(i=0;i<OUTPUT_BUFFER_SIZE-m_oAcquisition.m_nsamp;i++)
			outputBuffer[i] = outputBuffer[i+m_oAcquisition.m_nsamp];
		unsigned int start = m_oAcquisition.m_nsamp*outputChannel;
		for(i=0;i<m_oAcquisition.m_nsamp;i++)
			outputBuffer[OUTPUT_BUFFER_SIZE-m_oAcquisition.m_nsamp+i] = m_oAcquisition.m_buffer[start+i];
		bufferCount+=m_oAcquisition.m_nsamp;
		if(bufferCount>=10*512){
			//printf("printing output file...");
			bufferCount = 0;
			std::ofstream fout;
			fout.open("bufferOutput.txt",std::ios::out);			
			while(!fout.good()){				
				fout.close();
				fout.clear();
				fout.open("bufferOutput.txt",std::ios::out);
			}
			for(i=0;i<OUTPUT_BUFFER_SIZE;i++){
				//printf("on number %d\n", i);
				fout << outputBuffer[i] << std::endl;
			}
			fout.close();
			//printf("done.\n");
		}
	*/
		//printf("Frame: %d\t\tnumsamples %d\tnumchannels %d\n",m_nframe,*m_nsamp,*m_nchan);	
		
		float cubesLR[6];
		for(int i=0;i<6;i++)
			cubesLR[i] = 0;
		if(m_useCubes && lrc->m_openSuccess!=0){			
			CubeData left = lrc->getLeft();
			CubeData right = lrc->getRight();
			
			for(int i=0;i<3;i++){
				cubesLR[i]	= left.Euler[i];
				cubesLR[3+i]=right.Euler[i];
			}
			
		}
		cdfs.setFrameHeader(reinterpret_cast<char *>(&cubesLR),6*sizeof(float));							
	}
	
	
	//if saving the data
	if(m_saving){
		//printf("num frames = %d\n",this->m_nframe);
		//printf("num frames = %d\n",zp->m_nframe);
		//write the data		
		cdfs.writeFrame();
		//printf("wrote frame\n");
		//reset the frame to default
		
		
	}
	
	return S_OK;
}


void gExperiment::toggleCubes(bool u){
	char *onoff;
	m_useCubes = u;
	(m_useCubes)? onoff="Started":onoff="Stopped";
	printf("%s using cubes\n",onoff);
}

void gExperiment::toggleDisplayCubes(){
	char *onoff;
	m_displayCubes = !m_displayCubes;
	(m_displayCubes)? onoff="Started":onoff="Stopped";
	printf("%s displaying cubes\n",onoff);
}