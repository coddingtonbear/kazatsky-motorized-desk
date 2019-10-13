#include <Arduino.h>
#include <EEPROM.h>
#include <BTS7960.h>
#include <SerialCommand.h>

#include "main.h"

#define BUTTON_UP 3
#define BUTTON_DOWN 4

#define MOT_L_EN 7
#define MOT_L_PWM 5
#define MOT_R_EN 2
#define MOT_R_PWM 6

#define MOT_PWR_VOLTAGE A5
#define MOT_HALL A1

#define EEPROM_POSITION 0

BTS7960 motor(MOT_R_PWM, MOT_L_PWM, MOT_L_EN, MOT_R_EN);

#define SPEED 255

SerialCommand cmd;

int lastHallState = HIGH;
bool up = true;
int16_t position = 0;

bool isMoving = false;
bool wasMoving = false;

unsigned long lastButtonCheck = 0;

void handleMotorPosition() {
	int currentHallState = digitalRead(MOT_HALL);

	if(lastHallState && currentHallState == LOW) {
		if(up) {
			position++;
		} else {
			position--;
		}
	}

	lastHallState = currentHallState;
}

void cmdUnrecognized(const char*) {
	Serial.println("Unrecognized command");
}

void cmdGetPosition() {
	Serial.println(position);
}

void cmdToPosition() {
	int target;

	char* targetStr = cmd.next();
	if(targetStr) {
		target = atoi(targetStr);
		toPosition(target);
	}
}

void toPosition(int target) {
	unsigned long timeout = millis() + 10000;

	while(position != target && millis() < timeout) {
		if(position > target) {
			motorDown();
		} else {
			motorUp();
		}

		_loop();
	}
}

void storePosition() {
	EEPROM.write(EEPROM_POSITION, position);
}

void setup() {
	pinMode(BUTTON_UP, INPUT_PULLUP);
	pinMode(BUTTON_DOWN, INPUT_PULLUP);
	pinMode(MOT_HALL, INPUT);

	position = EEPROM.read(EEPROM_POSITION);

	cmd.addCommand("to_position", cmdToPosition);
	cmd.addCommand("get_position", cmdGetPosition);
	cmd.setDefaultHandler(cmdUnrecognized);
	
	motor.stop();
	motor.disable();

	Serial.begin(9600);
}

void motorUp() {
	motor.enable();
	motor.setSpeed(SPEED);
	isMoving = true;
	up = true;
}

void motorDown() {
	motor.enable();
	motor.setSpeed(-1 * SPEED);
	isMoving = true;
	up = false;
}

void motorStop() {
	isMoving = false;
	motor.stop();
}

void _loop() {
	if(millis() - lastButtonCheck > 100) {
		wasMoving = isMoving;
		lastButtonCheck = millis();
		if(digitalRead(BUTTON_UP) == LOW && digitalRead(BUTTON_DOWN) == LOW) {
			position = 0;
			isMoving = false;
		} else if (digitalRead(BUTTON_UP) == LOW) {
			motorUp();
		} else if (digitalRead(BUTTON_DOWN) == LOW) {
			motorDown();
		} else {
			motorStop();
		}
		if(wasMoving && !isMoving) {
			storePosition();
		}
	}

	handleMotorPosition();
}

void loop() {
	cmd.readSerial();

	_loop();
}
