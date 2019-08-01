#include "stdafx.h"
#include "icube.h"

extern HANDLE cubeMutex;

LRCubes::LRCubes(){
	m_left = m_right = NULL;	
	m_internalQuit = m_externalQuit = NULL;
	m_numOpenTrackers = 0;
	m_threadHandle = NULL;
	initialize();
}

LRCubes::LRCubes(HANDLE eq){
	m_left = m_right = NULL;
	m_internalQuit = NULL;
	m_externalQuit = eq;	
	initialize();
}

LRCubes::~LRCubes(){	
	//BOOL ret;
	m_left = m_right = NULL;
	ISD_CloseTracker(0);
	//printf("ret %d\n",ret);
	
	CloseHandle(m_internalQuit);
}

void LRCubes::initialize(){	
	m_openSuccess = 0;
	m_numOpenTrackers = 0;
	m_displayCubes = false;
	//for ISD_OpenAllTrackers(), need the destination array to be of size ISD_MAX_TRACKERS or else
	//there is some nasty out-of-bounds memory access violation upon deconstruction
	ISD_TRACKER_HANDLE	Trackers[ISD_MAX_TRACKERS];	
	m_openSuccess = ISD_OpenAllTrackers( (Hwnd) NULL, Trackers, FALSE, FALSE);	
	//printf("m_openSucess = %d\n", m_openSuccess);
	m_internalQuit = CreateEvent(NULL, TRUE, FALSE, TEXT("InertiaCubeQuitEvent"));
	m_threadHandle = NULL;
	if(m_openSuccess < 1)
		printf("Could not connect to trackers\n");
	else{
		ISD_NumOpenTrackers(&m_numOpenTrackers);
		if(m_numOpenTrackers<2){
			printf("did not find 2 trackers\n");			
		}
		else{
			m_cubes[0].handle = Trackers[0];
			m_cubes[1].handle = Trackers[1];
			m_left = &(m_cubes[0]);
			m_right = &(m_cubes[1]);		
			ISD_GetTrackerConfig(m_left->handle,&(m_left->trackerInfo),FALSE);
			ISD_GetTrackerConfig(m_right->handle,&(m_right->trackerInfo),FALSE);
		}
	}
}

void LRCubes::swapLR(){
	Cube *temp;
	temp = m_left;
	m_left = m_right;
	m_right = temp;
}

HANDLE LRCubes::start(){
	//stop first just in case
	stop();
	if(m_openSuccess==0){
		printf("Cubes not connected\n");
		return 0;
	}
	ResetEvent(m_internalQuit);	
	m_threadHandle = (HANDLE) _beginthreadex(NULL, 0, cubeCallback, (void * )this, NULL, NULL);	
	return m_threadHandle;
}

BOOL LRCubes::stop(){
	return SetEvent(m_internalQuit);
}

CubeData LRCubes::getLeft(){		
	CubeData c;
	if(m_openSuccess==0){
		printf("Left cube not connected\n");
		return c;
	}
	
	if(this!=NULL){
		
		for(int i=0;i<3;i++)
			c.Euler[i] = m_left->data.Station[0].Euler[i];
		for(int i=0;i<4;i++)
			c.Quaternion[i] = m_left->data.Station[0].Quaternion[i];
	}
	return c;
}
CubeData LRCubes::getRight(){
	CubeData c;
	if(m_openSuccess==0){
		printf("Right cube not connected\n");
		return c;
	}
	if(this!=NULL){		
		for(int i=0;i<3;i++)
			c.Euler[i] = m_right->data.Station[0].Euler[i];
		for(int i=0;i<4;i++)
			c.Quaternion[i] = m_right->data.Station[0].Quaternion[i];
	}
	return c;
}

BOOL LRCubes::boresight(){
	if(m_openSuccess==0){
		printf("Cubes not connected\n");
		return false;
	}
	printf("Cubes boresighted\n");
	return (ISD_Boresight(m_left->handle,1,true) | ISD_Boresight(m_right->handle,1,true));
}

unsigned __stdcall LRCubes::cubeCallback(void *obj){
	LRCubes *l = reinterpret_cast<LRCubes*>(obj);
	HANDLE quitEvents[2];
	quitEvents[0] = l->m_externalQuit;
	quitEvents[1] = l->m_internalQuit;
	
	DWORD result;
	//printf("started cubes,display cubes is %d\n",l->m_displayCubes);
	while(true){
		
		ISD_GetTrackingData(l->m_left->handle,&(l->m_left->data));
		ISD_GetTrackingData(l->m_right->handle,&(l->m_right->data));
		float cubesLR[6];
		for(int i=0;i<3;i++){
			cubesLR[i]	= l->m_left->data.Station[0].Euler[i];
			cubesLR[3+i]= l->m_right->data.Station[0].Euler[i];
		}
		//WaitForSingleObject(cubeMutex,INFINITE);
		if(l->m_displayCubes){
			//printf("                                                               \r");
			printf("L(% 7.2f, % 7.2f, % 7.2f)\tR(% 7.2f, % 7.2f, % 7.2f)\r",cubesLR[0],
								cubesLR[1],cubesLR[2],cubesLR[3],cubesLR[4],cubesLR[5]);		
		}
		//ReleaseMutex(cubeMutex);
		result = WaitForMultipleObjects(2,quitEvents,FALSE,0);				
		if(result - WAIT_OBJECT_0 <2){ //quitEvent has been signaled			
			DWORD r = WaitForSingleObject(l->m_externalQuit,0);
			//printf("0x%x\t",r);
			printf("Stopped polling cubes, event %d\n", result - WAIT_OBJECT_0);				
			return result;
		}
		Sleep(1);
	}
	
	return 0;
}