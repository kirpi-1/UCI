#pragma once
#include "isense.h"
#include<iostream>
#include<Windows.h>
#include<conio.h>
#include<stdio.h>
#include<process.h>


#define LEFT 0
#define RIGHT 1

struct Cube{
	ISD_TRACKER_HANDLE				handle;
	ISD_TRACKING_DATA_TYPE			data;
	ISD_TRACKER_INFO_TYPE			trackerInfo;
};

struct CubeData{
	float	Euler[3]; //yaw, pitch, roll
	float	Quaternion[4]; //(W,X,Y,Z)
};

class LRCubes{
public:
	LRCubes();
	LRCubes(HANDLE quitEvent);
	~LRCubes();
	Cube		m_cubes[2], *m_left, *m_right;
	WORD		m_numOpenTrackers, m_openSuccess;
	
	HANDLE		m_externalQuit;
	HANDLE		m_internalQuit;
	HANDLE		m_threadHandle;

	BOOL		m_displayCubes;

	void		swapLR();
	HANDLE		start();
	BOOL		stop();
	CubeData	getLeft();
	CubeData	getRight();
	BOOL		boresight();

private:
	void initialize();
	static unsigned __stdcall cubeCallback(void *obj);
};

