#include "fnet.h"
#include "fnet_user_config.h"
#if !defined(ARDUINO_TEENSY41)

//#if FNET_USB

fnet_uint16_t fnet_htons(fnet_uint16_t short_var)
{
    fnet_uint16_t result = FNET_HTONS(short_var);
    return result;
}
/* Convert long from host- to network byte order.*/
fnet_uint32_t fnet_htonl(fnet_uint32_t long_var)
{
    fnet_uint32_t result = FNET_HTONL(long_var);
    return result;
}

/********************************************************************/
void fnet_cpu_serial_putchar (fnet_index_t port_number, fnet_char_t character)  //Not setup
{
  (void) port_number;
}
/********************************************************************/
fnet_int32_t fnet_cpu_serial_getchar (fnet_index_t port_number) //Not setup
{
  (void) port_number;
    return FNET_OK;
}

/********************************************************************/
fnet_return_t fnet_cpu_serial_init(fnet_index_t port_number, fnet_uint32_t baud_rate)   //Not setup
{
  (void) port_number;
  (void) baud_rate;
    return FNET_OK;
}

//#endif /*FNET_USB*/
#endif // !ARDUINO_TEENSY41
