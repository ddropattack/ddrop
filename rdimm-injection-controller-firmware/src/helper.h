/* State machine for the experiment machine */
enum state_t {
	STATE_UNKNOWN,
	STATE_ALIVE,
	STATE_CRASH,
};

/* Converts a byte into two digit ASCII hex number */
void byte2hex(uint8_t in, char *out);

/* Converts a byte into two digit dec number */
void byte2dec(uint8_t in, char *out);

/* Converts an integer into a four digit dec number */
void int2dec(uint8_t in, char *out);

