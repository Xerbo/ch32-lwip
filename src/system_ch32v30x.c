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
 * Based upon https://github.com/openwch/ch32v307/blob/main/EVT/EXAM/GPIO/GPIO_Toggle/User/system_ch32v30x.c
 */

#include "system_ch32v30x.h"

#include <ch32v30x_rcc.h>

uint32_t SystemCoreClock = 0;

void set_sysclk_pll(uint16_t mul, uint16_t div) {
    // Enable HSE
    RCC->CTLR |= RCC_HSEON;
    while((RCC->CTLR & RCC_HSERDY) == 0);

    // Configure clock paths    
    RCC->CFGR0 = RCC_HPRE_DIV1  | // HCLK = SYSCLK
                 RCC_PPRE1_DIV1 | // API1 = HCLK
                 RCC_PPRE2_DIV1 | // APB2 = HCLK
                 RCC_PLLSRC_HSE | // PLLSRC = HSE
                 (mul & RCC_PLLMULL) |
                 (div & RCC_PLLXTPRE);

    // Enable PLL
    RCC->CTLR |= RCC_PLLON;
    while((RCC->CTLR & RCC_PLLRDY) == 0);

    // SYSCLK = PLLCLK
    RCC->CFGR0 &= ~RCC_SW;
    RCC->CFGR0 = RCC_SW_PLL;
    while ((RCC->CFGR0 & RCC_SWS) != RCC_SWS_PLL);
}

void SystemInit(void) {
    RCC->CTLR |= (uint32_t)0x00000001;

#ifdef CH32V30x_D8C
    RCC->CFGR0 &= (uint32_t)0xF8FF0000;
#else
    RCC->CFGR0 &= (uint32_t)0xF0FF0000;
#endif

    RCC->CTLR &= (uint32_t)0xFEF6FFFF;
    RCC->CTLR &= (uint32_t)0xFFFBFFFF;
    RCC->CFGR0 &= (uint32_t)0xFF80FFFF;

#ifdef CH32V30x_D8C
    RCC->CTLR &= (uint32_t)0xEBFFFFFF;
    RCC->INTR = 0x00FF0000;
    RCC->CFGR2 = 0x00000000;
#else
    RCC->INTR = 0x009F0000;   
#endif

#ifdef CH32V30x_D8C
    set_sysclk_pll(RCC_PLLMULL18_EXTEN, RCC_PLLXTPRE_HSE);
#else
    set_sysclk_pll(RCC_PLLMULL18, RCC_PLLXTPRE_HSE);
#endif
    SystemCoreClock = HSE_VALUE*18;
}
