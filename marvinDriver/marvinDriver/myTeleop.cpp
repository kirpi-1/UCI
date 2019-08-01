#include "stdafx.h"
#include "myTeleop.h"
#include "ArRobot.h"
#include "ArActionLimiterTableSensor.h"
#include "ArActionLimiterForwards.h"
#include "ArActionLimiterBackwards.h" 
#include "myKeydrive.h"
#include "ArActionKeydrive.h"


MyTeleop::MyTeleop(ArRobot *robot, int priority, int velInc):ArActionGroup(robot){
	//add limiters to prevent the robot from smashing into things
	//addAction(instance of functor(ArAction), priority(0 to 100, 100 is highest priority)
	/**/
	//addAction(new ArActionLimiterTableSensor, 100);	
	//addAction(new ArActionLimiterForwards("Speed Limiter Near", 100, 300, 250), 90);
	//addAction(new ArActionLimiterForwards("Speed Limiter Far", 300, 1100, 400), 89);
	//addAction(new ArActionLimiterBackwards, 80);	
	myInput = new ArActionRatioInput;
	addAction(myInput, 50);
	myKD = new myKeydrive(robot, myInput, priority, velInc, 50);
}



MyTeleop::~MyTeleop(){
	removeActions();
	deleteActions();
}