void cmdUnrecognized(const char*);
void cmdGetPosition();
void cmdToPosition();
void cmdToHeight();
void cmdGetHeight();
void cmdResetPosition();
void cmdGetVoltage();

void motorUp();
void motorDown();
void motorStop();
bool motorIsStalled();
void refreshStallTimeout();
void buttonLockout(bool lockout = true);

float getHeight();
void toHeight(float);
int getPosition();
void toPosition(int);
void handleMotorPosition();
void resetPosition();
void storePosition();

void setup();
void loop();
void _loop();
