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

enum {FORWARDS, BACKWARDS, LEFT, RIGHT};

DWORD sleepTime=100;
const unsigned int BUFFSIZE = 128;
char *DEFAULT_SERV_NAME = "192.168.0.199";
char *DEFAULT_PORT = "3817";