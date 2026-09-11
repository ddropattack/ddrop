#include <Arduino.h>
#include "pins.h"

//pin2state[<Teensy pin of a DISC signal>] gives you teensy pin of
//corresponding STATIC signal, e.g.,
// pin2state[PIN_CA0_DISC] = PIN_CA0_STATIC
uint8_t pin2state[] = {
	// Conversion table for teensy pins
	// of DISC and STATIC signals
	//STATIC DISC
	0, //0
	0, //1
	0, //2
	0, //3
	0, //4
	0, //5
	0, //6
	0, //7
	0, //8
	0, //9
	0, //10
	0, //11
	0, //12
	0, //13
	0, //14
	0, //15
	0, //16
	0, //17
	PIN_CA4_STATIC, //18
	PIN_CA5_STATIC, //19
	PIN_CA1_STATIC, //20
	PIN_CA6_STATIC, //21
	PIN_CA3_STATIC, //22
	0, //23
	0, //24
	0, //25
	0, //26
	0, //27
	0, //28
	0, //29
	0, //30
	0, //31
	0, //32
	0, //33
	0, //34
	0, //35
	0, //36
	0, //37
	0, //38
	PIN_CA2_STATIC, //39
	PIN_CA0_STATIC, //40
	0  //41
};

int init_gpios()
{
	pinMode(PIN_LED_ETH_RIGHT, OUTPUT);
	pinMode(PIN_LED_STATUS, OUTPUT);
	pinMode(PIN_PWR_BTN, OUTPUT);

	/* All fault injection pins */
	pinMode(PIN_CS_SWAP, OUTPUT);
	pinMode(PIN_ALERT_DISC, OUTPUT);
	pinMode(PIN_CA0_DISC, OUTPUT);
	pinMode(PIN_CA1_DISC, OUTPUT);
	pinMode(PIN_CA2_DISC, OUTPUT);
	pinMode(PIN_CA3_DISC, OUTPUT);
	pinMode(PIN_CA4_DISC, OUTPUT);
	pinMode(PIN_CA5_DISC, OUTPUT);
	pinMode(PIN_CA6_DISC, OUTPUT);
	pinMode(PIN_CA0_STATIC, OUTPUT);
	pinMode(PIN_CA1_STATIC, OUTPUT);
	pinMode(PIN_CA2_STATIC, OUTPUT);
	pinMode(PIN_CA3_STATIC, OUTPUT);
	pinMode(PIN_CA4_STATIC, OUTPUT);
	pinMode(PIN_CA5_STATIC, OUTPUT);
	pinMode(PIN_CA6_STATIC, OUTPUT);

	/* HIGH means "passthrough" mode */
	digitalWrite(PIN_CS_SWAP, LOW); //not swapped
	digitalWrite(PIN_ALERT_DISC, HIGH); //not disconnected
	digitalWrite(PIN_CA0_DISC, HIGH);
	digitalWrite(PIN_CA1_DISC, HIGH);
	digitalWrite(PIN_CA2_DISC, HIGH);
	digitalWrite(PIN_CA3_DISC, HIGH);
	digitalWrite(PIN_CA4_DISC, HIGH);
	digitalWrite(PIN_CA5_DISC, HIGH);
	digitalWrite(PIN_CA6_DISC, HIGH);
	digitalWrite(PIN_CA0_STATIC, HIGH);
	digitalWrite(PIN_CA1_STATIC, HIGH);
	digitalWrite(PIN_CA2_STATIC, HIGH);
	digitalWrite(PIN_CA3_STATIC, HIGH);
	digitalWrite(PIN_CA4_STATIC, HIGH);
	digitalWrite(PIN_CA5_STATIC, HIGH);
	digitalWrite(PIN_CA6_STATIC, HIGH);

	digitalWrite(PIN_LED_ETH_RIGHT, LOW);
	digitalWrite(PIN_LED_STATUS, LOW);

	return 0;
}
