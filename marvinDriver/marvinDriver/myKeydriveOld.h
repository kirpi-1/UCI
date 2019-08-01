#ifndef MYKEYDRIVE_H
#define MYKEYDRIVE_H

#include "stdafx.h"
#include "ArAction.h"
#include "ArFunctor.h"
#include "ArRobot.h"
#include "ArActionRatioInput.h"


/// This will use the keyboard arrow keys and the ArActionRatioInput to drive the robot
/**
   You have to make an ArActionRatioInput and add it to the robot like
   a normal action for this to work.
**/
class myKeydrive 
{
public:
  /// Constructor
  myKeydrive(ArRobot *robot, ArActionRatioInput *input, 
				int priority = 25, double velIncrement = 5);
  /// Destructor
  virtual ~myKeydrive();
  /// Takes the keys this action wants to use to drive
  void takeKeys(void);
  /// Gives up the keys this action wants to use to drive
  void giveUpKeys(void);
  /// Internal, callback for up arrow
  void up(void);
  /// Internal, callback for down arrow
  void down(void);
  /// Internal, callback for z
  void z(void);
  /// Internal, callback for x 
  void x(void);
  /// Internal, callback for left arrow
  void left(void);
  /// Internal, callback for right arrow
  void right(void);
  /// Internal, callback for space key
  void space(void);
  /// Internal, gets our firecb
  ArFunctor *getFireCB(void) { return &myFireCB; }
protected:
  void activate(void);
  void deactivate(void);
  void fireCallback(void);
  ArFunctorC<myKeydrive> myUpCB;
  ArFunctorC<myKeydrive> myDownCB;
  ArFunctorC<myKeydrive> myLeftCB;
  ArFunctorC<myKeydrive> myRightCB;
  ArFunctorC<myKeydrive> myZCB;
  ArFunctorC<myKeydrive> myXCB;
  ArFunctorC<myKeydrive> mySpaceCB;

  double myPrinting;
  double myTransRatio;
  double myRotRatio;
  double myThrottle;
  double myLatRatio;

  ArRobot *myRobot;
  bool myHaveKeys;
  double myVelIncrement;
  double myLatVelIncrement;
  ArActionRatioInput *myInput;
  ArFunctorC<myKeydrive> myFireCB;
  ArFunctorC<myKeydrive> myActivateCB;
  ArFunctorC<myKeydrive> myDeactivateCB;
};

#endif //MYKEYDRIVE_H