/*
 * PID_driver.c
 *
 *  Created on: Dec 23, 2019
 *      Author: Max
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

double k[] = {4,5};
double error[2];
uint16_t current_error, last_error;
int count;

double compute_PID(uint16_t set_point, uint16_t current_value)
{
   ++count;
   error[0] = k[0]*((double)set_point - (double)current_value);
   current_error = error[0];
   // compute d term
   last_error = current_error;
//   if(count%5== 0){
//      UART_OutUDec((unsigned long)error[0]);
//      UART_OutChar('\n');
//      count = 0;
//   }
   return error[0];
}
