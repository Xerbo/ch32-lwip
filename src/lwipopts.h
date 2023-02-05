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

#ifndef LWIPOPTS_H_
#define LWIPOPTS_H_

// Basic config
#define NO_SYS 1
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0
#define LWIP_NOASSERT 1

// Memory
#define MEM_SIZE (16 * 1024)
#define MEM_ALIGNMENT 4

// NETIF
#define LWIP_NETIF_HOSTNAME 1

// Checksums, all done in hardware :)
#define CHECKSUM_GEN_IP      0
#define CHECKSUM_GEN_UDP     0
#define CHECKSUM_GEN_TCP     0
#define CHECKSUM_GEN_ICMP    0
#define CHECKSUM_GEN_ICMP6   0
#define CHECKSUM_CHECK_IP    0
#define CHECKSUM_CHECK_UDP   0
#define CHECKSUM_CHECK_TCP   0
#define CHECKSUM_CHECK_ICMP  0
#define CHECKSUM_CHECK_ICMP6 0

// HTTP server returns garbage without this
#define HTTP_IS_DATA_VOLATILE(hs) TCP_WRITE_FLAG_COPY

#endif
