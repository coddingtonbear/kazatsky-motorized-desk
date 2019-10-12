#include <Arduino.h>
#include <BTS7960.h>

#define BUTTON_UP PD4
#define BUTTON_DOWN PD3

#define MOT_L_EN PD7
#define MOT_L_PWM PD5
#define MOT_R_EN PD2
#define MOT_R_PWM PD6

#define MOT_PWR_VOLTAGE PC5
#define MOT_HALL PC1

BTS7960 motor(MOT_R_PWM, MOT_L_PWM, MOT_L_EN, MOT_R_EN);

#define SPEED 200

void setup() {
	pinMode(BUTTON_UP, INPUT_PULLUP);
	pinMode(BUTTON_DOWN, INPUT_PULLUP);
	
	motor.stop();
	motor.disable();

	Serial.begin(9600);
}

void loop() {
	if(digitalRead(BUTTON_UP) == LOW && digitalRead(BUTTON_DOWN) == LOW) {
		// Nothing for now; later use to
		// set "Low" position on desk
	} else if (digitalRead(BUTTON_UP) == LOW) {
		Serial.println("UP");
		motor.enable();
		motor.setSpeed(SPEED);
	} else if (digitalRead(BUTTON_DOWN) == LOW) {
		Serial.println("DOWN");
		motor.enable();
		motor.setSpeed(-1 * SPEED);
	} else {
		motor.stop();
	}
	delay(100);
}
