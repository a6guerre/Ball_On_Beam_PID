/*
 * moving_average.h
 *
 *  Created on: Feb 22, 2020
 *      Author: Max
 */

#ifndef DRIVERS_HC_S04_DISTANCE_SENSOR_MOVING_AVERAGE_DOUBLE_H_
#define DRIVERS_HC_S04_DISTANCE_SENSOR_MOVING_AVERAGE_DOUBLE_H_

void pop_and_push_double(double *position_arr, double val, int len);

void compute_sum_double(double *arr, int *sum, int len);

double compute_average_double(double *position_arr, int len);

void populate_array_double(double measured_distance, int sample_count, double *position_arr, int len);

#endif /* DRIVERS_HC_S04_DISTANCE_SENSOR_MOVING_AVERAGE_H_ */
