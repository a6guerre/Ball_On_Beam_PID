#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/tm4c123gh6pm.h"
#include "HC-S04_Driver.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "moving_average.h"
#include "UART_Driver.h"

uint16_t hi;
uint16_t pulseWidth = 0; // Needs to be this data type to be congruent with register values.
uint16_t pulseWidth_2 = 0; // Needs to be this data type to be congruent with register values.
uint16_t risingEdge;
uint16_t distanceCM;
uint16_t distanceCM_2;
uint16_t position_arr[100];
uint16_t mean_distance;

uint8_t  isMainSensor = 1;

unsigned int overFlowCount;
unsigned int risingEdgeMeasured = 0;
int measured = 0;
int count;
static int sample_count;

void TriggerSignalEnable(){
    SYSCTL_RCGC2_R |= 0x01;
    GPIO_PORTA_PCTL_R &= ~0x00000F00; // 3) regular GPIO
    GPIO_PORTA_AMSEL_R &= ~0x80;      // 4) disable analog function on PA7
    GPIO_PORTA_DIR_R |= 0x80;        // 5) set direction to output
    GPIO_PORTA_AFSEL_R &= ~0x80;      // 6) regular port function
    GPIO_PORTA_DEN_R |= 0x80;         // 7) enable digital port
}

void TriggerSignalEnableRedundant(){
    GPIO_PORTA_PCTL_R &= ~0x0F000000; // 3) regular GPIO
    GPIO_PORTA_AMSEL_R &= ~0x40;      // 4) disable analog function on PA7
    GPIO_PORTA_DIR_R |= 0x40;        // 5) set direction to output
    GPIO_PORTA_AFSEL_R &= ~0x40;      // 6) regular port function
    GPIO_PORTA_DEN_R |= 0x40;         // 7) enable digital port
}

void EnableEdgeModeTimers(){
    SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER0;
    //Enable Input capture pins CCP0 and CCP1
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB + SYSCTL_RCGC2_GPIOF;
    GPIO_PORTB_DEN_R |= 0x80;
    GPIO_PORTB_AFSEL_R |= 0x80;
    GPIO_PORTB_DEN_R |= 0x40;
    GPIO_PORTB_AFSEL_R |= 0x40;
    GPIO_PORTB_PCTL_R |= 0x77000000;

    TIMER0_CTL_R &= ~0x101;
    TIMER0_CFG_R  = 0x00000004;
    TIMER0_TAMR_R = 0x00000017;
    TIMER0_TBMR_R = 0x00000017;
    TIMER0_CTL_R =  (TIMER0_CTL_R &~0x00C) + 0x04; // Timer A is falling
    TIMER0_CTL_R &= ~0xC00;                        // Timer B is rising edge
    TIMER0_IMR_R |= 0x404;
    TIMER0_ICR_R |= 0x404;
    NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000;  // Set up NVIC for Timers since be using interrupt driven
    NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFFFF00)|0x00000040;
    NVIC_EN0_R |= 0x0180000;
}

void EnableEdgeModeTimersRedundant(){
    SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER3;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB;//Enable Input capture pins CCP0 and CCP1

    GPIO_PORTB_DEN_R |= 0x04;
    GPIO_PORTB_AFSEL_R |= 0x04;
    GPIO_PORTB_DEN_R |= 0x08;
    GPIO_PORTB_AFSEL_R |= 0x08;
    GPIO_PORTB_PCTL_R |= 0x00007700;

    TIMER3_CTL_R &= ~0x101;
    TIMER3_CFG_R  = 0x00000004;
    TIMER3_TAMR_R = 0x00000017;
    TIMER3_TBMR_R = 0x00000017;
    TIMER3_CTL_R =  (TIMER3_CTL_R &~0x00C) + 0x04; // Timer3A is falling (PB2)
    TIMER3_CTL_R &= ~0xC00;                        // Timer3B is rising edge (PB3)
    TIMER3_IMR_R |= 0x404;
    TIMER3_ICR_R |= 0x404;
    NVIC_PRI8_R = (NVIC_PRI8_R&0x00FFFFFF)|0x40000000;  // Set up NVIC for Timers3A&B since be using interrupt driven
    NVIC_PRI9_R = (NVIC_PRI9_R&0xFFFFFF00)|0x00000040;
    NVIC_EN1_R |= 0x18;
}

void Timer1AB_Init(void){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER1;   // activate timer1
  TIMER1_CTL_R &= ~0x00000101;             // disable timer1A and timer 1B during setup
  TIMER1_CFG_R =  0x00000004;              // configure for 16-bit timer mode
  TIMER1_TAMR_R = 0x00000001;              // configure for one shot
  TIMER1_TBMR_R = 0x00000001;              // Configure as a one shot timer
  TIMER1_TAILR_R =  100;                   // The "period" of the trigger signal, (could try to make it smaller)
  TIMER1_TBILR_R =  10;                    // So that trigger signal is at least 10usec
  TIMER1_TAPR_R =   1;                     // No Prescaler
  TIMER1_TBPR_R =   1;
  TIMER1_ICR_R =  0x00000101;               // clear timer1A and timer 1B timeout flag
  TIMER1_IMR_R |= 0x00000101;               // arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R & NVIC_PRI5_TIME1AB_MASK)|0x00404000; // priority 2 for both timers
  NVIC_EN0_R |= 0x00600000;                 // enable interrupt 19 in NVIC
  TIMER1_CTL_R |= 0x00000101;               // enable timer1A and timer 1B
  GPIO_PORTA_DATA_R |= 0x80;                // Enable trigger signal
}

void Timer2AB_Init(void){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER2;   // activate timer1
  TIMER2_CTL_R &= ~0x00000101;             // disable timer1A and timer 1B during setup
  TIMER2_CFG_R =  0x00000004;              // configure for 16-bit timer mode
  TIMER2_TAMR_R = 0x00000001;              // configure for one shot
  TIMER2_TBMR_R = 0x00000001;              // Configure as a one shot timer
  TIMER2_TAILR_R =  100;                   // The "period" of the trigger signal, (could try to make it smaller)
  TIMER2_TBILR_R =  10;                    // So that trigger signal is at least 10usec
  TIMER2_TAPR_R =   1;                     // No Prescaler
  TIMER2_TBPR_R =   1;
  TIMER2_ICR_R =  0x00000101;               // clear timer1A and timer 1B timeout flag
  TIMER2_IMR_R |= 0x00000101;               // arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R & 0x00FFFFFF)|0x30000000; // priority 2 for both timers
  NVIC_PRI6_R = (NVIC_PRI6_R & 0xFFFFFF00)|0x00000030; // priority 2 for both timers
  NVIC_EN0_R |= 0x01800000;                 // enable interrupt 19 in NVIC
  GPIO_PORTA_DATA_R |= 0x40;                // Enable trigger signal
}

void Distance_Measure_Enable()
{
   TIMER1_CTL_R |= 0x00000101;               // enable timer1A and timer 1B, trigger signal generators
}

void Restart_measurement()
{
   TIMER0_CTL_R &= ~0x00000001;      // Input capture until trigger signal enable again.
   TIMER1_CTL_R |= 0x01;             //  "Schedule" start trigger after TIME1A Timeout
}

void Restart_measurement_2()
{
   TIMER3_CTL_R &= ~0x00000001;      // Input capture until trigger signal enable again.
   TIMER2_CTL_R |= 0x01;             //  "Schedule" start trigger after TIME2A Timeout
}


void Timer0A_Handler(void){
   ++sample_count;
   int len;
   if(!risingEdgeMeasured){
      TIMER0_ICR_R |= TIMER_ICR_CAECINT; // Acknowledge timer 0A capture flag
   }
   else{
      TIMER0_ICR_R = TIMER_ICR_CAECINT; // Acknowledge timer 0A Interrupt
      TIMER0_CTL_R &= ~0x00000100;      //Disable Timer0B Interrupts
      TIMER3_CTL_R &= ~0x00000101;      //Disable Timer0B and Timer0A Interrupts
      TIMER2_CTL_R &= ~0x00000101;
      pulseWidth = ((TIMER0_TAR_R&0xFFFF) - (TIMER0_TBR_R&0xFFFF));
      distanceCM = (0.5*pulseWidth*CLOCK_TICK*SPEED_SOUND);
      measured = 1;
      if(distanceCM < 7)
      {
          //distanceCM = 2;
      }
      else if(distanceCM < 5)
      {
         // distanceCM = 0;
      }
      else if(distanceCM > 60)
      {
          distanceCM = 0;
      }
      else if((distanceCM < 60) && (distanceCM > 30))
      {
        distanceCM = 40;
      }
      len = sizeof(position_arr)/sizeof(uint16_t);
       // populate_array(distanceCM, sample_count, position_arr, len); // compute average to reduce noise
      if (sample_count > len) {
        //compute_average(position_arr, &mean_distance, len);
      }
      risingEdgeMeasured = 0;
      if(distanceCM > 14)
      {
         //TIMER2_CTL_R |=  0x00000101;  // Use other sensor for data.
         //TIMER1_CTL_R &= ~0x00000101;
         TIMER3_CTL_R &= ~0x00000101;
        // UART_OutUDec((uint16_t)distanceCM);
         Restart_measurement();
         Restart_measurement_2();
         TIMER3_CTL_R |= 0x00000101;
         isMainSensor = 0;
         return;
      }
      isMainSensor = 1;

      if(count%5== 0 && measured && distanceCM > 0 && distanceCM < 2000){
         if(1){
           UART_OutUDec((uint16_t)distanceCM);
           UART_OutChar('\n');
         }
         count = 0;
      }
      overFlowCount = 0;

   }
   TIMER3_CTL_R |= 0x00000101;
}

void Timer3A_Handler(void){

   if(!risingEdgeMeasured){
      TIMER3_ICR_R |= TIMER_ICR_CAECINT; // Acknowledge timer 0A capture flag
   }
   else{
      TIMER3_ICR_R = TIMER_ICR_CAECINT; // Acknowledge timer 0A Interrupt
      TIMER3_CTL_R &= ~0x00000100;      //Disable Timer3B Interrupts
      TIMER0_CTL_R &= ~0x00000101;      //Disable Timer0B/0A Interrupts
      TIMER1_CTL_R &= ~0x00000101;
      pulseWidth_2 = ((TIMER3_TAR_R&0xFFFF) - (TIMER3_TBR_R&0xFFFF));
      distanceCM_2 = (0.5*pulseWidth_2*CLOCK_TICK*SPEED_SOUND);
      measured = 1;
     if(distanceCM_2 < 7)
     {
         // distanceCM_2 = 2;
      }
      else if(distanceCM_2 < 5)
      {
         // distanceCM = 0;
      }
      else if(distanceCM_2 > 60)
      {
          distanceCM_2 = 0;
      }
      else if((distanceCM_2 < 60) && (distanceCM_2 > 30))
      {
        //distanceCM_2 = 40;
      }
     if(distanceCM_2 > 35)
      {
          TIMER0_CTL_R |= 0x00000101;      // Re-Enable Timer0B/0A Interrupts
          Restart_measurement_2();
          Restart_measurement();
          isMainSensor = 1;
          return;
      }
      isMainSensor = 0;
      distanceCM_2 = 2*15 - distanceCM_2;  // Symmetry
      risingEdgeMeasured = 0;
      if(count%5== 0 && measured && distanceCM_2 > 0 && distanceCM_2 < 2000){
         if(1){
           UART_OutUDec((uint16_t)distanceCM_2);
           UART_OutChar('\n');
         }
         count = 0;
      }
      overFlowCount = 0;
   }
   TIMER0_CTL_R |= 0x00000101;      // Re-Enable imer0B/0A Interrupts
}

void Timer0B_Handler(void){
    risingEdgeMeasured = 1;
    TIMER0_ICR_R |= 0x00000400; //acknowledge timer 0B capture flag
}

void Timer3B_Handler(void)
{
    risingEdgeMeasured = 1;
    TIMER3_ICR_R |= 0x00000400; //acknowledge timer 3B capture flag
}

void Timer1A_Handler(void){
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;
    GPIO_PORTA_DATA_R = 0x80;
    TIMER1_CTL_R |= 0x00000100; // Re enable one shot timer
}

void Timer1B_Handler(void){
    IntMasterDisable();
    TIMER1_ICR_R = TIMER_ICR_TBTOCINT;
    GPIO_PORTA_DATA_R &= ~0x80;
    TIMER0_TAV_R &= ~0xFFFF;          // Clear the bottom 15:0 bits
    TIMER0_TBV_R &= ~0xFFFF;
    TIMER1_CTL_R &= ~0x00000001;      // Disable timer interrupts right after we trigger
    TIMER0_CTL_R |=  0x00000101;      // enable timer0A and timer 0B
    GPIO_PORTA_DATA_R &= ~0x80;
    IntMasterEnable();
}

void Timer2B_Handler(void){
    IntMasterDisable();
    TIMER2_ICR_R = TIMER_ICR_TBTOCINT;
    GPIO_PORTA_DATA_R &= ~0x40;
    TIMER3_TAV_R &= ~0xFFFF;          // Clear the bottom 15:0 bits
    TIMER3_TBV_R &= ~0xFFFF;
    TIMER2_CTL_R &= ~0x00000001;      // Disable timer interrupts right after we trigger
    TIMER3_CTL_R |=  0x00000101;      // enable timer3A and timer 3B
    GPIO_PORTA_DATA_R &= ~0x40;
    IntMasterEnable();
}

void Timer2A_Handler(void){
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
    GPIO_PORTA_DATA_R = 0x40;
    TIMER2_CTL_R |= 0x00000100; // Re enable one shot timer
}

void Initialize_HCS04(void)
{
    EnableEdgeModeTimers();
    EnableEdgeModeTimersRedundant();
    UART_Init();
    UART1_Init();
    TriggerSignalEnable();
    TriggerSignalEnableRedundant();
    Timer1AB_Init();
    Timer2AB_Init();
}
