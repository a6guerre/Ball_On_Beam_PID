/*
 * timing_driver.c
 *
 *  Created on: Feb 17, 2020
 *      Author: Max
 */
#include "inc/tm4c123gh6pm.h"
#include <stdint.h>

#define CLOCK_VALUE (12500000)

uint32_t get_time(uint32_t tick_count)
{
    uint32_t delta_t = CLOCK_VALUE - NVIC_ST_CURRENT_R;
    return (delta_t + tick_count*CLOCK_VALUE);
}

/*
 * Returns change in time from last call of this function to current.
 * Parameters: time in units of clock ticks
 * Returns: Change in time in units of milliseconds
 */
double get_delta_time(uint32_t time_now, uint32_t time_prev)
{
    return ((double)time_now - (double)time_prev);
}
