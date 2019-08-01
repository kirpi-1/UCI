#include "stdafx.h"
#include "icube.h"

extern HANDLE cubeMutex;

ICube::ICube(){
	m_internalQuit = m_externalQuit = NULL;
	m_numOpenTrackers = 0;
	m_threadHandle = NULL;
	p_info = NULL;
	p_outlet = NULL;
}

ICube::ICube(HANDLE eq){
	m_internalQuit = NULL;
	m_externalQuit = eq;	
}

ICube::~ICube(){	
	//BOOL ret;
	ISD_CloseTracker(m_cube.handle);
	//printf("ret %d\n",ret);
	CloseHandle(m_internalQuit);
	if(p_info!=NULL)
		delete p_info;
	if(p_outlet!=NULL)
		delete p_outlet;
}

void ICube::initialize(ISD_TRACKER_HANDLE hTracker){	
	m_openSuccess = 0;	
	m_connected = true;
	//printf("m_openSucess = %d\n", m_openSuccess);
	m_internalQuit = CreateEvent(NULL, TRUE, FALSE, TEXT("InertiaCubeQuitEvent"));
	m_threadHandle = NULL;	
	m_cube.handle = hTracker;							
	ISD_GetTrackerConfig(m_cube.handle,&(m_cube.trackerInfo),FALSE);
	//TODO: check if you need this stations thing, otherwise station is always 1	
	ISD_GetStationConfig(m_cube.handle,&(m_cube.stationInfo),1,FALSE);
	
}

BOOL ICube::boresight(){//BOOL ICube::boresight(WORD stationID){
	if(!m_connected)
		return -1;
	return ISD_Boresight(m_cube.handle,m_cube.stationInfo.ID,true);
}

void ICube::update(){
	if(!m_connected) //sanity check
		return;
	ISD_GetTrackingData(m_cube.handle,&(m_cube.data));
	for(int j=0;j<3;j++){
		m_data.YPR[j] = m_cube.data.Station[0].Euler[j];
		m_data.angVel[j] = m_cube.data.Station[0].AngularVelBodyFrame[j];
	}
	//have to shuffle values since it is recording roll, pitch, yaw
	//need yaw, pitch, roll
	/*
	float temp = m_data.angVel[0];
	m_data.angVel[0] = m_data.angVel[2];
	m_data.angVel[2] = m_data.angVel[1];
	m_data.angVel[1] = temp;
	*/
	//DON'T NEED TO? need to swap pitch and roll as well for whatever reason
	/*
	temp = m_data.YPR[1];
	m_data.YPR[1] = m_data.YPR[2];
	m_data.YPR[2] = temp;
	*/
	for(int j=0;j<4;j++){
		m_data.WXYZ[j] = m_cube.data.Station[0].Quaternion[j];		
	}
}
