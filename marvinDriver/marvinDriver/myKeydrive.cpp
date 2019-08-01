#include "stdafx.h"
#include "ariaOSDef.h"
#include "myKeydrive.h"
#include "ArRobot.h"
#include "ariaInternal.h"
#include "ArKeyHandler.h"


myKeydrive::myKeydrive(ArRobot *robot, 
						    ArActionRatioInput *input,
						    int priority,
						    double velIncrement,
							unsigned int maxThrottle) :
  myUpCB(this, &myKeydrive::up),
  myDownCB(this, &myKeydrive::down),
  myLeftCB(this, &myKeydrive::left),
  myRightCB(this, &myKeydrive::right),
  myZCB(this, &myKeydrive::z),
  myXCB(this, &myKeydrive::x),
  mySpaceCB(this, &myKeydrive::space),
  myFireCB(this, &myKeydrive::fireCallback),
  myActivateCB(this, &myKeydrive::activate),
  myDeactivateCB(this, &myKeydrive::deactivate)
{
  myRobot = robot;
  myInput = input;
  myInput->addFireCallback(priority, &myFireCB);
  myInput->addActivateCallback(&myActivateCB);
  myInput->addDeactivateCallback(&myDeactivateCB);
  myFireCB.setName("Keydrive");
  myVelIncrement = velIncrement;
  myLatVelIncrement = velIncrement;
  myTurnIncrement = velIncrement;
  myHaveKeys = false;
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  myMaxThrottle = maxThrottle;
  myThrottle = myMaxThrottle;
  myPrinting = false;
}

myKeydrive::~myKeydrive()
{
  myInput->remFireCallback(&myFireCB);
  myInput->remActivateCallback(&myActivateCB);
}


void myKeydrive::takeKeys(void)
{
  myHaveKeys = true;
  ArKeyHandler *keyHandler;
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "myKeydrive::takeKeys: There is no key handler, keydrive will not work.");
  }
   
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->addKeyHandler(ArKeyHandler::UP, &myUpCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for up, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::DOWN, &myDownCB))
    ArLog::log(ArLog::Terse, "The key handler already has a key for down, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::LEFT, &myLeftCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for left, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::RIGHT, &myRightCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for right, keydrive will not work correctly.");
  if (!keyHandler->addKeyHandler(ArKeyHandler::SPACE, &mySpaceCB))
    ArLog::log(ArLog::Terse,  
	       "The key handler already has a key for space, keydrive will not work correctly.");
  if (myRobot != NULL && myRobot->hasLatVel())
  {
    if (!keyHandler->addKeyHandler('z', &myZCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for z, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('Z', &myZCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for Z, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('x', &myXCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for x, keydrive will not work correctly.");
    if (!keyHandler->addKeyHandler('X', &myXCB))
      ArLog::log(ArLog::Terse,  
		 "The key handler already has a key for x, keydrive will not work correctly.");
  }
  
}

void myKeydrive::giveUpKeys(void)
{
  ArKeyHandler *keyHandler;
  myHaveKeys = false;
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    ArLog::log(ArLog::Terse, 
	       "myKeydrive::giveUpKeys: There is no key handler, something is probably horribly wrong .");
  }
  
  // now that we have one, add our keys as callbacks, print out big
  // warning messages if they fail
  if (!keyHandler->remKeyHandler(&myUpCB))
    ArLog::log(ArLog::Terse, "myKeydrive: The key handler already didn't have a key for up, something is wrong.");
  if (!keyHandler->remKeyHandler(&myDownCB))
    ArLog::log(ArLog::Terse, "myKeydrive: The key handler already didn't have a key for down, something is wrong.");
  if (!keyHandler->remKeyHandler(&myLeftCB))
    ArLog::log(ArLog::Terse,  
	       "myKeydrive: The key handler already didn't have a key for left, something is wrong.");
  if (!keyHandler->remKeyHandler(&myRightCB))
    ArLog::log(ArLog::Terse,  
	       "myKeydrive: The key handler already didn't have a key for right, something is wrong.");
  if (!keyHandler->remKeyHandler(&mySpaceCB))
    ArLog::log(ArLog::Terse,  
	       "myKeydrive: The key handler didn't have a key for space, something is wrong.");
  if (myRobot != NULL && myRobot->hasLatVel())
  {
    if (!keyHandler->remKeyHandler(&myZCB))
      ArLog::log(ArLog::Terse,  
		 "myKeydrive: The key handler didn't have a key for z, something is wrong.");
    if (!keyHandler->remKeyHandler(&myXCB))
      ArLog::log(ArLog::Terse,  
		 "myKeydrive: The key handler didn't have a key for x, something is wrong.");
  }
  

}


void myKeydrive::up(void)
{
  if (myPrinting)
    printf("up\n");
  myTransRatio += myVelIncrement;
  if (myTransRatio > 100)
    myTransRatio = 100;
}

void myKeydrive::down(void)
{
  if (myPrinting)
    printf("down\n");
  myTransRatio -= myVelIncrement;
  if (myTransRatio < -100)
    myTransRatio = -100;
}

void myKeydrive::left(void)
{
  if (myPrinting)
    printf("left\n");
  myRotRatio +=myTurnIncrement;
  if(myRotRatio>100)
	  myRotRatio=100;
}

void myKeydrive::right(void)
{
  if (myPrinting)
    printf("right\n");
  myRotRatio -= myTurnIncrement;
  if(myRotRatio<-100)
	  myRotRatio=-100;
}

void myKeydrive::space(void)
{
  if (myPrinting)
    printf("stop\n");
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
}

void myKeydrive::z(void)
{
  if (myPrinting)
    printf("up\n");
  myLatRatio += myLatVelIncrement;
  if (myLatRatio > 100)
    myLatRatio = 100;
}

void myKeydrive::x(void)
{
  if (myPrinting)
    printf("down\n");
  myLatRatio -= myLatVelIncrement;
  if (myLatRatio < -100)
    myLatRatio = -100;
}

void myKeydrive::setThrottle(double t){
	myThrottle = t;
	if(myThrottle > myMaxThrottle)
		myThrottle = myMaxThrottle;
	if(myThrottle<0)
		myThrottle=0;
	if(myThrottle>100)
		myThrottle=100;
}

void myKeydrive::setMaxThrottle(double t){
	myMaxThrottle = t;
	if(myMaxThrottle<0)
		myMaxThrottle=0;
	if(myMaxThrottle>100)
		myMaxThrottle=100;
	if(myThrottle > myMaxThrottle)
		myThrottle = myMaxThrottle;
}

void myKeydrive::activate(void)
{
  // set things so we'll stop
  myTransRatio = 0;
  myRotRatio = 0;
  myLatRatio = 0;
  if (myHaveKeys)
    takeKeys();
}

void myKeydrive::deactivate(void)
{
  if (myHaveKeys)
    giveUpKeys();
}

void myKeydrive::fireCallback(void)
{
  // set what we want to do
  myInput->setTransRatio(myTransRatio);
  myInput->setRotRatio(myRotRatio);
  myInput->setLatRatio(myLatRatio);
  myInput->setThrottleRatio(myThrottle);
  if(myPrinting){
	if(myTransRatio!=0)
		printf("myTransRatio: %f\n",myTransRatio);
	if(myRotRatio!=0)
		printf("myRotRatio: %f\n",myRotRatio);
	if(myLatRatio!=0)
		printf("myLatRatio: %f\n",myLatRatio);
	if(myThrottle!=0)
		printf("myThrottle: %f\n",myThrottle);
  }

  // reset us to going straight (if they're holding the key we'll keep turning)
  
  //recenter to 0 just in case rounding errors creep in
  if(myRotRatio<myTurnIncrement && myRotRatio>-1*myTurnIncrement)
	  myRotRatio=0;
  if(myRotRatio<0)
	  myRotRatio+=myTurnIncrement;
  else if(myRotRatio>0)
	  myRotRatio-=myTurnIncrement;
  //myRotRatio = 0;

  if (myHaveKeys)
    return;
  ArKeyHandler *keyHandler;
   
  // see if there is already a keyhandler, if not make one for ourselves
  if ((keyHandler = Aria::getKeyHandler()) == NULL)
  {
    keyHandler = new ArKeyHandler;
    Aria::setKeyHandler(keyHandler);
    myRobot->attachKeyHandler(keyHandler);
  }
  takeKeys();
}
