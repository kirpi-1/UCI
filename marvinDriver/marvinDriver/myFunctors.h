#ifndef MYFUNCTORS_H
#define MYFUNCTORS_H

#include "Aria.h"
#include "ArFunctor.h"
#include <iostream>
#include <string>
#include "stdafx.h"


class IncreaseSpeed:public ArFunctor {
public:
	IncreaseSpeed(unsigned int x,  ArRobot *r);
	void invoke();
private:
	unsigned int increaseValue;
	ArRobot *robot;
};
/*
class DecreaseSpeed:public ArFunctor {
public:
	DecreaseSpeed(unsigned int x);
	void operator()(ArRobot *robot);
private:
	unsigned int decreaseValue;
};
*/
class ChangeTransVel:public ArFunctor{
public:
	ChangeTransVel(double v, ArRobot *r);
	void invoke();
private:
	double velocity;
	ArRobot *robot;
};

class ChangeRotVel:public ArFunctor{
public:
	ChangeRotVel(double v, ArRobot *r);
	void invoke();
private:
	double velocity;
	ArRobot *robot;
};

class StopRobot:public ArFunctor{
public:
	StopRobot(ArRobot *r){robot=r;};
	void invoke();
private:
	ArRobot *robot;
};

class StopRotation:public ArFunctor{
public:
	StopRotation(ArRobot *r){robot=r;};
	void invoke(){robot->setRotVel(0);};
private:
	ArRobot *robot;
};

class Decelerate:public ArFunctor{
public:
	Decelerate(double a, ArRobot *r);
private:
	ArRobot *robot;
	double acceleration;
	void invoke();
};

//template <typename T>
class PollRobot:public ArFunctor{
public:
	PollRobot(std::string s, ArRobot *r, /*T*/double (ArRobot::*m)(void) const);
	//~PollRobot();
private:
	ArRobot *robot;
	/*T*/double (ArRobot::*member)(void) const;
	std::string message;
	void invoke();
};

template <typename Ret, typename T, typename P1>
class PollPrint:public ArConstRetFunctor1C<Ret, T, P1>{
public:
	//ctors
	PollPrint(std::string s, T *o, Ret (T::*f)(P1) const, P1 p):myString(s),myObj(o),myFunc(f), myP1(p) {}
	PollPrint(std::string s, T &o, Ret (T::*f)(P1) const, P1 p):myString(s),myObj(&o),myFunc(f), myP1(p) {}
	void invoke(void) {std::cout << myString << (myObj->*myFunc)(myP1) << std::endl;}

private:
	std::string myString;
	P1 myP1;
	T *myObj;
	Ret (T::*myFunc)(P1) const;

};


#endif //MYFUNCTORS_H