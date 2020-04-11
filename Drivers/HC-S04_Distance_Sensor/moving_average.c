/*
 * moving_average.c
 *
 *  Created on: Feb 22, 2020
 *      Author: Max
 */

#include <stdint.h>
#include "UART_Driver.h"

void pop_and_push(uint16_t *position_arr, uint16_t val, int len)
{
   int i;
   for (i = 0; i < len - 1; ++i)
   {
      position_arr[i + 1] = position_arr[i];
   }
   position_arr[0] = val;

}

void compute_sum(uint16_t *arr, int *sum, int len)
{
   int i;
   *sum = 0;
   for(i = 0;  i < len; ++i)
   {
      *sum += arr[i];
   }
}

void print_mean(uint16_t mean)
{
  if(mean == 0)
  {
      UART_OutUDec(mean);
      UART_OutChar('\n');
  }
  else{
      UART_OutUDec(mean);
      UART_OutChar('\n');
  }
}

uint16_t compute_average(uint16_t *position_arr, uint16_t *mean_distance, int len)
{
   int sum;
   compute_sum(position_arr, &sum, len);
   uint16_t mean = (uint16_t)(sum/len);
   if(mean == 0)
   {
       int k;
       k = 1;
   }
   //print_mean(mean);
   *mean_distance = mean;
   print_mean(*mean_distance);
   return (uint16_t)(sum/len);
}

void populate_array(uint16_t measured_distance, int sample_count, uint16_t *position_arr, int len)
{
   if (sample_count < (len + 1))
   {
     position_arr[sample_count - 1] = measured_distance;
   }
   else
   {
      pop_and_push(position_arr, measured_distance, len);
   }
}
