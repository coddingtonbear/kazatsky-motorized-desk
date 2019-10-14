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

float getHeight();
void toHeight(float);
int getPosition();
void toPosition(int);
void handleMotorPosition();
void storePosition();

void setup();
void loop();
void _loop();
