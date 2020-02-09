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

uint16_t set_point = 15.0;
extern uint16_t distanceCM;
extern int count;
extern int measured;
double output;
extern error[2];

int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_16| SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |SYSCTL_OSC_MAIN); //Set up clock to run at 12.5MHz
    //SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    Config_PWM();
    Initialize_HCS04();
    Distance_Measure_Enable();
    IntMasterEnable();

    double idx = 0;
    double max = 150;
    double min = -150;
    double neutral = 0;
    while(1){
       ++count;
       if(measured)
       {
          output = compute_PID(set_point, distanceCM);
          PWM_Set_Servo(PWM0_BASE, PWM_OUT_2, error[0]);
          SysCtlDelay(40000);
          measured = 0;
          Restart_measurement();
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
