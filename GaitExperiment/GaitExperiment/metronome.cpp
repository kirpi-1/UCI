#include "stdafx.h"
#include "metronome.h"


Metronome::Metronome(){
}
Metronome::Metronome(UINT numTrials, UINT *trialList, HANDLE quitEvent, UINT startTrial){	
	setupExperiment(numTrials, trialList, quitEvent, startTrial);
}
Metronome::~Metronome(){
	if(m_trialList != NULL)
		delete[] m_trialList;
	CloseHandle(m_internalQuit);
}

void Metronome::setupExperiment(UINT nt, UINT *tl, HANDLE quitEvent, UINT startTrial){
	m_currentTrial = startTrial;
	m_numTrials = nt;
	if(m_trialList != NULL)
		delete[] m_trialList;
	m_trialList = new UINT[nt];
	memcpy(m_trialList,tl,nt*sizeof(UINT));		
	m_externalQuit = quitEvent;
	m_internalQuit = CreateEvent(NULL, TRUE, FALSE, TEXT("MetronomeQuitEvent"));
	m_metronomeDone = CreateEvent(NULL,TRUE, FALSE, TEXT("MetronomeDoneEvent"));
}

void Metronome::setupMetronome(UINT bpm, float time, UINT count){
	m_bpm = bpm;
	m_time = time;
	m_count = count;
	m_playSound = true;
	wcscpy(m_soundFile, L"click.wav");
}

void Metronome::setSoundFile(char *fn){
	size_t numCharConv;
	mbstowcs_s(&numCharConv,m_soundFile,fn,DEFAULT_SOUNDFILE_LENGTH);
}

int Metronome::loadNextMetronome(){
	m_currentTrial++;
	return loadMetronome();
}

int Metronome::loadMetronome(){
	if(m_currentTrial>=m_numTrials)
		return -1;
	UINT trialType = m_trialList[m_currentTrial];
	if(trialType == 1 || trialType == 4)
		m_bpm = 60;
	else if(trialType == 2 || trialType == 5)
		m_bpm = 90;
	else
		m_bpm = 120;
	if(trialType<=3)
		m_playSound = true;
	else
		m_playSound = false;	
	m_time = METRONOME_DEFAULT_TRIAL_LENGTH;
	m_count = 0;	
	return 0;
}


HANDLE Metronome::start(){
	ResetEvent(m_internalQuit);
	m_threadHandle = (HANDLE) _beginthreadex(NULL, 0, play, (void * )this, NULL, NULL);
	return m_threadHandle;
}

BOOL Metronome::stop(){
	return SetEvent(m_internalQuit);
}

void Metronome::reset(){
	ResetEvent(m_internalQuit);
	ResetEvent(m_metronomeDone);
}
unsigned __stdcall Metronome::play(void *obj){
	Metronome *m = reinterpret_cast<Metronome*>(obj);
	FILETIME ft_start, ft_now;
	ULARGE_INTEGER ul_now, ul_start;
	HANDLE quitEvents[2];
	quitEvents[0] = m->m_externalQuit;
	quitEvents[1] = m->m_internalQuit;

	double totalTime = 0;
	double nextClick = 0;	
	int count = 0;
	double MSPB = (1.0/((double)(m->m_bpm)/60.0)*1000.0);
	double slop = 0;
	bool exit = false;

	GetSystemTimeAsFileTime(&ft_start);
	ul_start.LowPart = ft_start.dwLowDateTime;
	ul_start.HighPart = ft_start.dwHighDateTime;
	
	DWORD result;
	// 4 count metronome cue
	UINT m_countTemp = m->m_count;
	m->m_count = 8;
	
	while(true){		
		GetSystemTimeAsFileTime(&ft_now);
		ul_now.LowPart = ft_now.dwLowDateTime;
		ul_now.HighPart = ft_now.dwHighDateTime;
		totalTime = (ul_now.QuadPart - ul_start.QuadPart)/10000;		

		//cout << totalTime << endl;		
		if(totalTime > nextClick){			
			
			if(count<m->m_count-1){
				PlaySound(m->m_soundFile,NULL,SND_FILENAME);				
				printf("                                                               \r");
				std::cout << "click " << count << std::endl;
			}
			else{
				PlaySound(TEXT("beep.wav"),NULL,SND_FILENAME);
				printf("                                                               \r");
				std::cout << "ding" << std::endl;
			}
			
			
			slop = totalTime - nextClick;
			//cout << totalTime  << " with slop of " << slop << endl;	
			//cout << totalTime - nextClick << endl;
			nextClick = totalTime + MSPB - slop;						
			count++;		
		}
		if(count>=m->m_count){				
			break;
		}
		result = WaitForMultipleObjects(2,quitEvents,FALSE,0);				
		if(result - WAIT_OBJECT_0 <2){ //quitEvent has been signaled			
			//printf("Stopping metronome\n");				
			return result+1;
		}		
	}

	count = 0;
	m->m_count = m_countTemp;
	nextClick -= totalTime;
	totalTime = 0;	

	ft_start = ft_now;
	ul_start = ul_now;
	/*
	GetSystemTimeAsFileTime(&ft_start);
	ul_start.LowPart = ft_start.dwLowDateTime;
	ul_start.HighPart = ft_start.dwHighDateTime;
	*/
	// actual metronome
	while(true){		
		GetSystemTimeAsFileTime(&ft_now);
		ul_now.LowPart = ft_now.dwLowDateTime;
		ul_now.HighPart = ft_now.dwHighDateTime;
		totalTime = (ul_now.QuadPart - ul_start.QuadPart)/10000;		

		//cout << totalTime << endl;		
		if(totalTime > nextClick){
			//only use count or time if they are greater than 0			
			if(m->m_playSound){
				if(count<m->m_count-1 || totalTime < m->m_time*1000 - MSPB)
					PlaySound(m->m_soundFile,NULL,SND_FILENAME);
				else
					PlaySound(TEXT("beep.wav"),NULL,SND_FILENAME);
			}
			printf("                                                               \r");
			std::cout << "click " << count << std::endl;
			slop = totalTime - nextClick;
			//cout << totalTime  << " with slop of " << slop << endl;	
			//cout << totalTime - nextClick << endl;
			nextClick = totalTime + MSPB - slop;						
			count++;		
		}
		if(m->m_count>0 && count>=m->m_count || m->m_time>0 && totalTime > m->m_time*1000){
			break;
		}
		result = WaitForMultipleObjects(2,quitEvents,FALSE,0);				
		if(result - WAIT_OBJECT_0 <2){ //quitEvent has been signaled			
			//printf("Stopping metronome\n");				
			return result+1;
		}		
	}
	//if it makes it here, that means the metronome stopped
	SetEvent(m->m_metronomeDone);
	PlaySound(TEXT("ding.wav"),NULL,SND_FILENAME);
	return 0;
}