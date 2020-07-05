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

#define MOT_L_EN 7
#define MOT_L_PWM 5
#define MOT_R_EN 2
#define MOT_R_PWM 6

#define MOT_PWR_VOLTAGE A5
#define MOT_HALL A1
#define MOT_HALL_DEBOUNCE_MS 25
#define MOT_HALL_DEBOUNCE_STALL_TIMEOUT 250

#define EEPROM_STORE_MAGIC 24
#define EEPROM_STORE_SIGNAL 99
#define EEPROM_POSITION 100

BTS7960 motor(MOT_R_PWM, MOT_L_PWM, MOT_L_EN, MOT_R_EN);

#define SPEED 255

SerialCommand cmd;

int lastHallState = HIGH;
bool up = true;
uint8_t position = 0;

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
    int currentHallState = hall.read();

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

void cmdGetVoltage()
{
    uint8_t reading = analogRead(MOT_PWR_VOLTAGE);
    double voltage = (reading / 1024.0) * 20;

    Serial.println(voltage);
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
    EEPROM.update(EEPROM_STORE_SIGNAL, EEPROM_STORE_MAGIC);
    EEPROM.update(EEPROM_POSITION, position);
}

void setup()
{
    buttonUp.attach(BUTTON_UP, INPUT_PULLUP);
    buttonDown.attach(BUTTON_DOWN, INPUT_PULLUP);
    hall.attach(MOT_HALL, INPUT);

    cmd.addCommand("to_height", cmdToHeight);
    cmd.addCommand("get_height", cmdGetHeight);
    cmd.addCommand("to_position", cmdToPosition);
    cmd.addCommand("get_position", cmdGetPosition);
    cmd.addCommand("reset_position", cmdResetPosition);
    cmd.addCommand("get_voltage", cmdGetVoltage);
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
}

void motorUp()
{
    if (!isMoving) {
        refreshStallTimeout();
    }

    motor.enable();
    motor.setSpeed(SPEED);
    isMoving = true;
    up = true;
}

void motorDown()
{
    if (!isMoving) {
        refreshStallTimeout();
    }

    motor.enable();
    motor.setSpeed(-1 * SPEED);
    isMoving = true;
    up = false;
}

void motorStop()
{
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
    hall.update();

    if (millis() - lastButtonCheck > 100) {
        int buttonUpState = buttonUp.read();
        int buttonDownState = buttonDown.read();

        if ((buttonUpState == HIGH && buttonDownState == HIGH) && buttonsLocked) {
            buttonsLocked = false;
        }

        wasMoving = isMoving;
        lastButtonCheck = millis();
        if (!buttonsLocked) {
            if (buttonUpState == LOW && buttonDownState == LOW) {
                motorStop();
                resetPosition();
            } else if (buttonUpState == LOW) {
                motorUp();
            } else if (buttonDownState == LOW) {
                motorDown();
            } else {
                motorStop();
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
