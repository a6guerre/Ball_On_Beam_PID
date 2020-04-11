/*
 * moving_average.h
 *
 *  Created on: Feb 22, 2020
 *      Author: Max
 */

#ifndef DRIVERS_HC_S04_DISTANCE_SENSOR_MOVING_AVERAGE_H_
#define DRIVERS_HC_S04_DISTANCE_SENSOR_MOVING_AVERAGE_H_

void pop_and_push(uint16_t *position_arr, uint16_t val, int len);

void compute_sum(uint16_t *arr, int *sum, int len);

void print_mean(uint16_t mean);

uint16_t compute_average(uint16_t *position_arr, uint16_t *mean_distance, int len);

void populate_array(uint16_t measured_distance, int sample_count, uint16_t *position_arr, int len);

#endif /* DRIVERS_HC_S04_DISTANCE_SENSOR_MOVING_AVERAGE_H_ */
