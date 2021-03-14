#include <Arduino.h>
#include <BTS7960.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <SerialCommand.h>

#include "main.h"

#define BASELINE_HEIGHT 72
#define POSITION_TO_MM 0.45
#define POSITION_MAX 95

#define BUTTON_UP 3
#define BUTTON_DOWN 4
#define BUTTON_DEBOUNCE_MS 25

#define MOT_L_EN 23
#define MOT_L_PWM 5
#define MOT_R_EN 9
#define MOT_R_PWM 6

//#define MOT_PWR_VOLTAGE A5
//#define ENABLE_HALL
#define MOT_HALL 14
#define MOT_HALL_DEBOUNCE_MS 25
#define MOT_HALL_DEBOUNCE_STALL_TIMEOUT 250

#define LIMIT_MIN 15

#define EEPROM_STORE_MAGIC 24
#define EEPROM_STORE_SIGNAL 99
#define EEPROM_POSITION 100

BTS7960 motor(MOT_R_PWM, MOT_L_PWM, MOT_L_EN, MOT_R_EN);

#define SPEED_SETTLE_RAMP 500
#define SPEED_RAMP 1000
#define SPEED 175

#define TICK_ON 100
#define TICK_INTERVAL 25
#define TICK_SPEED 1

SerialCommand cmd;

int lastHallState = HIGH;
bool up = true;
uint8_t position = 0;
uint8_t lastPosition = 0;

bool motorDisabled = false;
unsigned long stoppingStarted = 0;
unsigned long movementStarted = 0;
bool isMoving = false;
bool wasMoving = false;
bool buttonsLocked = false;

unsigned long lastButtonCheck = 0;
unsigned long stallTimeoutTimestamp = 0;

Bounce buttonUp = Bounce();
Bounce buttonDown = Bounce();
Bounce hall = Bounce();

bool motorIsStalled()
{
    if (isMoving && millis() > stallTimeoutTimestamp) {
        return true;
    }

    return false;
}

void refreshStallTimeout()
{
    stallTimeoutTimestamp = millis() + MOT_HALL_DEBOUNCE_STALL_TIMEOUT;
}

void handleMotorPosition()
{
    int limitTripped = digitalRead(LIMIT_MIN);

    if (limitTripped == LOW && !up && isMoving) {
        resetPosition();
        motorStop();
        buttonLockout();
        tick(3);
    } else {
        buttonLockout(false);
    }

    #ifdef ENABLE_HALL
        int currentHallState = hall.read();
        lastPosition = position;

        if (lastHallState == HIGH && currentHallState == LOW) {
            if (up) {
                position++;
            } else {
                position--;
            }
        }

        if (lastHallState != currentHallState) {
            lastHallState = currentHallState;
            refreshStallTimeout();
        }

        if (motorIsStalled()) {
            resetPosition();
            motorStop();
            buttonLockout();
        }
    #endif
}

bool positionHasChanged()
{
    return lastPosition != position;
}

void cmdUnrecognized(const char*)
{
    Serial.println("Unrecognized command");
}

void cmdGetPosition()
{
    Serial.println(getPosition());
}

void cmdGetHeight()
{
    Serial.println(getHeight());
}

void cmdToPosition()
{
    int target;

    char* targetStr = cmd.next();
    if (targetStr) {
        target = atoi(targetStr);
        toPosition(target);
    }
}

void tick(uint8_t count)
{
    motor.setSpeed(TICK_SPEED);


    for(uint8_t i = 0; i<count; i++) {
        motor.enable();
        delay(TICK_ON);
        motor.disable();
        delay(TICK_INTERVAL);
    }
}

void cmdToHeight()
{
    int target;

    char* targetStr = cmd.next();
    if (targetStr) {
        target = atoi(targetStr);
        toHeight(target);
    }
}

void resetPosition()
{
    position = 0;
    storePosition();
}

void cmdResetPosition()
{
    resetPosition();
}

void toPosition(int target)
{
    unsigned long timeout = millis() + 10000;

    if (target < 0 || target > POSITION_MAX) {
        return;
    }

    while (position != target && millis() < timeout) {
        if (motorIsStalled()) {
            return;
        }

        if (position > target) {
            motorDown();
        } else {
            motorUp();
        }

        _loop();
    }
}

float getHeight()
{
    return BASELINE_HEIGHT + (POSITION_TO_MM * getPosition());
}

int getPosition()
{
    return position;
}

void toHeight(float mm)
{
    int targetPosition = (mm - BASELINE_HEIGHT) / POSITION_TO_MM;
    toPosition(targetPosition);
}

void storePosition()
{
    #ifdef ENABLE_HALL
    EEPROM.update(EEPROM_STORE_SIGNAL, EEPROM_STORE_MAGIC);
    EEPROM.update(EEPROM_POSITION, position);
    #endif
}

void setup()
{
    pinMode(LIMIT_MIN, INPUT_PULLUP);

    buttonUp.attach(BUTTON_UP, INPUT_PULLUP);
    buttonDown.attach(BUTTON_DOWN, INPUT_PULLUP);
    hall.attach(MOT_HALL, INPUT);

    cmd.addCommand("to_height", cmdToHeight);
    cmd.addCommand("get_height", cmdGetHeight);
    cmd.addCommand("to_position", cmdToPosition);
    cmd.addCommand("get_position", cmdGetPosition);
    cmd.addCommand("reset_position", cmdResetPosition);
    cmd.setDefaultHandler(cmdUnrecognized);

    motor.stop();
    motor.disable();

    Serial.begin(9600);

    Serial.println("<Ready>");

    if (EEPROM.read(EEPROM_STORE_SIGNAL) == EEPROM_STORE_MAGIC) {
        position = EEPROM.read(EEPROM_POSITION);
    } else {
        position = 0;
    }

    // Safety machinery
    if (digitalRead(BUTTON_UP) == LOW && digitalRead(BUTTON_DOWN) == LOW) {
        motorDisabled = true;
        tick(10);
    } else {
        tick(3);
    }
}

uint8_t getRampedSpeed() {
    return float(SPEED) * min(1, float(millis() - movementStarted) / float(SPEED_RAMP));
}

uint8_t getRampedSettlingSpeed() {
    return float(getRampedSpeed()) * (1 - min(1, float(millis() - stoppingStarted) / float(SPEED_SETTLE_RAMP)));
}

void motorUp()
{
    if(motorDisabled) {
        return;
    }
    if (!isMoving) {
        refreshStallTimeout();
    }
    if (movementStarted == 0) {
        movementStarted = millis();
    }

    motor.enable();
    motor.setSpeed(getRampedSpeed());
    isMoving = true;
    up = true;
}

void motorDown()
{
    if(motorDisabled) {
        return;
    }
    if (!isMoving) {
        refreshStallTimeout();
    }
    if (movementStarted == 0) {
        movementStarted = millis();
    }

    motor.enable();
    motor.setSpeed(-1 * getRampedSpeed());
    isMoving = true;
    up = false;
}

void motorSettle() {
    if(motorDisabled || !isMoving) {
        motorStop();
        return;
    }
    if (stoppingStarted == 0) {
        stoppingStarted = millis();
    }

    int speed = getRampedSettlingSpeed() * (up ? 1 : -1);
    if (speed == 0) {
        stoppingStarted = 0;
        motorStop();
    } else {
        motor.enable();
        motor.setSpeed(speed);
    }
}

void motorStop()
{
    movementStarted = 0;
    isMoving = false;
    motor.stop();
}

void buttonLockout(bool locked)
{
    buttonsLocked = locked;
}

void _loop()
{
    buttonUp.update();
    buttonDown.update();
    #ifdef ENABLE_HALL
        hall.update();
    #endif

    if (millis() - lastButtonCheck > 100) {

        int buttonUpState = buttonUp.read();
        int buttonDownState = buttonDown.read();

        wasMoving = isMoving;
        lastButtonCheck = millis();
        if (buttonUpState == LOW && buttonDownState == LOW) {
            motorStop();
            resetPosition();
        } else if (positionHasChanged() && getPosition() == 0) {
            buttonLockout();
            motorStop();
        } else if (buttonUpState == LOW) {
            motorUp();
        } else if (!buttonsLocked && buttonDownState == LOW) {
            motorDown();
        } else {
            motorSettle();
        }
        if(isMoving) {
            if(stoppingStarted) {
                Serial.print("<Stopping: speed=");
                Serial.print(getRampedSettlingSpeed());
                Serial.print(" ");
                Serial.print("direction=");
                Serial.print(up ? "up" : "down");
                Serial.println(">");
            } else {
                Serial.print("<Moving: speed=");
                Serial.print(getRampedSpeed());
                Serial.print(" ");
                Serial.print("direction=");
                Serial.print(up ? "up" : "down");
                Serial.println(">");
            }
        }
        if (wasMoving && !isMoving) {
            storePosition();
        }
    }

    handleMotorPosition();
}

void loop()
{
    cmd.readSerial();

    _loop();
}
