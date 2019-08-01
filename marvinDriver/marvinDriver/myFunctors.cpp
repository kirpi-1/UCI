#include "stdafx.h"
#include "myFunctors.h"

IncreaseSpeed::IncreaseSpeed(unsigned int x, ArRobot *r){
	increaseValue=x;
	robot = r;
}

void IncreaseSpeed::invoke(){
	robot->setVel(robot->getVel()+increaseValue);
}

/*
DecreaseSpeed::DecreaseSpeed(unsigned int x){
	decreaseValue=x;
}

void DecreaseSpeed::operator()(ArRobot *robot){
	robot->setVel(robot->getVel()+decreaseValue);
}
*/

ChangeTransVel::ChangeTransVel(double v, ArRobot *r){
	velocity = v;
	robot = r;
}

void ChangeTransVel::invoke(){
	//robot->setTransAccel(
	robot->setVel(robot->getVel()+velocity);
}

void StopRobot::invoke(){
	printf("stopping robot\n");
/*	int i;
	if(robot->getVel()<0)
		i=1;
	else
		i=-1;
	while(!(robot->getVel()<50 && robot->getVel()>-50)){
		robot->setVel(robot->getVel()+50*i);
*/


//	}
	robot->setVel(0);
	robot->setRotVel(0);
}

ChangeRotVel::ChangeRotVel(double v, ArRobot *r){
	velocity = v;
	robot = r;
}

void ChangeRotVel::invoke(){
	robot->setRotVel(robot->getRotVel()+velocity);
}

//template <typename T>
PollRobot/*<T>*/::PollRobot(std::string s,ArRobot *r, /*T*/double (ArRobot::*m)(void) const){
	robot = r;
	member = m;
	message = s;
}

/*
template <typename T>
PollRobot/*<T>::~PollRobot(){
	free(message);
}
*/

Decelerate::Decelerate(double a, ArRobot *r){
	robot = r;
	acceleration = a;
}

void Decelerate::invoke(){
	std::cout << "decelerating by " << acceleration << std::endl;
	robot->setTransDecel(acceleration);
}

//template <typename T>
void PollRobot/*<T>*/::invoke(){
	std::cout << message << ((*robot).*member)() << std::endl;
}
