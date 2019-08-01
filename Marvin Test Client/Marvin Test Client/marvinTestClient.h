#include "Aria.h"

class MyRobotTelemetry{
public:
	double voltage;
	double rotVel;
	double vel;
	double odometerDistance;
	double odometerTime;
	double frontSonar;
	double backSonar;
	double x;
	double y;
	double t;
	bool operator==(MyRobotTelemetry &mtr){
		return (
			this->rotVel == mtr.rotVel &&
			this->odometerDistance == mtr.odometerDistance &&
			this->odometerTime == mtr.odometerTime &&
			this->vel == mtr.vel &&
			this->voltage == mtr.voltage &&
			this->frontSonar == mtr.frontSonar &&
			this->backSonar == mtr.backSonar &&
			this->x == mtr.x &&	
			this->y == mtr.y &&
			this->t == mtr.t			
		);
	};
	void getRobotTelemetry(ArRobot &robot, double range);
};

void MyRobotTelemetry::getRobotTelemetry(ArRobot &robot, double range){
	voltage = robot.getBatteryVoltage();
	rotVel = robot.getRotVel();
	vel = robot.getVel();
	odometerDistance = robot.getOdometerDistance();
	odometerTime = robot.getOdometerTime();
	frontSonar = robot.getClosestSonarRange(0-range/2.0,0+range/2.0);
	backSonar = robot.getClosestSonarRange(180-range/2.0,180+range/2.0);
	x = robot.getX();
	y = robot.getY();
	t = robot.getTh();
}

enum {FORWARDS, BACKWARDS, LEFT, RIGHT};

DWORD sleepTime=100;
const unsigned int BUFFSIZE = 128;
char *DEFAULT_SERV_NAME = "192.168.0.199";
char *DEFAULT_PORT = "3817";