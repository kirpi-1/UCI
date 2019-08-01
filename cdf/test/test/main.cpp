#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include "..\\..\\cdf.h"
#include "..\\..\\zANT.h"

#define RAW (0)
#define AMP (1)
#define FREQ (2)
typedef unsigned int uint;
using namespace std;
using namespace cdf;
const int NUMELECTRODES = 64;

void readCOIActionNames(char ***coiNames, char ***actionNames){
	*coiNames = new char*[2];
	*actionNames = new char*[2];
	char *memblock;
	for(int i=0;i<2;i++){
		memblock = "poop";
		(*coiNames)[i] = memblock;
	}
	for(int i=0;i<2;i++){
		memblock = "on you!";
		(*actionNames)[i] = memblock;
	}
	//delete[] memblock;
}
int myMain(){
	{	
	CDFSession myWriteSession;
	//open new file, overwrite if it exists
	int res = myWriteSession.open("test.cdf");
	if(res!=0)
		cout << "open failed\n";		
	myWriteSession.setSubNum(26);	
	string vheader = "this is my variable header\n";
	myWriteSession.setVarHeader(vheader.c_str(),vheader.size());

	if(myWriteSession.writeHeader()!= 0)
		cout << "failed to write header\n";
	
	Frame fi,fi2,fi3;
	int x = 3;
	int y = 4; 
	float *data = new float[x*y];
	for(uint r=0;r<y;r++){
		for(uint c=0;c<x;c++){
			data[r*x+c]=(float)1*c+10*r;
			printf("%f ",data[r*x+c]);
		}
		printf("\n");
	}
	char frameHeader[64] = "this is my frame footer\0";
	
	myWriteSession.setFrameData(data,x,y);
	myWriteSession.setFrameFooter(frameHeader,64);
	
	delete[] data;
	
	myWriteSession.writeFrame();
	myWriteSession.writeFrame();
	myWriteSession.writeFrame();
	myWriteSession.writeFrame();
	myWriteSession.close();


	/*
	if(res!=0)
		cout << "writeFrame failed\n";
	res = myWriteSession.writeFrame(&fi2);
	if(res!=0)
		cout << "writeFrame failed 2\n";
	res = myWriteSession.writeFrame(&fi3);
	if(res!=0)
		cout << "writeFrame failed 3\n";	
	
	
	//cout << myWriteSession.asciiHeader;
	//myWriteSession.close();
	//myWriteSession.open("test.cdf");
	}
	
	{
	//reading a session
	CDFSession mySession;
	mySession.open("test.cdf");

	mySession.printAsciiHeader();
	mySession.printCOI();
	mySession.printFreqs();
	mySession.printActions();
	mySession.printFrame(0);	
	mySession.printFrame(1);
	
	//printf("%s",mySession.asciiHeader);
	//printCOI(mySession.session,mySession.vh.cois,mySession.vh.coiNames);
	*/
	}
	
	return 0;
}
int main(){	
	int res = myMain();
	_CrtDumpMemoryLeaks();
	system("PAUSE");
	return 0;	
}