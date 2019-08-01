#ifndef MYTELEOP_H
#define	MYTELEOP_H

#include "ariaTypedefs.h"
#include "ArActionGroups.h"
#include "myKeydrive.h"
#include "ArActionRatioInput.h"
#include "stdafx.h"


class MyTeleop:public ArActionGroup{
private:
	ArActionRatioInput *myInput;
	myKeydrive *myKD;
public:
	//MyTeleop(ArRobot *robot);
	MyTeleop(ArRobot *robot, int priority = 25, int velInc = 25);
	virtual ~MyTeleop();
	void initialize(ArRobot *robot, int priority = 25, int velInc = 25);
	void up(){myKD->up();};
	void down(){myKD->down();};
	void left(){myKD->left();};
	void right(){myKD->right();};
	void space(){myKD->space();};
	void setThrottle(unsigned int t){myKD->setThrottle(t);};
	void setMaxThrottle(unsigned int t){myKD->setMaxThrottle(t);};
	unsigned int getThrottle(){return myKD->getThrottle();};
	unsigned int getMaxThrottle(){return myKD->getMaxThrottle();};

};


#endif //MYTELEOP_H