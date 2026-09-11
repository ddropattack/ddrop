#include <Arduino.h>
#include <NativeEthernet.h>
#include <TeensyThreads.h>
#include "http.h"
#include "helper.h"

/* Minimal HTTP server for control server API */

/* MAC and IP address for the HTTP API server */
#define ID 178
byte mac[] = { 45, 23, 179, 2, 18, 77 }; // is set by hardware in init()
IPAddress ip(192, 168, 2, ID);

/* HTTP standard listens on standard port 80 */
EthernetServer server(80);

extern char dimm_type[64];
extern char dimm_manufacturer[64];
extern char dimm_description[64];
extern char dimm_year[3];
extern char dimm_week[3];
extern int runtime_hours;
extern int runtime_mins;
extern int runtime_secs;
extern char runtime[16];

extern volatile enum state_t state;
extern char *state_names[];

extern uint8_t gp_data[];
extern uint32_t gp_data_len;

extern volatile bool pwr_btn_long_pressed;
extern volatile bool pwr_btn_short_pressed;

void get_mac_from_hardware(uint8_t *mac) {
        for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
        for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
}

int init_http()
{
    get_mac_from_hardware(mac);
	Ethernet.begin(mac, ip);

	while (Ethernet.linkStatus() == LinkOFF) {
		delay(2000);
	}

	// start the server
	server.begin();

	return 0;
}

static void http_ok(EthernetClient *client)
{
	client->println("HTTP/1.1 200 OK");
	client->println("Content-Type: application/json");
	client->println("Connection: close");
	client->println("");
}

static void http_404(EthernetClient *client)
{
	client->println("HTTP/1.1 404 Not Found");
	client->println("Content-Type: application/json");
	client->println("Connection: close");
	client->println("");
}
void http_server()
{
	while (1) {
		EthernetClient client = server.available();
		if (client) {
			//client.println("HTTP/1.1 200 OK");
			if (client.available()) {
				uint8_t buf[2048];

				//digitalWriteFast(PIN_LED_STATUS, HIGH);
				//digitalWriteFast(PIN_LED_STATUS, LOW);

				int len = client.read(buf, sizeof(buf));
				buf[len] = '\0';

				/* copy the first line */
				char request_line[128];
				char resource[256];
				uint16_t i = 0;
				while (buf[i] != '\r' && i < len &&
				       i < sizeof(request_line)) {
					request_line[i] = buf[i];
					i++;
				}
				request_line[i] = '\0';
				char *split = strtok(request_line, " ");

				/* parse method, request target and version */
				if (split) {
					//todo
					split = strtok(NULL, " ");
				} else {
				}
				if (split) {
					strcpy(resource, split + 1);
					split = strtok(NULL, " ");
				} else {
				}

				bool transmit_data = false;

				if (!strcmp("pwr_btn_short", resource)) {
					pwr_btn_short_pressed = true;
					http_ok(&client);

					state = STATE_UNKNOWN;
				} else if (!strcmp("pwr_btn_long", resource)) {
					pwr_btn_long_pressed = true;
					http_ok(&client);

					state = STATE_UNKNOWN;

				} else if (!strcmp("get_data", resource)) {
					http_ok(&client);
					transmit_data = true;
				} else {
					http_404(&client);
				}

				client.print("{\"command_given\":\"");
				client.print(resource);
				client.print("\"");
				if (transmit_data) {
					client.print(",");
					client.print(
						"\"general_purpose_data\": {\"data\":\"");
					for (uint32_t i = 0; i < gp_data_len;
					     i++) {
						char byte[3];
						byte2hex(gp_data[i], byte);
						byte[2] = '\0';
						client.print(byte);
					}
					client.print("\",");
					client.print("\"length\":\"0x");
					char len[5];
					byte2hex((gp_data_len & 0xff00) >> 8,
						 len);
					byte2hex(gp_data_len & 0x00ff, len + 2);
					len[4] = '\0';
					client.print(len);
					gp_data_len = 0;
					transmit_data = true;
					client.print("\"}");
				}

				client.print(",");
				if (state == STATE_ALIVE) {
					client.print("\"dimm\": {\"type\":\"");
					client.print(dimm_type);
					client.print("\",\"manufacturer\":\"");
					client.print(dimm_manufacturer);
					client.print("\",\"description\":\"");
					client.print(dimm_description);
					client.print(
						"\",\"manufacturing_date\":\"");
					if (*dimm_week >= '0' &&
					    *dimm_week <= '9' &&
					    *dimm_year >= '0' &&
					    *dimm_year <= '9') {
						client.print("W");
						client.print(dimm_week);
						client.print("-20");
						client.print(dimm_year);
					}
					client.print("\"},");
					if (runtime != 0) {
						client.print("\"uptime\":\"");
						client.print(runtime);
						client.print("\",");
					}
				}
				client.print("\"machine_status\":\"");
				if (state_names[state] != 0) {
					client.print(state_names[state]);
				}

				client.println("\"}");
			}
			delay(5);
			client.stop();
		}
		delay(50);
	}
}

