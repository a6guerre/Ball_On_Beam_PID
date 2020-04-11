/*
 * moving_average.c
 *
 *  Created on: Feb 22, 2020
 *      Author: Max
 */

#include <stdint.h>
#include "moving_average_double.h"

void pop_and_push_double(double *position_arr, double val, int len)
{
   int i;
   for (i = 0; i < len - 1; ++i)
   {
      position_arr[i + 1] = position_arr[i];
   }
   position_arr[0] = val;

}

void compute_sum_double(double *arr, int *sum, int len)
{
   int i;
   *sum = 0;
   for(i = 0;  i < len; ++i)
   {
      *sum += arr[i];
   }
}

double compute_average_double(double *position_arr, int len)
{
   double sum;
   compute_sum(position_arr, &sum, len);
   double mean = (double)(sum/len);
   //print_mean(mean);
   return (sum/len);
}

void populate_array_double(double measured_distance, int sample_count, double *position_arr, int len)
{
   if (sample_count < (len + 1))
   {
     position_arr[sample_count - 1] = measured_distance;
   }
   else
   {
      pop_and_push_double(position_arr, measured_distance, len);
   }
}
