#include <opencv2/opencv.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2/highgui/highgui.hpp>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <OVR.h>

#include <stdlib.h>
#include <iostream>


using namespace cv;
using namespace std;

int main(){
	
	VideoCapture leftEye;
	VideoCapture rightEye;
	
	leftEye.open(0);
	rightEye.open(1);
/*
	int idx = 0;
	for(idx=0;idx<10;idx++){
		if(!leftEye.isOpened()){
			leftEye.open(idx);
			printf("left eye at %d\n",idx);
			continue;
		}
		if(!rightEye.isOpened()){
			rightEye.open(idx);		 
			printf("left eye at %d\n",idx);
		}
		if(leftEye.isOpened() && rightEye.isOpened())
			break;
	}*/
		


	if(!leftEye.isOpened()){
		printf("couldn't open left eye\n");
		return -1;
	}

	/*Mat tmp;
	while(true){
		leftEye >> tmp;		
		imshow("whole",tmp);
		if(waitKey(30) >= 0) break;
	}
*/
	
	if(!rightEye.isOpened()){
		printf("couldn't open right eye\n");
		return -1;
	}	
	Mat frameL, frameR, whole;

	//ovr_Initialize();
	//ovrHmd hmd = ovrHmd_Create(0);

	//if(!hmd){
	//	printf("could not find rift\n");
	//	system("PAUSE");
	//	return -1;
	//}

	while(true){
		leftEye >> frameL;		
		rightEye >> frameR;
		hconcat(frameL, frameR,whole);
		imshow("whole",whole);
		if(waitKey(30) >= 0) break;
	}

	//ovrHmd_Destroy(hmd);
	//ovr_Shutdown();

	return 0;
}