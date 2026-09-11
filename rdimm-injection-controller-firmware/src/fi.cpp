/*
 * USB Server that also does the actual fault injection
 */

#include <Arduino.h>
#include <TeensyThreads.h>
#include "fi.h"
#include "helper.h"

/* General purpose data packet buffer */
#define GP_DATA_BUF_SIZE 65536
uint8_t gp_data[GP_DATA_BUF_SIZE] = { 0 };
uint32_t gp_data_len = 0;

extern uint8_t pin2state[];
extern uint8_t alive_tick_cnt;
extern enum state_t state;

char dimm_type[64];
char dimm_manufacturer[64];
char dimm_description[64];
char dimm_year[3];
char dimm_week[3];

int runtime_hours;
int runtime_mins;
int runtime_secs;
char runtime[16];

void usb_server()
{
	while (1) {
		if (usb_rawhid_available()) {
			threads.stop();
			/* A new packet has arrived */

			char hidbuf[64] = { 0 };
			usb_rawhid_recv(hidbuf, 64);

			/* Fault-injection trigger */
			if (hidbuf[0] == 0xAA && hidbuf[1] == 0xBB) {
				digitalWriteFast(pin2state[hidbuf[2]],
						 !hidbuf[5]);
				/* making sure analog switch has transitioned first */
				delayNanoseconds(500);

				/* Less delay than 600 ns is not useful,
         * as the HF switch has a maximum switch time
         * of 600 ns, and therefore an actual transition
         * is not guaranteed.
         */
				for (int i = 0; i < hidbuf[6]; i++) {
					/* disconnect the line */
					digitalWriteFast(hidbuf[2], LOW);

					/* fault duration */
					delayMicroseconds(
						((hidbuf[3] << 8) | hidbuf[4]));

					/* reconnect the line */
					digitalWriteFast(hidbuf[2], HIGH);

					/* inter-fault delay */
					delayMicroseconds(
						((hidbuf[7] << 8) | hidbuf[8]));
				}
			}

			// CA pins: Passthrough
			// CS lines: Swap
			if (hidbuf[0] == 0xAA && hidbuf[1] == 0x11) {
				digitalWriteFast(pin2state[hidbuf[2]], !hidbuf[5]);

				/* making sure analog switch has transitioned first */
				delayNanoseconds(1000);
				
				/* reconnect the line */
				digitalWriteFast(hidbuf[2], HIGH);

				digitalWrite(2, HIGH);
			}

			// CA pins: Disconnect
			// CS lines: Passthrough
			if (hidbuf[0] == 0xAA && hidbuf[1] == 0x00) {
				digitalWriteFast(pin2state[hidbuf[2]], !hidbuf[5]);

				/* making sure analog switch has transitioned first */
				delayNanoseconds(1000);

				/* reconnect the line */
				digitalWriteFast(hidbuf[2], LOW);

				digitalWrite(2, LOW);
			}

			// Put the teensy in bootloader mode to allow reprogramming remotely
			if (hidbuf[0] == 0xCC && hidbuf[1] == 0x22) {
				asm("bkpt #251");
			}

			threads.start();

			/* At least we now know that the machine 
       * is up and well 
       */
			state = STATE_ALIVE;
			alive_tick_cnt = 0;

			/* General purpose data */
			if (gp_data_len < GP_DATA_BUF_SIZE - 61) {
				if (hidbuf[0] == 0xBB && hidbuf[1] == 0x12) {
					memcpy(gp_data + gp_data_len, hidbuf,
					       64);
					gp_data_len += 64;
				}
			}

			/* DIMM info */
			if (hidbuf[0] == 0xBB && hidbuf[1] == 0x13) {
				memcpy(dimm_type, hidbuf + 2, 5);
				memcpy(dimm_manufacturer, hidbuf + 10, 20);
				memcpy(dimm_description, hidbuf + 28, 33);
				byte2dec(hidbuf[62], dimm_week);
				byte2dec(hidbuf[63], dimm_year);
			}

			/* heart beat */
			if (hidbuf[0] == 0xBB && hidbuf[1] == 0x14) {
				runtime_hours = hidbuf[3] << 8;
				runtime_hours |= hidbuf[4];
				runtime_mins = hidbuf[5];
				runtime_secs = hidbuf[6];
				int2dec(runtime_hours, runtime);
				runtime[4] = ':';
				byte2dec(runtime_mins, runtime + 5);
				runtime[7] = ':';
				byte2dec(runtime_secs, runtime + 8);
				runtime[10] = '\0';
			}
		}
	}
}
