#pragma once
#include <iostream>
#include <conio.h>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif 

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <string>


enum Command {FORWARDS, BACKWARDS, LEFT, RIGHT, STOP, 
			  EXIT, CHANGE_SPEED, POLL_SPEED, POLL_SONAR, POLL_TELEMETRY, NONE};
enum ConnectionStatus {FREE, CONNECTING, ACTIVE,INACTIVE};
class ConnectionError{};

template <typename T>
struct SharedData{
public:
	T data;
	int myInt;
};

struct ArgStruct{
	int *argc;
	char **argv;
};

struct MySockType{
	SOCKET clientSock;
	SOCKET servSock;
	int index;
};


template <typename T>
class MyBuffer{
public:	
	MyBuffer(int ms=100);
	~MyBuffer();
	void initialize(int ms=100);
	void push(T d);
	bool read;
	int pos;
	int size;
	T *data;
private:
	void kill();
	int maxSize;
};


class MyRobotTelemetry{
public:
	double voltage;
	double rotVel;
	double vel;
	double odometerDistance;
	double odometerTime;
	double frontSonar;
	double backSonar;
	bool operator==(MyRobotTelemetry &mtr){
		return (
			this->rotVel == mtr.rotVel &&
			this->odometerDistance == mtr.odometerDistance &&
			this->odometerTime == mtr.odometerTime &&
			this->vel == mtr.vel &&
			this->voltage == mtr.voltage &&
			this->frontSonar == mtr.frontSonar &&
			this->backSonar == mtr.backSonar);
	};
};
