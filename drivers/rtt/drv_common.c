/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-03     Ghazigq   first version
 */

#include "drv_common.h"
#include "board.h"

#define OSTICK_CLOCK_HZ  ( 32768UL )
#define COUNTER_MAX 0x00ffffff
#define CYC_PER_TICK (OSTICK_CLOCK_HZ / RT_TICK_PER_SECOND)
#define MAX_TICKS ((COUNTER_MAX - CYC_PER_TICK) / CYC_PER_TICK)
#define MIN_DELAY 32

#define TICK_RATE_HZ  RT_TICK_PER_SECOND

#define NRF_RTC_REG        NRF_RTC1
/* IRQn used by the selected RTC */
#define NRF_RTC_IRQn       RTC1_IRQn

static uint32_t last_count;

static uint32_t counter_sub(uint32_t a, uint32_t b)
{
    return (a - b) & COUNTER_MAX;
}

static void set_comparator(uint32_t cyc)
{
    nrf_rtc_cc_set(NRF_RTC_REG, 0, cyc & COUNTER_MAX);
}

void RTC1_IRQHandler(void)
{
    uint32_t t, dticks, next;

    NRF_RTC_REG->EVENTS_COMPARE[0] = 0;

    t = nrf_rtc_counter_get(NRF_RTC_REG);
    dticks = counter_sub(t, last_count) / CYC_PER_TICK;
    last_count += dticks * CYC_PER_TICK;
    next = last_count + CYC_PER_TICK;

    if ((int32_t)(next - t) < MIN_DELAY)
    {
        next += CYC_PER_TICK;
    }
    set_comparator(next);

    rt_tick_increase();
}

uint32_t _timer_cycle_get_32(void)
{
    // k_spinlock_key_t key = k_spin_lock(&lock);
    uint32_t ret = counter_sub(nrf_rtc_counter_get(NRF_RTC_REG), last_count) + last_count;

    // k_spin_unlock(&lock, key);
    return ret;
}

/* SysTick configuration */
void rt_hw_systick_init(void)
{
    nrf_rtc_prescaler_set(NRF_RTC_REG, 0);

    nrf_rtc_cc_set(NRF_RTC_REG, 0, CYC_PER_TICK);
    nrf_rtc_event_enable(NRF_RTC_REG, RTC_EVTENSET_COMPARE0_Msk);
    nrf_rtc_int_enable(NRF_RTC_REG, RTC_INTENSET_COMPARE0_Msk);

    /* Clear the event flag and possible pending interrupt */
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
    NVIC_ClearPendingIRQ(NRF_RTC_IRQn);

    NVIC_SetPriority(NRF_RTC_IRQn, 1);
    NVIC_EnableIRQ(NRF_RTC_IRQn);

    nrf_rtc_task_trigger(NRF_RTC_REG, NRF_RTC_TASK_CLEAR);
    nrf_rtc_task_trigger(NRF_RTC_REG, NRF_RTC_TASK_START);

    set_comparator(nrf_rtc_counter_get(NRF_RTC_REG) + CYC_PER_TICK);
}


/**
 * This function will delay for some us.
 *
 * @param us the delay time of us
 */
RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{

}


#ifdef RT_USING_FINSH
#include <finsh.h>
static void reboot(uint8_t argc, char **argv)
{
    rt_hw_cpu_reset();
}
FINSH_FUNCTION_EXPORT_ALIAS(reboot, __cmd_reboot, Reboot System);
#endif /* RT_USING_FINSH */
