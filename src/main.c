/*
 * Copyright 2023 Xerbo
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
 */

#include <debug.h>
#include <lwip/init.h>
#include <lwip/pbuf.h>
#include <lwip/timeouts.h>
#include <netif/ethernet.h>
#include <lwip/apps/httpd.h>

#include "eth.h"

#define INTERRUPT(name) __attribute__((interrupt("WCH-Interrupt-fast"))) void name(void)
#define PHY_ADDRESS 1
#define ETH_RX_RING_SIZE 6
#define ETH_TX_RING_SIZE 2
#define UART_BAUDRATE 115200

__attribute__((aligned(4))) ETH_DMADESCTypeDef eth_dma_rx[ETH_RX_RING_SIZE];
__attribute__((aligned(4))) ETH_DMADESCTypeDef eth_dma_tx[ETH_TX_RING_SIZE];
__attribute__((aligned(4))) uint8_t eth_buffer_rx[ETH_RX_RING_SIZE][ETH_MAX_PACKET_SIZE];
__attribute__((aligned(4))) uint8_t eth_buffer_tx[ETH_TX_RING_SIZE][ETH_MAX_PACKET_SIZE];
struct pbuf *p = NULL;
volatile int have_frame = 0;
volatile int link_status_update = 0;

INTERRUPT(NMI_Handler) {
    printf("Something bad happened");
}

INTERRUPT(HardFault_Handler) {
    printf("Something very bad happened");
    while(1);
}

INTERRUPT(ETH_IRQHandler) {
    // Receive
    if (ETH_GetDMAITStatus(ETH_DMA_IT_R)) {
        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
        if (have_frame == 0) {
            uint8_t *buffer;
            uint16_t length;
            if (eth_get_packet(&buffer, &length) == ETH_ERROR) {
                return;
            }

            p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
            pbuf_take(p, buffer, length);
            have_frame = 1;
        }
    }

    // Link status
    if (ETH_GetDMAITStatus(ETH_DMA_IT_PHYLINK)) {
        ETH_DMAClearITPendingBit(ETH_DMA_IT_PHYLINK);
        link_status_update = 1;
    }

    // Normal interrupt
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
}

int main(void) {
    // Enable SysTick with HCLK/8
    // This will overflow every 32475 years, give or take
    SysTick->CTLR = 1;

    USART_Printf_Init(UART_BAUDRATE);

    eth_configure_clock();
    eth_init(PHY_ADDRESS);
    ETH_DMARxDescChainInit(eth_dma_rx, &eth_buffer_rx[0][0], ETH_RX_RING_SIZE);
    ETH_DMATxDescChainInit(eth_dma_tx, &eth_buffer_tx[0][0], ETH_TX_RING_SIZE);
    ETH_Start();

    lwip_init();
    httpd_init();
    struct netif netif;
    ip_addr_t address = IPADDR4_INIT_BYTES(192, 168, 1,   10);
    ip_addr_t gateway = IPADDR4_INIT_BYTES(192, 168, 1,   1);
    ip_addr_t netmask = IPADDR4_INIT_BYTES(255, 255, 255, 0);
    netif_add(&netif, &address, &netmask, &gateway, NULL, &ch32netif_init, &ethernet_input);
    netif_set_default(&netif);
    netif_set_up(&netif);

    while (1) {
        if (link_status_update) {
            link_status_update = 0;
            if (ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BMSR) & PHY_Linked_Status) {
                netif_set_link_up(&netif);
                printf("Link up ");

                uint32_t mode;
                if (ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BMCR) & (1 << 8)) {
                    mode = ETH_Mode_FullDuplex;
                    printf("full-duplex\n");
                } else {
                    mode = ETH_Mode_HalfDuplex;
                    printf("half-duplex\n");
                }

                // Send auto negotiated values to the MAC
                ETH->MACCR &= ~0x0000C800;
                ETH->MACCR |= mode | ETH_Speed_10M;
            } else {
                netif_set_link_up(&netif);
                printf("Link down\n");
            }
        }
        if (have_frame) {
            LINK_STATS_INC(link.recv);
            if (netif.input(p, &netif) != ERR_OK) {
                pbuf_free(p);
            }
            have_frame = 0;
        }

        sys_check_timeouts();
    }
}
