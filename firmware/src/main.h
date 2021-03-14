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
void motorSettle();
void buttonLockout(bool lockout = true);

float getHeight();
void toHeight(float);
bool positionHasChanged();
int getPosition();
void toPosition(int);
void handleMotorPosition();
void resetPosition();
void storePosition();

uint8_t getRampedSpeed();
uint8_t getRampedSettlingSpeed();

void tick(uint8_t count = 1);

void setup();
void loop();
void _loop();
