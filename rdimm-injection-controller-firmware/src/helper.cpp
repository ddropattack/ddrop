#include <Arduino.h>
#include "helper.h"

void byte2hex(uint8_t in, char *out)
{
	uint8_t n1 = in & 0x0F;
	uint8_t n2 = (in & 0xF0) >> 4;
	out[1] = (n1 < 10) ? (n1 + 0x30) : (n1 + 0x41 - 10);
	out[0] = (n2 < 10) ? (n2 + 0x30) : (n2 + 0x41 - 10);
}

void byte2dec(uint8_t in, char *out)
{
	out[2] = '\0';
	out[1] = (in % 10) + 0x30;
	out[0] = ((in / 10) % 10) + 0x30;
}

void int2dec(uint8_t in, char *out)
{
	out[4] = '\0';
	out[3] = (in % 10) + 0x30;
	out[2] = ((in / 10) % 10) + 0x30;
	out[1] = ((in / 100) % 10) + 0x30;
	out[0] = ((in / 1000) % 10) + 0x30;
}

