#ifndef fnet_usb_h
#define fnet_usb_h

#include "fnet_config.h"

#include "fnet.h"
#include "stack/fnet_eth_prv.h"
#include "stack/fnet_error.h"
#include "stack/fnet_debug.h"
#include "stack/fnet_isr.h"
#include "stack/fnet_prot.h"
#include "stack/fnet_arp.h"
#include "stack/fnet_timer_prv.h"
#include "stack/fnet_loop.h"

#include "stack/fnet_stdlib.h"

//#if !defined(ARDUINO_TEENSY41)


/************************************************************************
 *     Function Prototypes
 *************************************************************************/
fnet_return_t fnet_usb_init(fnet_netif_t *netif);
void fnet_usb_release(fnet_netif_t *netif);

fnet_return_t fnet_usb_get_hw_addr(fnet_netif_t *netif, uint8_t * hw_addr);
fnet_return_t fnet_usb_set_hw_addr(fnet_netif_t *netif, uint8_t * hw_addr);
fnet_bool_t fnet_usb_is_connected(fnet_netif_t *netif);
fnet_return_t fnet_usb_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics);
void fnet_usb_eth_output(fnet_netif_t *netif, fnet_netbuf_t *nb);

#ifdef FNET_CFG_MULTICAST
void fnet_usb_multicast_join(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
void fnet_usb_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
#endif /* FNET_CFG_MULTICAST */

fnet_return_t _fnet_usb_phy_read(fnet_netif_t *netif, fnet_uint32_t reg_addr, fnet_uint16_t *data);
fnet_return_t _fnet_usb_phy_write(fnet_netif_t *netif, fnet_uint32_t reg_addr, fnet_uint16_t data);
#if defined(__cplusplus)
extern "C" {
#endif
fnet_uint32_t fnet_usb_crc_hash(fnet_mac_addr_t multicast_addr);
#if defined(__cplusplus)
}
#endif
void fnet_usb_multicast_join(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
void fnet_usb_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);

/* For debug needs.*/
void fnet_usb_stop(fnet_netif_t *netif);
void fnet_usb_resume(fnet_netif_t *netif);

void (*_handleGetMACAddress)(fnet_mac_addr_t * hw_addr);
void setHandleGetMACAddress(void (*fptr)(fnet_mac_addr_t * hw_addr)) {
    _handleGetMACAddress = fptr;
};
void (*_handleSetMACAddress)(uint8_t * hw_addr);
void setHandleSetMACAddress(void (*fptr)(uint8_t * hw_addr)) {
    _handleSetMACAddress = fptr;
};
void (*_handleMulticastJoin)(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
void setHandleMulticastJoin(void (*fptr)(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr)) {
    _handleMulticastJoin = fptr;
};
void (*_handleMulticastLeave)(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
void setHandleMulticastLeave(void (*fptr)(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr)) {
    _handleMulticastLeave = fptr;
};
void (*_handlePHYRead)(fnet_uint32_t reg_addr, fnet_uint16_t *data);
void setHandlePHYRead(void (*fptr)(fnet_uint32_t reg_addr, fnet_uint16_t *data)) {
    _handlePHYRead = fptr;
};
void (*_handlePHYWrite)(fnet_uint32_t reg_addr, fnet_uint16_t data);
void setHandlePHYWrite(void (*fptr)(fnet_uint32_t reg_addr, fnet_uint16_t data)) {
    _handlePHYWrite = fptr;
};
void (*_handleOutput)(fnet_netif_t *netif, fnet_netbuf_t *nb);
void setHandleOutput(void (*fptr)(fnet_netif_t *netif, fnet_netbuf_t *nb)) {
    _handleOutput = fptr;
};
void (*_handleStatistics)(struct fnet_netif *netif, struct fnet_netif_statistics * statistics);
void setHandleStatistics(void (*fptr)(struct fnet_netif *netif, struct fnet_netif_statistics * statistics)) {
    _handleStatistics = fptr;
};
fnet_bool_t (*_handleIsConnected)();
void setHandleIsConnected(fnet_bool_t (*fptr)()) {
    _handleIsConnected = fptr;
};
extern fnet_netif_t fnet_usb0_if;
#define FNET_CPU_USB0_IF ((fnet_netif_desc_t)(&fnet_usb0_if))
#if !defined(ARDUINO_TEENSY41)
#define FNET_CPU_ETH0_IF FNET_CPU_USB0_IF
#endif

#define FNET_CPU_DATA_MEMORY_BARRIER    FNET_COMP_ASM("DMB")
fnet_netif_t * fnet_usb_get_netif();

//#endif // !ARDUINO_TEENSY41
#endif /* fnet_usb_eth_h */
