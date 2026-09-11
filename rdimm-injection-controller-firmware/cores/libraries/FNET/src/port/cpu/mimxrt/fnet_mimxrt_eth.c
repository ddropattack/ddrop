/**************************************************************************
*
* Copyright 2018 by Andrey Butok. FNET Community.
*
***************************************************************************
*
*  Licensed under the Apache License, Version 2.0 (the "License"); you may
*  not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
*  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
***************************************************************************
*
*  Ethernet driver interafce.
*
***************************************************************************/

#include "fnet.h"
#if FNET_MIMXRT && FNET_CFG_CPU_ETH0
#define CLRSET(reg, clear, set) ((reg) = ((reg) & ~(clear)) | (set))
#define RMII_PAD_INPUT_PULLDOWN 0x30E9
#define RMII_PAD_INPUT_PULLUP   0xB0E9
#define RMII_PAD_CLOCK          0x0031

#include "port/netif/fec/fnet_fec.h"

static fnet_return_t fnet_mimxrt_eth_init(fnet_netif_t *netif);
static fnet_return_t fnet_mimxrt_eth_phy_init(fnet_netif_t *netif);

/************************************************************************
* Ethernet interface structure.
*************************************************************************/
static fnet_eth_if_t fnet_mimxrt_eth0_if =
{
    .eth_prv = &fnet_fec0_if,                       /* Points to Ethernet driver-specific control data structure. */
    .eth_mac_number = 0,                            /* MAC module number. */
    .eth_output = fnet_fec_output,                  /* Ethernet driver output.*/
    .eth_phy_addr = FNET_CFG_CPU_ETH0_PHY_ADDR,     /* Set default PHY address */
    .eth_cpu_init = fnet_mimxrt_eth_init,
    .eth_cpu_phy_init = fnet_mimxrt_eth_phy_init,
#if FNET_CFG_MULTICAST
    .eth_multicast_join = fnet_fec_multicast_join,  /* Ethernet driver join multicast group.*/
    .eth_multicast_leave = fnet_fec_multicast_leave /* Ethernet driver leave multicast group.*/
#endif
};

fnet_netif_t fnet_cpu_eth0_if =
{
    .netif_name = FNET_CFG_CPU_ETH0_NAME,     /* Network interface name.*/
    .netif_mtu = FNET_CFG_CPU_ETH0_MTU,       /* Maximum transmission unit.*/
    .netif_prv = &fnet_mimxrt_eth0_if,        /* Points to interface specific data structure.*/
    .netif_api = &fnet_fec_api                /* Interface API */
};

/************************************************************************
* DESCRIPTION: Ethernet IO initialization.
*************************************************************************/
static fnet_return_t fnet_mimxrt_eth_init(fnet_netif_t *netif)
{
#if FNET_CFG_CPU_ETH_IO_INIT
#if FNET_CFG_CPU_MIMXRT1052 || FNET_CFG_CPU_MIMXRT1062
      CCM_CCGR1 |= CCM_CCGR1_ENET(CCM_CCGR_ON);
      // configure PLL6 for 50 MHz, pg 1173
      CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_POWERDOWN
        | CCM_ANALOG_PLL_ENET_BYPASS | 0x0F;
      CCM_ANALOG_PLL_ENET_SET = CCM_ANALOG_PLL_ENET_ENABLE | CCM_ANALOG_PLL_ENET_BYPASS
        /*| CCM_ANALOG_PLL_ENET_ENET2_REF_EN*/ | CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN
        /*| CCM_ANALOG_PLL_ENET_ENET2_DIV_SELECT(1)*/ | CCM_ANALOG_PLL_ENET_DIV_SELECT(1);
      while (!(CCM_ANALOG_PLL_ENET & CCM_ANALOG_PLL_ENET_LOCK)) ; // wait for PLL lock
      CCM_ANALOG_PLL_ENET_CLR = CCM_ANALOG_PLL_ENET_BYPASS;
    //  Serial.printf("PLL6 = %08X (should be 80202001)\n", CCM_ANALOG_PLL_ENET);
      // configure REFCLK to be driven as output by PLL6, pg 326
      CLRSET(IOMUXC_GPR_GPR1, IOMUXC_GPR_GPR1_ENET1_CLK_SEL | IOMUXC_GPR_GPR1_ENET_IPG_CLK_S_EN,
        IOMUXC_GPR_GPR1_ENET1_TX_CLK_DIR);
    //  Serial.printf("GPR1 = %08X\n", IOMUXC_GPR_GPR1);

      // configure pins
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_14 = 5; // Reset   B0_14 Alt5 GPIO7.15
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_15 = 5; // Power   B0_15 Alt5 GPIO7.14
      GPIO7_GDIR |= (1<<14) | (1<<15);
      GPIO7_DR_SET = (1<<15);   // power on
      GPIO7_DR_CLEAR = (1<<14); // reset PHY chip
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_04 = RMII_PAD_INPUT_PULLDOWN; // PhyAdd[0] = 0
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_06 = RMII_PAD_INPUT_PULLDOWN; // PhyAdd[1] = 1
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_05 = RMII_PAD_INPUT_PULLUP;   // Master/Slave = slave mode
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_11 = RMII_PAD_INPUT_PULLDOWN; // Auto MDIX Enable
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_07 = RMII_PAD_INPUT_PULLUP;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_08 = RMII_PAD_INPUT_PULLUP;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_09 = RMII_PAD_INPUT_PULLUP;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_B1_10 = RMII_PAD_CLOCK;
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_05 = 3; // RXD1    B1_05 Alt3, pg 525
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_04 = 3; // RXD0    B1_04 Alt3, pg 524
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_10 = 6 | 0x10; // REFCLK  B1_10 Alt6, pg 530
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_11 = 3; // RXER    B1_11 Alt3, pg 531
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_06 = 3; // RXEN    B1_06 Alt3, pg 526
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_09 = 3; // TXEN    B1_09 Alt3, pg 529
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_07 = 3; // TXD0    B1_07 Alt3, pg 527
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_08 = 3; // TXD1    B1_08 Alt3, pg 528
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_15 = 0; // MDIO    B1_15 Alt0, pg 535
      IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_14 = 0; // MDC     B1_14 Alt0, pg 534
      IOMUXC_ENET_MDIO_SELECT_INPUT = 2; // GPIO_B1_15_ALT0, pg 792
      IOMUXC_ENET0_RXDATA_SELECT_INPUT = 1; // GPIO_B1_04_ALT3, pg 792
      IOMUXC_ENET1_RXDATA_SELECT_INPUT = 1; // GPIO_B1_05_ALT3, pg 793
      IOMUXC_ENET_RXEN_SELECT_INPUT = 1; // GPIO_B1_06_ALT3, pg 794
      IOMUXC_ENET_RXERR_SELECT_INPUT = 1; // GPIO_B1_11_ALT3, pg 795
      IOMUXC_ENET_IPG_CLK_RMII_SELECT_INPUT = 1; // GPIO_B1_10_ALT6, pg 791
      delayMicroseconds(2);
      GPIO7_DR_SET = (1<<14); // start PHY chip
      ENET_MSCR = ENET_MSCR_MII_SPEED(9);
      delayMicroseconds(5);
    //  Serial.printf("RCSR:%04X, LEDCR:%04X, PHYCR %04X\n",
    //    mdio_read(0, 0x17), mdio_read(0, 0x18), mdio_read(0, 0x19));

      // LEDCR offset 0x18, set LED_Link_Polarity, pg 62
      _fnet_eth_phy_write(netif, 0x18, 0x0280); // LED shows link status, active high
      // RCSR offset 0x17, set RMII_Clock_Select, pg 61
      _fnet_eth_phy_write(netif, 0x17, 0x0081); // config for 50 MHz clock input

    //  Serial.printf("RCSR:%04X, LEDCR:%04X, PHYCR %04X\n",
    //    mdio_read(0, 0x17), mdio_read(0, 0x18), mdio_read(0, 0x19));

    //  printhex("MDIO PHY ID2 (LAN8720A is 0007, DP83825I is 2000): ", mdio_read(0, 2));
    //  printhex("MDIO PHY ID3 (LAN8720A is C0F?, DP83825I is A140): ", mdio_read(0, 3));
    //  printhex("BMCR: ", mdio_read(0, 0));
    //  printhex("BMSR: ", mdio_read(0, 1));

#endif /* FNET_CFG_CPU_MIMXRT1052 || FNET_CFG_CPU_MIMXRT1062 */
#endif /*!FNET_CFG_CPU_ETH_IO_INIT*/
    return FNET_OK;
}

/************************************************************************
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/
static fnet_return_t fnet_mimxrt_eth_phy_init(fnet_netif_t *netif)
{
#if FNET_CFG_CPU_MIMXRT1052 || FNET_CFG_CPU_MIMXRT1062
  
#endif /* (FNET_CFG_CPU_MIMXRT1052 || FNET_CFG_CPU_MIMXRT1062) & KSZ8081RNB PHY */

    return FNET_OK;
    
}

/* If vector table is in ROM, pre-install FNET ISR for ENET Receive Frame interrupt*/
#if !FNET_CFG_CPU_VECTOR_TABLE_IS_IN_RAM
void ENET_IRQHandler (void)
{
    FNET_ISR_HANDLER();
}
#endif

#endif /* FNET_MIMXRT && FNET_CFG_CPU_ETH0 */
