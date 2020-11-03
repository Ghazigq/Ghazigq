/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-03     Ghazigq   first version
 */

#include <board.h>
#include <rtthread.h>
#include "drv_common.h"

void system_clock_config(int target_freq_Mhz)
{
    nrf_clock_lf_src_set((nrf_clock_lfclk_t)NRFX_CLOCK_CONFIG_LF_SRC);
    nrfx_clock_lfclk_start();
}

void clk_init(char *clk_source, int source_freq, int target_freq)
{
    system_clock_config(target_freq);
}

