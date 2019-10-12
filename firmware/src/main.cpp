#include <Arduino.h>
#include <EEPROM.h>
#include <BTS7960.h>

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

void setup() {
	pinMode(BUTTON_UP, INPUT_PULLUP);
	pinMode(BUTTON_DOWN, INPUT_PULLUP);
	pinMode(MOT_HALL, INPUT);

	position = EEPROM.read(EEPROM_POSITION);
	
	motor.stop();
	motor.disable();

	Serial.begin(9600);
}

void loop() {
	if(millis() - lastButtonCheck > 100) {
		wasMoving = isMoving;
		lastButtonCheck = millis();
		if(digitalRead(BUTTON_UP) == LOW && digitalRead(BUTTON_DOWN) == LOW) {
			position = 0;
			isMoving = false;
		} else if (digitalRead(BUTTON_UP) == LOW) {
			//Serial.println("UP");
			motor.enable();
			motor.setSpeed(SPEED);
			isMoving = true;
			up = true;
		} else if (digitalRead(BUTTON_DOWN) == LOW) {
			//Serial.println("DOWN");
			motor.enable();
			motor.setSpeed(-1 * SPEED);
			isMoving = true;
			up = false;
		} else {
			isMoving = false;
			motor.stop();
		}
		if(wasMoving && !isMoving) {
			EEPROM.write(EEPROM_POSITION, position);
		}
	}

	handleMotorPosition();
}
