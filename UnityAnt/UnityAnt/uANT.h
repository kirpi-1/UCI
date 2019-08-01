#pragma once
#include "stdafx.h"
#include "zANT.h"
#include <iostream>
#include <string>

#define BUFFSIZE 512
#define HMD_DATA_SIZE 9
#define INITIAL_HEADER_BUFFER_SIZE 256

extern HANDLE clientConnected, cubeMutex, gotNewFrameHeaderData, wroteFrameHeaderData,HMDDataMutex,gotNewHMDData;
extern LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern void *frameBuffer;
extern BOOL frameHeaderFull;
extern unsigned int frameBufferSize;
extern float HMDDataBuffer[];
BOOL CALLBACK printWindowName(__in HWND hwnd, __in LPARAM lParam);

class uANT:public zANT{
public:
	uANT();
	~uANT();
	HRESULT			OnNewData();
	LRESULT CALLBACK m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual void	variantSetup();		
	
private:
	std::string msg;
public:
	void setMessage(std::string m){msg = m;};
	std::string message(){return msg;};


	HWND biopacWindow;
	bool foundBiopacWindow;
	float hmdData[HMD_DATA_SIZE];
	BYTE *headerBuffer;
	unsigned int headerBufferSize;
	unsigned int headerAllocatedSize;
	unsigned int subNum;
	unsigned int trialNum;
	bool biopacRunning;
	int			 trialList[BUFFSIZE];
	void setupCDF();
	void setTrialList(int in[], UINT s);
	bool toggleBiopac();
	

};
unsigned int __stdcall toggleBiopacHelper(void *param);
static uANT *uap;