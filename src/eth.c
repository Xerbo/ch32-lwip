/*
 * Copyright 2023 Xerbo, 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ethernet driver for CH32 series chips, this code is based upon:
 * https://github.com/openwch/ch32v307/blob/73b7e5/EVT/EXAM/ETH/ETH_internal_10BASE-T_PHY/User/my_eth_driver.c
 */

#include "eth.h"

#include <string.h>
#include <lwip/etharp.h>

// From ch32vXXX_eth.c
extern ETH_DMADESCTypeDef *DMATxDescToSet;
extern ETH_DMADESCTypeDef *DMARxDescToGet;

extern void usleep(uint32_t time);
static uint32_t link_init(ETH_InitTypeDef *eth, uint16_t phy_address);
static void eth_apply_settings(const ETH_InitTypeDef *eth);

static err_t ch32netif_output(struct netif *netif, struct pbuf *p) {
    (void)netif;
    LINK_STATS_INC(link.xmit);

    if (eth_send_packet(p->payload, p->tot_len) == ETH_ERROR) {
        return ERR_IF;
    }
    return ERR_OK;
}

err_t ch32netif_init(struct netif *netif) {
    netif->linkoutput = ch32netif_output;
    netif->output     = etharp_output;
    netif->mtu        = MAX_ETH_PAYLOAD;
    netif->flags      = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    netif->hostname = "lwip";
    netif->name[0] = 'c';
    netif->name[1] = 'h';

    eth_get_mac(netif->hwaddr);
    ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr);
    netif->hwaddr_len = 6;

    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
        netif->hwaddr[0],
        netif->hwaddr[1],
        netif->hwaddr[2],
        netif->hwaddr[3],
        netif->hwaddr[4],
        netif->hwaddr[5]
    );

    return ERR_OK;
}

void eth_get_mac(uint8_t *mac) {
    const uint8_t *esig_uid = (uint8_t *)0x1FFFF7E8;
    mac[0] = esig_uid[5];
    mac[1] = esig_uid[4];
    mac[2] = esig_uid[3];
    mac[3] = esig_uid[2];
    mac[4] = esig_uid[1];
    mac[5] = esig_uid[0];
}

void eth_configure_clock(void) {
    // 8Mhz/2*15 = 60MHz
    RCC_PREDIV2Config(RCC_PREDIV2_Div2);
    RCC_PLL3Config(RCC_PLL3Mul_15);
    RCC_PLL3Cmd(ENABLE);

    while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == 0);
}

uint32_t eth_init(uint16_t phy_address) {
    // Enable the ethernet MAC
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);
    // Enable the internal 10BASE-T PHY
    EXTEN->EXTEN_CTR |= EXTEN_ETH_10M_EN;

    // Reset MAC
    ETH_DeInit();
    ETH_SoftwareReset();
    for (uint8_t i = 0; i < 100; i++) {
        usleep(10000);
        if ((ETH->DMABMR & ETH_DMABMR_SR) == 0) {
            break;
        }
        if (i == 99) {
            printf("Error: MAC reset timed out\n");
            return ETH_ERROR;
        }
    }

    // Ethernet configuration
    ETH_InitTypeDef eth;
    ETH_StructInit(&eth);
    eth.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    eth.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
    eth.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    eth.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
    eth.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Enable;
    if (link_init(&eth, phy_address) == ETH_ERROR) {
        return ETH_ERROR;
    }

    // Configure interrupts
    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_PHYLINK, ENABLE);
    // Enable them
    NVIC_EnableIRQ(ETH_IRQn);
    return ETH_SUCCESS;
}

uint32_t eth_send_packet(const uint8_t *buffer, uint16_t len) {
    // Check the DMA descriptor isn't owned by the MAC
    // i.e. a transfer is in progress
    if (DMATxDescToSet->Status & ETH_DMATxDesc_OWN) {
        return ETH_ERROR;
    }

    memcpy((uint8_t *)ETH_GetCurrentTxBufferAddress(), buffer, len);

    // Frame length
    DMATxDescToSet->ControlBufferSize = len & ETH_DMATxDesc_TBS1;
    // This is the only segment, therefore first and last
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;
    // Give ownership back to the MAC
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;
    // Enable automatic CRC generation
    DMATxDescToSet->Status |= ETH_DMATxDesc_CIC_TCPUDPICMP_Full;

    // If the unavailable flag is set, reset it and resume transmission
    if (ETH->DMASR & ETH_DMASR_TBUS) {
        ETH->DMASR = ETH_DMASR_TBUS;
        ETH->DMATPDR = 0;
    }

    // Get the next DMA descriptor from the ring
    DMATxDescToSet = (ETH_DMADESCTypeDef *)DMATxDescToSet->Buffer2NextDescAddr;

    return ETH_SUCCESS;
}

uint32_t eth_get_packet(uint8_t **buffer, uint16_t *len) {
    // Check the DMA descriptor isn't owned by the MAC
    // i.e. a transfer is in progress
    if (DMARxDescToGet->Status & ETH_DMARxDesc_OWN) {
        // If the unavailable flag is set, reset it
        if (ETH->DMASR & ETH_DMASR_RBUS) {
            ETH->DMASR = ETH_DMASR_RBUS;
            ETH->DMARPDR = 0;
        }
        return ETH_ERROR;
    }

    // Make sure this is the only segment and no errors occurred
    uint32_t status = DMARxDescToGet->Status;
    if ((status & ETH_DMARxDesc_LS) && (status & ETH_DMARxDesc_FS) && (status & ETH_DMARxDesc_ES) == 0) {
        // minus 4 bytes because of CRC
        *len = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> 16) - 4;
        *buffer = (uint8_t *)DMARxDescToGet->Buffer1Addr;
    } else {
        return ETH_ERROR;
    }

    // Give ownership back to the MAC
    DMARxDescToGet->Status |= ETH_DMARxDesc_OWN;
    // Grab the next DMA descriptor from the ring
    DMARxDescToGet = (ETH_DMADESCTypeDef *)DMARxDescToGet->Buffer2NextDescAddr;

    return ETH_SUCCESS;
}

static uint32_t link_init(ETH_InitTypeDef *eth, uint16_t phy_address) {
    // Set MAC frequency (sets CR to 0b000)
    ETH->MACMIIAR &= MACMIIAR_CR_MASK;

    // Reset PHY
    ETH_WritePHYRegister(phy_address, PHY_BMCR, PHY_Reset);
    for (uint8_t i = 0; i < 100; i++) {
        usleep(10000);
        if ((ETH_ReadPHYRegister(phy_address, PHY_BMCR) & PHY_Reset) == 0) {
            break;
        }
        if (i == 99) {
            printf("Error: PHY reset timed out\n");
            return ETH_ERROR;
        }
    }

    eth_apply_settings(eth);
    return ETH_SUCCESS;
}

static void eth_apply_settings(const ETH_InitTypeDef *eth) {
    // Hash list
    ETH->MACHTHR = eth->ETH_HashTableHigh;
    ETH->MACHTLR = eth->ETH_HashTableLow;

    // MAC control
    ETH->MACCR &= MACCR_CLEAR_MASK;
    ETH->MACCR |= (eth->ETH_Watchdog |
                   eth->ETH_Jabber |
                   eth->ETH_InterFrameGap |
                   eth->ETH_CarrierSense |
                   eth->ETH_Speed |
                   eth->ETH_LoopbackMode |
                   eth->ETH_Mode |
                   eth->ETH_ChecksumOffload |
                   eth->ETH_AutomaticPadCRCStrip |
                   eth->ETH_RetryTransmission |
                   eth->ETH_BackOffLimit |
                   eth->ETH_DeferralCheck |
                   ETH_Internal_Pull_Up_Res_Enable);

    // MAC frame filter
    ETH->MACFFR = (eth->ETH_ReceiveAll |
                   eth->ETH_SourceAddrFilter |
                   eth->ETH_PassControlFrames |
                   eth->ETH_BroadcastFramesReception |
                   eth->ETH_DestinationAddrFilter |
                   eth->ETH_PromiscuousMode |
                   eth->ETH_MulticastFramesFilter |
                   eth->ETH_UnicastFramesFilter);

    // MAC flow control
    ETH->MACFCR &= MACFCR_CLEAR_MASK;
    ETH->MACFCR |= ((eth->ETH_PauseTime << 16) |
                     eth->ETH_ZeroQuantaPause |
                     eth->ETH_PauseLowThreshold |
                     eth->ETH_UnicastPauseFrameDetect |
                     eth->ETH_ReceiveFlowControl |
                     eth->ETH_TransmitFlowControl);

    // VLAN tag
    ETH->MACVLANTR = (eth->ETH_VLANTagComparison |
                      eth->ETH_VLANTagIdentifier);

    // DMA operation mode
    ETH->DMAOMR &= DMAOMR_CLEAR_MASK;
    ETH->DMAOMR |= (eth->ETH_DropTCPIPChecksumErrorFrame |
                    eth->ETH_ReceiveStoreForward |
                    eth->ETH_FlushReceivedFrame |
                    eth->ETH_TransmitStoreForward |
                    eth->ETH_TransmitThresholdControl |
                    eth->ETH_ForwardErrorFrames |
                    eth->ETH_ForwardUndersizedGoodFrames |
                    eth->ETH_ReceiveThresholdControl |
                    eth->ETH_SecondFrameOperate);

    // DMA bus mode
    ETH->DMABMR = (eth->ETH_AddressAlignedBeats |
                   eth->ETH_FixedBurst |
                   eth->ETH_RxDMABurstLength |
                   eth->ETH_TxDMABurstLength |
                   (eth->ETH_DescriptorSkipLength << 2) |
                   eth->ETH_DMAArbitration |
                   ETH_DMABMR_USP);
}
