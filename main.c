/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  PIC24FJ32GB002
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
 * 
*/

/**
  Section: Included Files
*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/mcc.h"

/*
 * Timings below can be increased for debugging but all 3 works fine with the Hitachi Display
 * and are all in MilliSeconds.
 * 
 * This is all managed by TMR_1
*/

#define TIME_WAIT_BEFOR_E_HIGH  3       //  Time to wait after the Data Bits are set before the E Signal
#define TIME_E_HIGH             3       //  Time the E_Signal should be high fore
#define TIME_E_LOW              3       //  Time the E_Signal should be low before it goes high again (after pulling E Down)
#define TIME_BOOT_DELAY         3       //  Time it takes for the display to initialize
/*
 * These two procedure implement my Timer Logic
 * My_ISR / TMR_1 fires every 1 ms
 * 
 * - myTimers counts those ticks
 * - waitingForTimer = 1 means we haven't reached the target state
 * - timerDelayMilliSec = the time we are waiting to reach
 */

int16_t     myTimer             =   0;
int         waitingForTimer     =   0;
int16_t     timerDelayMilliSec  =   0;
#define     INVERSE_PULSE       1

void    timerDelay(int16_t waitTime) {
    myTimer             =   0;
    waitingForTimer     =   1;
    timerDelayMilliSec  =   waitTime;
    while (waitingForTimer) {}
    return;
}

/*
 * Pulse Width Modulation for most Servo's work at 50Hz 
 * which is 50 times per second. So 1 Sec / 50 = 0.02 Seconds = 20 Milli Second per Period
 * TMR_2 fires every (0.0001 Seconds) so you need 200 of those to get 1 Period
 * TMR_3 Controls the Pulse within this 20Ms period
 * To read more about PWM: https://dronebotworkshop.com/servo-motors-with-arduino/
 */
#define PWM_Period               200 // 20 Milli Second Cycle


int16_t     pwmPeriodCounter    =   0;  //  Tick to reach PWM_Period
uint16_t    defaultPulseWidth   =   0;  //  The default Pulse With as the Timer is configured
                                        //  used to calculate the new value based on the POT setting
uint16_t    nextPulseWidth      =   0;  //  The next Pulse Width - based on the POT setting

/*
 * This timer times out onNextPulseWidth and takes the PWM_Pin low. It defines the Duty Cycle/Pulse Width 
 */
void My_ISR_TMR3(void) {
    
    TMR3_Stop();
    if (INVERSE_PULSE) {
        PWM_PIN_SetHigh();
    }
    else {
        PWM_PIN_SetLow();
    }
}

/*
 * This timer increases the pwmPeriodCounter and if we need to start a new period
 * set the PWM Pin high (until the TMR3 runs out)
 */
void My_ISR_TMR2(void) {
    pwmPeriodCounter++;
    if ((pwmPeriodCounter > PWM_Period) || (pwmPeriodCounter == 0))  {
        pwmPeriodCounter = 0;
        if (INVERSE_PULSE) {
            PWM_PIN_SetLow(); 
        }
        else {
            PWM_PIN_SetHigh();
        }
        TMR3_Start();
    }    
}

/*
 See description above this time is mainly for managing the display
 */
void My_ISR_TMR1(void) {
    
    myTimer++;
    
    if (waitingForTimer) {
        if (myTimer > timerDelayMilliSec) {
            myTimer         = 0;
            waitingForTimer = 0;
        }
    }
    
}

/*
 * Timings to Display a Character - code says it all
 */

void displayNibble(         int     dataRegister,
                            char    character) {
    
    
    //  Switch all lines to Low
    DISPLAY_E_SetLow();
    
    //  Is this a command or a display character?
    if (dataRegister) {
        DISPLAY_RS_SetHigh();
    } else
    {
        DISPLAY_RS_SetLow();
    }
    
    //  Set the Data bits
    if (character & (1 << 0)) {
        DISPLAY_D4_SetHigh();
    }
    else {
        DISPLAY_D4_SetLow();
    }
    
    if (character & (1 << 1)) {
        DISPLAY_D5_SetHigh();
    }
    else {
        DISPLAY_D5_SetLow();
    }
    if (character & (1 << 2)) {
        DISPLAY_D6_SetHigh();
    }
    else {
        DISPLAY_D6_SetLow();
    }
    if (character & (1 << 3)) {
        DISPLAY_D7_SetHigh();
    }
    else {
        DISPLAY_D7_SetLow();
    }
       
    //  Wait for long enough for the Data bits to be high and present
    timerDelay(TIME_WAIT_BEFOR_E_HIGH);
    DISPLAY_E_SetHigh();
    
    //  Switch E Line up for the right amount of time
    timerDelay(TIME_E_HIGH);
    DISPLAY_E_SetLow();
    
    //  Makes sure the eLine stays down for long enough
    timerDelay(TIME_E_LOW);
    
    DISPLAY_D4_SetLow();
    DISPLAY_D5_SetLow();
    DISPLAY_D6_SetLow();
    DISPLAY_D7_SetLow();
    
    return;
}
/*
 * We use the Display in 4 Bit mode so we are sending 2 Nibbles, individually
 */
void displayCharacter ( int  dataRegister,
                        char character) {
    //  Upper Nibble First, Lower Second
    displayNibble(  dataRegister,(character >> 4));
    displayNibble(  dataRegister,character);
}

/*
 * Put a display string on the display on a particular row/column
 */
void displayString (int row,
                    int col,
                    char *strToDisplay) {
    
    int i = 0;
    char    myChar = 0x00;
    //  Look at Datasheet: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
    //  Page 8,9,10
    char    myPos = 0x80 | (row * 0x40) | col;   
    displayCharacter(0,myPos);   
    for (i=0;i<strlen(strToDisplay);i++) {
        myChar = strToDisplay[i];
        displayCharacter(1,myChar);
    }
    return;
}

/*
 * Main Code
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    myTimer             = 0;
    TMR1_SetInterruptHandler(My_ISR_TMR1);
    // TMR1_Start();    //  Not necessary if you enable it in MCC
    
    pwmPeriodCounter = -1;
    TMR2_SetInterruptHandler(My_ISR_TMR2);

    defaultPulseWidth = TMR3_Period16BitGet();
    TMR3_SetInterruptHandler(My_ISR_TMR3);

    //  Initial Delay to wait for the Display to get ready
    
    timerDelay(TIME_BOOT_DELAY);

    //  Lower all signals
    
    DISPLAY_E_SetLow();
    DISPLAY_RS_SetLow();
    DISPLAY_D4_SetLow();
    DISPLAY_D5_SetLow();
    DISPLAY_D6_SetLow();
    DISPLAY_D7_SetLow();
    
    displayNibble(0,0x02);      //  0010 - Lower bit Only - put in 4 bit mode
    displayCharacter(0,0x2C);   //  0010$1100 (4 Bit Mode, 2 Line display)
    displayCharacter(0,0x2C);   //  0010$1100 (4 Bit Mode, 2 Line display)
    displayCharacter(0,0x0F);   //  0000$1111 (Display On - Blinking Cursor)
    displayCharacter(0,0x01);   //  Clear Display    //displayNibble(0,0x00000001);      //  0010 - Lower bit Only - put in 4 bit mode
    
    char        strToDisplay[21];  
    uint16_t    potValue;
    
    double potPercentage    = 0;    //  Percentage pot is turned from 0 - 100
    int oldPotPercentage    = 0;    //  last Percentage to avoid having to change TMR_3 all the tiem
    double   tmpDouble      = 0;
    int i                   = 0;
    
    displayString(0,0,"Pot Value");
    
    while (1) {
        //  Logic to check the A2D port and get the value from the Pot
        ADC1_Initialize();
        ADC1_ChannelSelect(POT_AN0);
        ADC1_Start();
        //Provide Delay
        for(i=0;i <1000;i++)
        {
        }
        ADC1_Stop();
        while(!ADC1_IsConversionComplete())
        {
            ADC1_Tasks();   
        }
        potValue = ADC1_ConversionResultGet();
        
        //  converted Pot Value can be between 0 - 1023
        //  So this calculates the percentage it should be above TMR_1 (which is set to 1Ms)
        //  500 on Pot will give 149%
        //  So 149% of 1ms = 1.49ms
        
        potPercentage = 100 + (potValue / 10.23); 
                                                  
        //  Update PR3, the TMR_3 timeout if necessary
        //  Remember defaultPulseWidth is the value as set by MCC
        
        if (oldPotPercentage != potPercentage) {
            oldPotPercentage = potPercentage;
            tmpDouble = (defaultPulseWidth * (potPercentage / 100));
            nextPulseWidth = (int) tmpDouble;
            PR3 = nextPulseWidth;
        }
        //  Display some stuff on the display and start again
        sprintf(strToDisplay,"%05d %05d %03f",defaultPulseWidth,nextPulseWidth,potPercentage);
        displayString(1,0,strToDisplay);
    }
    
    return 1;
}
/**
 End of File
*/

