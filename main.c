#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
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
#include "PWM_Driver.h"

uint16_t set_point = 13.0;
extern uint16_t distanceCM;
extern uint16_t distanceCM_2;
extern int count;
extern int measured;
extern uint8_t isMainSensor;
double output;
extern int error[2];
extern uint32_t time_now;
extern uint32_t time_prev;
extern uint16_t mean_distance;
uint32_t tick_count;
double output_net;

void Config_SysTick(void)
{
   NVIC_ST_CTRL_R = 0;
   NVIC_ST_CURRENT_R = 0;
   NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R& 0x00FFFFFF)|0xE0000000;
}

void Init_SysTick(void)
{
   NVIC_ST_CTRL_R = 0x00000007;
   NVIC_ST_RELOAD_R = 12500000;
}

void Init_HeartBeatLED(void)
{
    //GPIO_PORTF_PCTL_R &= ~0x00000F00; // 3) regular GPIO
    GPIO_PORTF_AMSEL_R &= ~0x02;      // 4) disable analog function on PA7
    GPIO_PORTF_DIR_R |= 0x02;         // 5) set direction to output
    GPIO_PORTF_AFSEL_R &= ~0x02;      // 6) regular port function
    GPIO_PORTF_DEN_R |= 0x02;         // 7) enable digital port
}

int main(void) {

    SysCtlClockSet(SYSCTL_SYSDIV_16| SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |SYSCTL_OSC_MAIN); //Set up clock to run at 12.5MHz
    //SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    uint32_t clock_rate = SysCtlClockGet();
    Config_PWM();
    Config_SysTick();
    Initialize_HCS04();
    Init_HeartBeatLED();
    Distance_Measure_Enable();
    Config_SysTick();
    IntMasterEnable();
    Init_SysTick();

    double idx = 0;
    double max = 150;
    double min = -150;
    double neutral = 0;
    while(1){
       ++count;
       if(measured)
       {
          if(isMainSensor) {
             output = compute_PID(set_point, distanceCM, tick_count);
             Restart_measurement();
          }
          else
          {
             output = compute_PID(set_point, distanceCM_2, tick_count);
             Restart_measurement_2();
          }
          output_net = 10*(error[0] + error[1]);
          PWM_Set_Servo(PWM0_BASE, PWM_OUT_2, ((error[0] + error[1])));
          SysCtlDelay(1000); //change back to 40000
          measured = 0;
         // Restart_measurement();
         // Restart_measurement_2();
       }
//        int i;
//        for(idx = 0; idx < max; idx+=5){
//           PWM_Set_Servo(PWM0_BASE, PWM_OUT_2, idx);
//           SysCtlDelay(40000);
//        }
//        for(i = 0; i < 5; ++i){
//            SysCtlDelay(2000000);
//        }
//        for(idx = max; idx > min; idx-=5){
//           PWM_Set_Servo(PWM0_BASE, PWM_OUT_2, idx);
//           SysCtlDelay(40000);
//        }
//        for(i = 0; i < 5; ++i){
//            SysCtlDelay(2000000);
//        }

    }
    return 0;
}

void SysTick_Handler(void)
{
   ++tick_count;
   if(tick_count == 0xFFFFFFFF)
   {
      tick_count = 0x00;
   }
   GPIO_PORTF_DATA_R ^= 0x02;
   Restart_measurement();
   Restart_measurement_2();
}
