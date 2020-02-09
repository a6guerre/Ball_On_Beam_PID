/*
 * PWM_Driver.c
 *
 *  Created on: Dec 16, 2019
 *      Author: Max
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/pwm.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/debug.h"


#define SERVO_DRIVE_FREQ 50
#define SERVO_DRIVE_PER 0.02
#define SERVO_NEUTRAL 273
#define SERVO_MAX 351
#define SERVO_MIN 117
uint32_t ui32PWM_Freq;
uint32_t ui32ServoLoad;

void Config_PWM(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);  // We need less resolution since servo needs 50 Hz driving.
    GPIOPinConfigure(GPIO_PB4_M0PWM2);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN); //PB4 Clock 1
    ui32PWM_Freq = SysCtlClockGet()*0.015625;
    ui32ServoLoad = ui32PWM_Freq*SERVO_DRIVE_PER;
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1,  ui32ServoLoad);
    double duty_cycle = (0.0006/.02);
    uint32_t ui32DutyCycle_load = duty_cycle*ui32ServoLoad;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, ui32DutyCycle_load);

    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}

void PWM_Set_Servo(uint32_t pwm_base, uint32_t pwm_out, double delta_Theta)
{
    if(delta_Theta > 0)
    {
       if(delta_Theta > 98)
       {
           PWMPulseWidthSet(pwm_base, pwm_out, SERVO_MAX);

       } else
       {
           PWMPulseWidthSet(pwm_base, pwm_out, SERVO_NEUTRAL + delta_Theta);
       }

    }
    else
    {
       if(delta_Theta < -156){
          PWMPulseWidthSet(pwm_base, pwm_out, SERVO_MIN);
       }
       else{
          PWMPulseWidthSet(pwm_base, pwm_out, SERVO_NEUTRAL + delta_Theta);
       }
    }
}


