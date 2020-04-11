/*
 * PID_driver.c
 *
 *  Created on: Dec 23, 2019
 *      Author: Max
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "moving_average_double.h"

double k[] = {2.9, 0.06}; //2.8, 0.0425 smoothest so far
double error[2];

double current_error, prev_error;
uint32_t time_now;
uint32_t time_prev;
double dt;
double delta_error;
int count;
double mean_dev_error;    // filtered derivative term
double dev_error_arr[500]; // array for filtering derivative

double compute_PID(uint16_t set_point, uint16_t current_value, uint32_t tick_count)
{
   ++count;
   error[0] = k[0]*((double)set_point - (double)current_value);
   current_error = error[0];
   // compute derivative error term
   time_now = get_time(tick_count);
   delta_error = current_error - prev_error;
   dt = ((double)time_now - (double)time_prev)/(12500000);
   error[1] = k[1]*(delta_error/dt);

   int len = sizeof(dev_error_arr)/sizeof(double);
   //populate_array_double(error[1], count, dev_error_arr, len);
   if (count > len && 0){
     //mean_dev_error = compute_average_double(dev_error_arr, len);
     //error[1] = mean_dev_error;
     //if( error[1] != 0)
    // {
    //    error[1] = error[1];
    // }
   }
   time_prev = time_now;
   prev_error = current_error;

   return error[0];
}
