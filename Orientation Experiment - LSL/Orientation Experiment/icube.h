#pragma once
#include "stdafx.h"
#include "isense.h"
#include<iostream>
#include<conio.h>
#include<process.h>
#include<lsl_cpp.h>

struct CubeInfo{
	ISD_TRACKER_HANDLE				handle;
	ISD_TRACKING_DATA_TYPE			data;
	ISD_TRACKER_INFO_TYPE			trackerInfo;
	ISD_STATION_INFO_TYPE			stationInfo;
};

#pragma pack(1)
struct CubeData{
	float	YPR[3]; //yaw, pitch, roll
	float	WXYZ[4]; //(W,X,Y,Z)
	float	angVel[3]; //roll, pitch, yaw angular velocity
};

class ICube{
public:
	ICube();
	ICube(HANDLE quitEvent);
	~ICube();

	CubeInfo	m_cube;
	WORD		m_numOpenTrackers, m_openSuccess;
	CubeData	m_data;
	bool		m_connected;

	HANDLE		m_externalQuit;
	HANDLE		m_internalQuit;
	HANDLE		m_threadHandle;	
	
	BOOL		setQuitEvent(HANDLE quitEvent);
	void		initialize(ISD_TRACKER_HANDLE hTracker);	
	void		update();	
	BOOL		boresight();//WORD stationID = 1);

	lsl::stream_info	*p_info;
	lsl::stream_outlet	*p_outlet;
};

