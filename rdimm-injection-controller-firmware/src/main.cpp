#include <Arduino.h>
#include <NativeEthernet.h>
#include <TeensyThreads.h>
#include <SPI.h>

#include "pins.h"
#include "helper.h"
#include "http.h"
#include "fi.h"

/* Button press times */
#define PWR_BTN_LONG_PRESS 5000
#define PWR_BTN_SHORT_PRESS 1000

/* Seconds after machine is presumed dead after silence */
#define ALIVE_TIMEOUT 10

const char *state_names[] = {
	"Unknown",
	"Alive",
	"Crashed",
};
volatile enum state_t state = STATE_UNKNOWN;

/* Flags which indicate whether the power button should be pressed */
volatile bool pwr_btn_short_pressed = false;
volatile bool pwr_btn_long_pressed = false;

/* This counter increments continously each second.
 * If the experiment machine is alive, it will send a 
 * heart beat packet via USB HID, resetting this counter.
 * If this counter exceeds ALIVE_TIMEOUT, the machine
 * is believed to have crashed.
 */
uint32_t alive_tick_cnt = 0;

/* This thread checks for the btn_flags which 
 * indicates that the power button should be pressed 
 * and actives the power button relay for some time 
 */
void power_button()
{
	while (1) {
		if (pwr_btn_long_pressed || pwr_btn_short_pressed) {
			digitalWriteFast(PIN_PWR_BTN, HIGH);
			threads.delay(pwr_btn_long_pressed ?
					      PWR_BTN_LONG_PRESS :
					      PWR_BTN_SHORT_PRESS);
			pwr_btn_long_pressed = false;
			pwr_btn_short_pressed = false;
		}
		digitalWriteFast(PIN_PWR_BTN, LOW);
		threads.delay(100);
	}
}

/* This threads checks whether the alive counter
 * has been reset recently (machine is still awake)
 * and sets internal state accordingly.
 * While the machine has not crashed,
 * the LED is blinking.
 */
void alive_tick()
{
	while (1) {
		if (alive_tick_cnt > ALIVE_TIMEOUT) {
			if (state == STATE_ALIVE)
				state = STATE_CRASH;
		} else {
			if (state == STATE_CRASH)
				state = STATE_ALIVE;
			alive_tick_cnt++;
			digitalWrite(PIN_LED_STATUS, LOW);
		}
		threads.delay(500);
		digitalWrite(PIN_LED_STATUS, HIGH);
		threads.delay(500);
	}
}

extern "C" int main()
{
	init_gpios();

#ifndef DISABLE_ETHERNET
	init_http();
#endif

	digitalWrite(PIN_LED_ETH_RIGHT, HIGH);

	/* Set time slice of the scheduler to 
     * 1 microseconds. This creates a somewhat
     * large overhead as context switches are expensive.
     * However, it ensures low latency for the fault injection
     */
	threads.setSliceMicros(20);

	/* Start the HTTP and USB server */
	threads.addThread(alive_tick);
#ifndef DISABLE_ETHERNET
	threads.addThread(usb_server);
#endif
	threads.addThread(power_button);

	/* If everything is working normally, the user LED
     * on the injection controller should blink
     */

#ifndef DISABLE_ETHERNET
	/* HTTP server does not terminate */
	http_server();
#else
	usb_server();
#endif

	yield();
	return 0;
}

