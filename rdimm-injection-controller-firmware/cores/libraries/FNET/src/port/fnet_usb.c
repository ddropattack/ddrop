
#include "fnet_user_config.h"
#include "fnet.h"
#include "fnet_config.h"
#include "fnet_usb.h"
#include "stack/fnet_eth_prv.h"

//#if !defined(ARDUINO_TEENSY41)

/************************************************************************
 * NAME: inits
 *
 * DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
 *************************************************************************/
fnet_return_t fnet_usb_init(fnet_netif_t *netif){
    (void) netif;
    return FNET_OK;
}

void fnet_usb_release(fnet_netif_t *netif) {
    (void) netif;
}


/************************************************************************
 * NAME: fnet_usb_get_hw_addr
 *
 * DESCRIPTION: This function reads MAC address.
 *************************************************************************/
fnet_return_t fnet_usb_get_hw_addr(fnet_netif_t *netif, uint8_t * hw_addr){
    fnet_return_t result;

    if(netif && (netif->netif_api->netif_type == FNET_NETIF_TYPE_ETHERNET)
       && (hw_addr) )
    {
//        Get MAC adress here
        if(_handleGetMACAddress != NULL){
          _handleGetMACAddress((fnet_mac_addr_t *)hw_addr);
        }
        result = FNET_OK;
    }
    else
    {
        result = FNET_ERR;
    }

    return result;
}

fnet_return_t fnet_usb_set_hw_addr(fnet_netif_t *netif, uint8_t * hw_addr){
    fnet_return_t result;

    /* Set the source address for the controller. */
    if(netif
       && (netif->netif_api->netif_type == FNET_NETIF_TYPE_ETHERNET)
       && hw_addr
       && (fnet_memcmp(hw_addr, fnet_eth_null_addr, sizeof(fnet_mac_addr_t)) != 0)
       && (fnet_memcmp(hw_addr, fnet_eth_broadcast, sizeof(fnet_mac_addr_t)) != 0)
       && ((hw_addr[0] & 0x01U) == 0x00U)) /* Most significant nibble should be always even.*/
    {
//        Set MAC adress here
        if(_handleSetMACAddress != NULL){
          _handleSetMACAddress(hw_addr);
        }
        _fnet_eth_change_addr_notify(netif);

        result = FNET_OK;
    }
    else
    {
        result = FNET_ERR;
    }

    return result;
}

fnet_return_t fnet_usb_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics){ //Number of rx and tx packets
    (void) netif;
    (void) statistics;
    return FNET_OK;
}

fnet_bool_t fnet_usb_is_connected(fnet_netif_t *netif){
    if (_handleIsConnected != NULL) {
        return _handleIsConnected();
    }
    return false;
}

/************************************************************************
 * NAME: fnet_usb_output
 *
 * DESCRIPTION: Ethernet low-level output function.
 *************************************************************************/
void fnet_usb_eth_output(fnet_netif_t *netif, fnet_netbuf_t *nb){
//    fnet_uint8_t        *tx_buffer;
//
//    if(nb && (nb->total_length >= FNET_ETH_HDR_SIZE))
//    {
//        _fnet_netbuf_to_buf(nb, 0u, FNET_NETBUF_COPYALL, tx_buffer);
//    }
    if(_handleOutput != NULL){
      _handleOutput(netif, nb);
    }
    _fnet_netbuf_free_chain(nb);
}

void fnet_usb_stop(fnet_netif_t *netif) {
    (void) netif;
}

void fnet_usb_resume(fnet_netif_t *netif) {
    (void) netif;
}


/************************************************************************
* DESCRIPTION: Read a value from a PHY's MII register.
* reg_addr < address of the register in the PHY
* data < Pointer to storage for the Data to be read from the PHY register (passed by reference)
* Return FNET_ERR on failure, FNET_OK on success
*************************************************************************/
fnet_return_t _fnet_usb_phy_read(fnet_netif_t *netif, fnet_uint32_t reg_addr, fnet_uint16_t *data) {
//    fnet_return_t       result;
//    fnet_eth_if_t       *eth_if = (fnet_eth_if_t *)(netif->netif_prv);

//Read phy into data pointer
    if(_handlePHYRead != NULL){
      _handlePHYRead(reg_addr, data);
    }

return FNET_OK; //Or FNET_OK
}

/************************************************************************
* DESCRIPTION: Write a value to a PHY's MII register.
* reg_addr = address of the register in the PHY
* data = Data to be writen to the PHY register (passed by reference)
* Return FNET_ERR on failure (timeout), FNET_OK on success
*************************************************************************/
fnet_return_t _fnet_usb_phy_write(fnet_netif_t *netif, fnet_uint32_t reg_addr, fnet_uint16_t data) {
//    fnet_return_t       result;
//    fnet_eth_if_t       *eth_if = (fnet_eth_if_t *)(netif->netif_prv);
    if(_handlePHYWrite != NULL){
        _handlePHYWrite(reg_addr, data);
    }
    return FNET_OK; //Or FNET_OK
}

/*******************************************************************************
 *     Ethernet interface API structure for PHY.
 ******************************************************************************/
const fnet_eth_api_t fnet_usb_eth_api = {
    .phy_read = _fnet_usb_phy_read,
    .phy_write = _fnet_usb_phy_write,
};

#if FNET_CFG_MULTICAST
/************************************************************************
* DESCRIPTION: Joins a multicast group on USB interface.
*************************************************************************/
fnet_uint32_t fnet_usb_crc_hash(fnet_mac_addr_t multicast_addr) {
    fnet_uint32_t    crc = 0xFFFFFFFFu;
    fnet_index_t     i;
    fnet_index_t     j;
    
    for (i = 0u; i < 6u; i++)
    {
        fnet_uint8_t c = multicast_addr[i];
        for (j = 0u; j < 8u; j++)
        {
            if (((c ^ crc) & 1u) != 0u)
            {
                crc >>= 1;
                c >>= 1;
                crc ^= 0xEDB88320u;
            }
            else
            {
                crc >>= 1;
                c >>= 1;
            }
        }
    }
    return  crc;
}
void fnet_usb_multicast_join(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr) {
//    fnet_uint32_t   reg_value;
//    fnet_uint32_t   crc;
//
//    /* Set the appropriate bit in the hash table */
//    crc = fnet_usb_crc_hash(multicast_addr );
//    crc >>= 26;
//    crc &= 0x3FU;
//
//    reg_value = (fnet_uint32_t)(0x1U << (crc & 0x1FU));
    if(_handleMulticastJoin != NULL){
      _handleMulticastJoin(netif, multicast_addr);
    }
}

/************************************************************************
* DESCRIPTION: Leavess a multicast group on USB interface.
*************************************************************************/
void fnet_usb_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr) {
//    fnet_uint32_t   reg_value;
//    fnet_uint32_t   crc;
//
//    /* Set the appropriate bit in the hash table */
//    crc = fnet_usb_crc_hash(multicast_addr );
//    crc >>= 26;
//    crc &= 0x3FU;
//
//    reg_value = (fnet_uint32_t)(0x1U << (crc & 0x1FU));
    if(_handleMulticastLeave != NULL){
      _handleMulticastLeave(netif, multicast_addr);
    }
}
#endif /* FNET_CFG_MULTICAST */

/*****************************************************************************
 * network-interface general API structure.
 ******************************************************************************/

const fnet_netif_api_t fnet_usb_mac_api = {
    .netif_type = FNET_NETIF_TYPE_ETHERNET,             /* Data-link type. */
    .netif_hw_addr_size = 6,
    .netif_init = fnet_usb_init,                        /* Initialization function.*/
    .netif_release = fnet_usb_release,                  /* Shutdown function.*/
#if FNET_CFG_IP4
    .netif_output_ip4 = _fnet_eth_output_ip4,            /* IPv4 Transmit function.*/
#endif
    .netif_change_addr_notify = _fnet_eth_change_addr_notify,    /* Address change notification function.*/
    .netif_drain = _fnet_eth_drain,                      /* Drain function.*/
    .netif_get_hw_addr = fnet_usb_get_hw_addr,
    .netif_set_hw_addr = fnet_usb_set_hw_addr,
    .netif_is_connected = fnet_usb_is_connected,
    .netif_get_statistics = fnet_usb_get_statistics,
#if FNET_CFG_MULTICAST
#if FNET_CFG_IP4
    .netif_multicast_join_ip4 = _fnet_eth_multicast_join_ip4,
    .netif_multicast_leave_ip4 = _fnet_eth_multicast_leave_ip4,
#endif
#if FNET_CFG_IP6
    .netif_multicast_join_ip6 = _fnet_eth_multicast_join_ip6,
    .netif_multicast_leave_ip6 = _fnet_eth_multicast_leave_ip6,
#endif
#endif
#if FNET_CFG_IP6
    .netif_output_ip6 = _fnet_eth_output_ip6,           /* IPv6 Transmit function.*/
#endif
    .eth_api = &fnet_usb_eth_api,
};

/*****************************************************************************
 *     Ethernet Control data structure
 ******************************************************************************/
static fnet_eth_if_t fnet_usb_usb0_if = {
//    .eth_prv = NULL,                       /* Points to Ethernet driver-specific control data structure. */
    .eth_mac_number = 0,                            /* MAC module number. */
    .eth_output = fnet_usb_eth_output,                  /* Ethernet driver output.*/
    .eth_phy_addr = 0x10,     /* Set default PHY address */
#if FNET_CFG_MULTICAST
    .eth_multicast_join = fnet_usb_multicast_join,  /* Ethernet driver join multicast group.*/
    .eth_multicast_leave = fnet_usb_multicast_leave /* Ethernet driver leave multicast group.*/
#endif
};

/************************************************************************
 * Network interface structure.
 * DESCRIPTION: Indexes ethernet interface and network API structures. See
 *  fnet_netif_prv.h for full specification of this struct.
 *************************************************************************/
fnet_netif_t fnet_usb0_if = {
    .netif_name = {'e','t','h','0'},     /* Network interface name.*/
    .netif_mtu = 1500,       /* Maximum transmission unit.*/
    .netif_prv = &fnet_usb_usb0_if,        /* Points to interface specific data structure.*/
    .netif_api = &fnet_usb_mac_api                /* Interface API */
};

fnet_netif_t * fnet_usb_get_netif() {
    return &fnet_usb0_if;
}

//#endif // !ARDUINO_TEENSY41
