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

#ifndef ETH_H_
#define ETH_H_

#include <debug.h>
#include <lwip/netif.h>

void eth_get_mac(uint8_t *mac);
void eth_configure_clock(void);
uint32_t eth_init(uint16_t phy_address);

uint32_t eth_send_packet(const uint8_t *buffer, uint16_t len);
uint32_t eth_get_packet(uint8_t **buffer, uint16_t *len);

// LwIP driver
err_t ch32netif_init(struct netif *netif);

#endif
