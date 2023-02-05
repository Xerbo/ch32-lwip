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

#include <stdint.h>

#include "ch32v30x_conf.h"
#include "arch/cc.h"

// Standard system calls (here to shut up the linker)
void _close() { }
void _fstat() { }
void _getpid() { }
void _isatty() { }
void _kill() { }
void _lseek() { }
void _read() { }

// Use instead of Delay_Us
void usleep(uint32_t time) {
    uint64_t end = SysTick->CNT + (uint64_t)(time*18);
    while (SysTick->CNT < end);
}

// LwIP
sys_prot_t sys_arch_protect(void) {
    return 1;
}

void sys_arch_unprotect(sys_prot_t pval) {
    (void)pval;
}

uint32_t sys_now(void) {
    return SysTick->CNT/18000;
}
