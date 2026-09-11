/**************************************************************************
* FNET User configuration file.
* It should be used to change any default configuration parameter.
***************************************************************************/

#ifndef _FNET_USER_CONFIG_H_

#define _FNET_USER_CONFIG_H_

#define FNET_CFG_COMP_GNUC  (1) //GCC compiler

#include <Arduino.h>
#define FNET_CFG_MULTITHREADING (1)
#define FNET_CFG_BENCH_CLN (1)  //Benchmark
#define FNET_CFG_BENCH_CLN_BUFFER_SIZE          (64*1024)
#define FNET_CFG_SOCKET_TCP_TX_BUF_SIZE     (32U * 1024U)
#define FNET_CFG_SOCKET_TCP_RX_BUF_SIZE     (32U * 1024U)
#define FNET_CFG_BENCH_SRV (1)  //Benchmark
#define FNET_CFG_BENCH_SRV_BUFFER_SIZE          (128*1024)

//#define FNET_CFG_SOCKET_CALLBACK_ON_RX (1)


//#define FNET_CFG_DEBUG              (1)
//#define FNET_CFG_DEBUG_AUTOIP       (1)
//#define FNET_CFG_DEBUG_TIMER        (1)
//#define FNET_CFG_DEBUG_HTTP         (1)
//#define FNET_CFG_DEBUG_DHCP_CLN     (1)
//#define FNET_CFG_DEBUG_DHCP_SRV     (1)
//#define FNET_CFG_DEBUG_ARP          (1)
//#define FNET_CFG_DEBUG_MEMPOOL      (1)
//#define FNET_CFG_DEBUG_TFTP_CLN     (1)
//#define FNET_CFG_DEBUG_TFTP_SRV     (1)
//#define FNET_CFG_DEBUG_STACK        (1)
//#define FNET_CFG_DEBUG_TELNET       (1)
//#define FNET_CFG_DEBUG_SHELL        (1)
//#define FNET_CFG_DEBUG_DNS          (1)
//#define FNET_CFG_DEBUG_DNS          (1)
//#define FNET_CFG_DEBUG_AZURE        (1)
//#define FNET_CFG_DEBUG_IP6          (1)
//#define FNET_CFG_DEBUG_LINK         (1)
//#define FNET_CFG_DEBUG_LLMNR        (1)
//#define FNET_CFG_DEBUG_MDNS         (1)
//#define FNET_CFG_DEBUG_PING         (1)
//#define FNET_CFG_DEBUG_SNTP         (1)
//#define FNET_CFG_DEBUG_QCA          (1)
//#define FNET_CFG_DEBUG_ENET         (1)
//#define FNET_CFG_DEBUG_TRACE        (1)
//#define FNET_CFG_DEBUG_TRACE_IP4    (1)
//#define FNET_CFG_DEBUG_TRACE_ICMP4  (1)
//#define FNET_CFG_DEBUG_TRACE_IGMP   (1)
//#define FNET_CFG_DEBUG_TRACE_ETH    (1)
//#define FNET_CFG_DEBUG_TRACE_ARP    (1)
//#define FNET_CFG_DEBUG_TRACE_UDP    (1)
//#define FNET_CFG_DEBUG_TRACE_TCP    (1)

/**************************************************************************/ /*!
 * @brief General return codes, used by most of API functions.
 ******************************************************************************/
typedef enum
{
    FNET_OK  = (0), /**< No error.*/
    FNET_ERR = (-1) /**< There is error.*/
} fnet_return_t;

#if !defined(ARDUINO_TEENSY41)
/*********************************************************************
 *
 * The basic data types.
 *
 *********************************************************************/
typedef uint8_t fnet_uint8_t;         /*  8 bits */
typedef uint16_t fnet_uint16_t;   /* 16 bits */
typedef uint32_t fnet_uint32_t;    /* 32 bits */
typedef uint64_t fnet_uint64_t;   /* 64 bits */

typedef int8_t fnet_int8_t;            /*  8 bits */
typedef int16_t fnet_int16_t;      /* 16 bits */
typedef int32_t fnet_int32_t;       /* 32 bits */
typedef int64_t fnet_int64_t;      /* 64 bits */

typedef volatile fnet_uint8_t fnet_vuint8_t;     /*  8 bits */
typedef volatile fnet_uint16_t fnet_vuint16_t;   /* 16 bits */
typedef volatile fnet_uint32_t fnet_vuint32_t;   /* 32 bits */
typedef volatile fnet_uint64_t fnet_vuint64_t;   /* 64 bits */
/**************************************************************************/ /*!
 * @brief Unsigned integer type representing the index.
 ******************************************************************************/
typedef unsigned int fnet_index_t;

/**************************************************************************/ /*!
 * @brief Type representing the charecter.
 ******************************************************************************/
typedef char fnet_char_t;

/**************************************************************************/ /*!
 * @def FNET_HTONS
 * @param short_var A 16-bit number in host byte order.
 * @hideinitializer
 * @see FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 * @brief Macros which converts the unsigned short integer from host byte order to
 * network byte order.
 ******************************************************************************/
#define FNET_HTONS(short_var)   (((((fnet_uint16_t)(short_var)) & 0x00FFU) << 8U) | (((fnet_uint16_t)(short_var) & 0xFF00U) >> 8U))
/**************************************************************************/ /*!
 * @def FNET_NTOHS
 * @param short_var A 16-bit number in network byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 * @brief Macros which converts the unsigned 16-bit integer from network byte order to
 * host byte order.
 ******************************************************************************/
#define FNET_NTOHS(short_var)   FNET_HTONS(short_var)
/**************************************************************************/ /*!
 * @def FNET_HTONL
 * @param long_var A 32-bit number in host byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 * @brief Macros which converts the unsigned long integer from host byte order to
 * network byte order.
 ******************************************************************************/
#define FNET_HTONL(long_var)    ((((long_var) & 0x000000FFU) << 24U) | (((long_var) & 0x0000FF00U) << 8U) | (((long_var) & 0x00FF0000U) >> 8U) | (((long_var) & 0xFF000000U) >> 24U))
/**************************************************************************/ /*!
 * @def FNET_NTOHL
 * @param long_var A 32-bit number in network byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 * @brief Macros which converts the unsigned long integer from network byte order to
 * host byte order.
 ******************************************************************************/
#define FNET_NTOHL(long_var)    FNET_HTONL(long_var)

/***************************************************************************/ /*!
 *
 * @brief    Converts 16-bit value from host to network byte order.
 *
 *
 * @param short_var A 16-bit number in host byte order.
 *
 *
 * @return This function returns the network byte-ordered @c short_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned 16-bit integer from host byte order to
 * network byte order.
 *
 ******************************************************************************/
fnet_uint16_t fnet_htons(fnet_uint16_t short_var);

#define fnet_ntohs   fnet_htons
/***************************************************************************/ /*!
 *
 * @brief    Converts 16-bit value from network to host byte order.
 *
 *
 * @param short_var A 16-bit number in network byte order.
 *
 *
 * @return This function returns the host byte-ordered @c short_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_htonl(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned 16-bit integer from network byte order to
 * host byte order.
 *
 ******************************************************************************/
fnet_uint16_t fnet_ntohs(fnet_uint16_t short_var);

/***************************************************************************/ /*!
 *
 * @brief    Converts 32-bit value from host to network byte order.
 *
 *
 * @param long_var A 32-bit number in host byte order.
 *
 *
 * @return This function returns the network byte-ordered @c long_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_ntohs(), fnet_htons(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned long integer from host byte order to
 * network byte order.
 *
 ******************************************************************************/
fnet_uint32_t fnet_htonl(fnet_uint32_t long_var);

#define fnet_ntohl    fnet_htonl
/***************************************************************************/ /*!
 *
 * @brief    Converts 32-bit value from network to host byte order.
 *
 *
 * @param long_var A 32-bit number in network byte order.
 *
 *
 * @return This function returns the host byte-ordered @c long_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned long integer from network byte order to
 * host byte order.
 *
 ******************************************************************************/
fnet_uint32_t fnet_ntohl(fnet_uint32_t long_var);

/***************************************************************************/ /*!
 *
 * @brief    Writes character to the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @param character       Character to be written to the serial port.
 *
 * @see fnet_cpu_serial_getchar(), fnet_cpu_serial_init()
 *
 ******************************************************************************
 *
 * This function writes @c character to the serial port defined
 * by @c port_number. @n
 *
 ******************************************************************************/
#define FNET_CFG_CPU_SERIAL_PORT_DEFAULT (0U)
void fnet_cpu_serial_putchar( fnet_index_t port_number, fnet_char_t character );

/***************************************************************************/ /*!
 *
 * @brief    Reads character from the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @return This function returns:
 *   - character received by the serial port.
 *   - @ref FNET_ERR if no character is available.
 *
 * @see fnet_cpu_serial_putchar(), fnet_cpu_serial_init()
 *
 ******************************************************************************
 *
 * This function reads character from the serial port defined
 * by @c port_number. @n
 *
 ******************************************************************************/
fnet_int32_t fnet_cpu_serial_getchar( fnet_index_t port_number );

/***************************************************************************/ /*!
 *
 * @brief    Initializes the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @param baud_rate       Baud rate to be set to the serial port.
 *
 * @return This function returns:
 *   - @ref FNET_OK if successful.
 *   - @ref FNET_ERR if failed.
 *
 * @see fnet_cpu_serial_putchar(), fnet_cpu_serial_getchar()
 *
 ******************************************************************************
 *
 * This function executes the HW initialization of the serial port defined
 * by the @c port_number.
 *
 ******************************************************************************/
fnet_return_t fnet_cpu_serial_init(fnet_index_t port_number, fnet_uint32_t baud_rate);

/***************************************************************************/ /*!
 * @def FNET_CPU_ADDR_TO_INSTRUCTION
 *
 * @brief           Converts address value to the instruction.
 *
 * @param addr      Address to be adjusted.
 *
 * @see FNET_CPU_INSTRUCTION_TO_ADDR
 ******************************************************************************
 *
 * This is CPU-specific macro that Converts address value to the instruction address. @n
 * If the current platform is Kinetis, it sets the Thumb bit (bit 0) of
 * the address to 1. @n
 * If the current platform is ColdFire or MPC, it does nothing.
 *
 ******************************************************************************/
#ifndef FNET_CPU_ADDR_TO_INSTRUCTION
#define FNET_CPU_ADDR_TO_INSTRUCTION(addr)    (addr)
#endif

/***************************************************************************/ /*!
 * @def FNET_CPU_INSTRUCTION_TO_ADDR
 *
 * @brief           Converts instruction to address value.
 *
 * @param addr      Instruction to be adjusted.
 *
 * @see FNET_CPU_ADDR_TO_INSTRUCTION
 ******************************************************************************
 *
 * This is CPU-specific macro that Converts instruction to address value. @n
 * If the current platform is Kinetis, it sets the Thumb bit (bit 0) of
 * the address to 0. @n
 * If the current platform is ColdFire or MPC, it does nothing.
 *
 ******************************************************************************/
#ifndef FNET_CPU_INSTRUCTION_TO_ADDR
#define FNET_CPU_INSTRUCTION_TO_ADDR(addr)    (addr)
#endif

#endif //!ARDUINO_TEENSY41

/*****************************************************************************
* Compiler support FNET_CFG_COMP_<name> is defined in project options.
******************************************************************************/
/*****************************************************************************
* Processor type FNET_CFG_CPU_<name> is defined in project options.
******************************************************************************/
/*****************************************************************************
*  Default serial port number FNET_CFG_CPU_SERIAL_PORT_DEFAULT is defined
*  in project options.
******************************************************************************/

/*****************************************************************************
* IPv4 and/or IPv6 protocol support.
******************************************************************************/
#define FNET_CFG_IP4                (1)
//#define FNET_CFG_IP6                (1)

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP_MAX_PACKET
 * @brief    Maximum size for the IPv4 and IPv6 datagram,
 *           the largest allowed value is 65535. @n
 *           Default value is 10 KB.
 * @showinitializer
 ******************************************************************************/
#define FNET_CFG_IP_MAX_PACKET              (32U*1024U)

/*****************************************************************************
* TCP protocol support.
* You can disable it to save a substantial amount of code if
* your application only needs UDP. By default it is enabled.
******************************************************************************/
#define FNET_CFG_TCP                (1)

/*****************************************************************************
* UDP protocol support.
* You can disable it to save a some amount of code if your
* application only needs TCP. By default it is enabled.
******************************************************************************/
#define FNET_CFG_UDP                (1)

/*****************************************************************************
* UDP checksum.
* If enabled, the UDP checksum will be generated for transmitted
* datagrams and be verified on received UDP datagrams.
* You can disable it to speedup UDP applications.
* By default it is enabled.
******************************************************************************/
#define FNET_CFG_UDP_CHECKSUM       (1)

/*****************************************************************************
* IP fragmentation.
* If the IP fragmentation is enabled, the IP will attempt to reassemble IP
* packet fragments and will able to generate fragmented IP packets.
* If disabled, the IP will  silently discard fragmented IP packets..
******************************************************************************/
#define FNET_CFG_IP4_FRAGMENTATION  (1)

/*****************************************************************************
* DHCP Client service support.
******************************************************************************/
#define FNET_CFG_DHCP_CLN           (1)

/*****************************************************************************
* Auto-IP service support.
******************************************************************************/
#define FNET_CFG_AUTOIP             (1)

/*****************************************************************************
* HTTP Server service support.
******************************************************************************/
#define FNET_CFG_HTTP_SRV                       (1)
#define FNET_CFG_HTTP_SRV_AUTHENTICATION_BASIC  (1) /* Enable HTTP authentication.*/
#define FNET_CFG_HTTP_SRV_POST                  (1) /* Enable HTTP POST-method support.*/

/*****************************************************************************
* Telnet Server service support.
******************************************************************************/
#define FNET_CFG_TELNET                     (1)

/*****************************************************************************
* Flash Module driver support. It is valid only if a platform-specific
* Flash driver is availble (FNET_CFG_CPU_FLASH).
******************************************************************************/
#define FNET_CFG_FLASH                      (1)

/*****************************************************************************
* DNS client/resolver service support.
******************************************************************************/
#define FNET_CFG_DNS                        (1)

/*****************************************************************************
* Link-Local Multicast Name Resolution (LLMNR) server/responder support.
******************************************************************************/
#define FNET_CFG_LLMNR                      (1)
#define FNET_CFG_LLMNR_HOSTNAME_TTL         (2u)

/*****************************************************************************
* Multicast DNS (mDNS) "Bonjour" server/responder support.
******************************************************************************/
#define FNET_CFG_MDNS                       (1)

/*****************************************************************************
* PING service support.
******************************************************************************/
#define FNET_CFG_PING                       (1)

/*****************************************************************************
* SNTP client support.
******************************************************************************/
#define FNET_CFG_SNTP                       (1)

/*****************************************************************************
* Link-Detection service support.
******************************************************************************/
#define FNET_CFG_LINK                       (1)

/**************************************************************************/ /*!
 * @def      FNET_CFG_TLS
 * @brief    TLS Library support:
 *               - @c 1..n = is enabled (mbedTLS). Its value defines
 *                      the maximum number of the TLS contexts that can be initialized
 *                      simultaneously, using fnet_tls_init().
 *               - @b @c 0 = is disabled (Default value).
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_TLS
    #define FNET_CFG_TLS                (2)
#endif

/**************************************************************************
* Ethernet Adjustable Timer
******************************************************************************/
//#ifndef FNET_CFG_CPU_ETH_ADJUSTABLE_TIMER
//    #define FNET_CFG_CPU_ETH_ADJUSTABLE_TIMER       (1)
//#endif

/**************************************************************************
* Ethernet Enhanced Buffer Descriptor
******************************************************************************/
//#ifndef FNET_CFG_CPU_ETH_ENHANCED_BUFFER_DESCRIPTOR
//    #define FNET_CFG_CPU_ETH_ENHANCED_BUFFER_DESCRIPTOR     (1)
//#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TIMER_ALT
 * @brief    Alternative timer support:
 *               - @c 1 = is enabled. User application must implement own Timer API defined by fnet_timer_api_t and provide it during FNET initialization.
 *                 Also, user application must call fnet_timer_poll() periodically with 100 ms period or less. @n
 *                 It is mainly used by applications that want to use RTOS timer instead of FNET bare-metal one.
 *               - @b @c 0 = is disabled (Default value).@n
 *                 FNET uses own bare-metal timer and does automatic timer polling in interrupt.
 * @see fnet_timer_api_t, fnet_timer_poll()
 ******************************************************************************/
#ifndef FNET_CFG_TIMER_ALT
    #define FNET_CFG_TIMER_ALT                  (1)
#endif

#endif /* _FNET_USER_CONFIG_H_ */
