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
#define TIME_DATA_BITS_HIGH     2000
#define TIME_WAIT_BEFOR_E_HIGH  1000
#define TIME_E_HIGH             500
#define TIME_E_LOW              4000
#define TIME_BOOT_DELAY         5000
*/
/* */
#define TIME_DATA_BITS_HIGH     3
#define TIME_WAIT_BEFOR_E_HIGH  3
#define TIME_E_HIGH             3
#define TIME_E_LOW              3
#define TIME_BOOT_DELAY         3

#define PWM_Cycle               100 // 20 Milli Second Cycle
#define PWM_TicksPerMs          5

/* */
/*
 * These two procedure implement my Timer Logic
 * My_ISR fires every 1 ms
 * 
 * - myTimers counts those ticks
 * - waitingForTimer = 1 means we haven't reached the target state
 * - timerDelayMilliSec = the time we are waiting to reach
 */

int16_t     myTimer             =   0;
int16_t     pwmTimer            =   0;
int         pwmSignal           =   0;
double      pwmDutyCycle        =   0;
int         pwmDutyCycleCount   =   0;
int         waitingForTimer     =   0;
int16_t     timerDelayMilliSec  =   0;
uint16_t    dutyCycleTimerValue =   0;

void    timerDelay(int16_t waitTime) {
    myTimer             =   0;
    waitingForTimer     =   1;
    timerDelayMilliSec  =   waitTime;
    while (waitingForTimer) {}
    return;
}
/*
Main application
 */
//  
//  Fires every 100us (20 * = 20 Ms) (PWM)
/*void My_ISR_TMR3(void) {
    pwmDutyCycleCount++;
    if (pwmDutyCycleCount > pwmDutyCycle) {
        pwmDutyCycleCount=0;
        PWM_PIN_SetLow();
    }
}
*/
void My_ISR_TMR2(void) {
    pwmTimer++;
    if (pwmTimer > PWM_Cycle) {
        pwmDutyCycleCount   = 0;
        pwmTimer = 0;
        PWM_PIN_SetHigh();
        pwmDutyCycle = 4;
    }

    pwmDutyCycleCount++;
    if (pwmDutyCycleCount > pwmDutyCycle) {
        PWM_PIN_SetLow();
    }

    
}
void My_ISR_TMR1(void) {
    /*
    myTimer++;
    pwmDutyCycleCount++;
    if (pwmDutyCycleCount > 1) {
        PWM_PIN_SetLow();
    }
    */
    if (waitingForTimer) {
        if (myTimer > timerDelayMilliSec) {
            myTimer         = 0;
            waitingForTimer = 0;
        }
    }
    
}

/*
 * Timings to Display a Character
 * - Put Data on Data Bus
 * - Wait 200ns to make sure it's present and stable
 * - Pull line E up for at least 450 nano Seconds
 * - Pull line E down for at least 500 nano Seconds
 */

void displayNibble(         int     dataRegister,
                            char    character) {
    
    
    //  Switch all lines to Low
    DISPLAY_E_SetLow();
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
    
    //DISPLAY_D5_SetHigh();
    //DISPLAY_D6_SetHigh();
    //DISPLAY_D7_SetHigh();
    
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

void displayString (int row,
                    int col,
                    char *myString) {
    
    int i = 0;
    char    myChar = 0x00;
    char    myPos = 0x80 | (row * 0x40) | col;
    displayCharacter(0,myPos);   
    for (i=0;i<strlen(myString);i++) {
        myChar = myString[i];
        displayCharacter(1,myChar);
    }
    return;
}

int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    TMR1_SetInterruptHandler(My_ISR_TMR1);
    TMR1_Start();
    pwmTimer = 0;
    TMR2_SetInterruptHandler(My_ISR_TMR2);
    TMR2_Start();
/*    TMR3_SetInterruptHandler(My_ISR_TMR3);
    TMR3_Start();
    dutyCycleTimerValue = TMR3_Period16BitGet();    //  Set Initial Value
    */
    myTimer             = 0;
    // int displayState    = 0;

    //  Initial Delay to wait for the Display to get ready
    
    timerDelay(TIME_BOOT_DELAY);

    //  Lower all signals
    
    DISPLAY_E_SetLow();
    DISPLAY_RS_SetLow();
    DISPLAY_D4_SetLow();
    DISPLAY_D5_SetLow();
    DISPLAY_D6_SetLow();
    DISPLAY_D7_SetLow();
    //  dataRegister, Row, Column, Character
    //  put display in 4 bit mode
    
    //  Init sequence from: http://web.alfredstate.edu/faculty/weimandn/lcd/lcd_initialization/lcd_initialization_index.html
    //  If you don't do this - re-running the pick doesn't initialize the display properly
    /*
    displayNibble(0,0b0011);
    displayNibble(0,0b0011);
    displayNibble(0,0b0011);
    displayNibble(0,0b0010);
    displayCharacter(0,0b00101000);
    displayCharacter(0,0b00001000);
    displayCharacter(0,0b00000001);
    displayCharacter(0,0b00000110);
    */
    
    displayNibble(0,0x02);      //  0010 - Lower bit Only - put in 4 bit mode
    displayCharacter(0,0x2C);   //  0010$1100 (4 Bit Mode, 2 Line display)
    displayCharacter(0,0x2C);   //  0010$1100 (4 Bit Mode, 2 Line display)
    displayCharacter(0,0x0F);   //  0000$1111 (Display On - Blinking Cursor)
    displayCharacter(0,0x01);   //  Clear Display    //displayNibble(0,0x00000001);      //  0010 - Lower bit Only - put in 4 bit mode
    
    //  Move Cursor (see CGRAM Address)
    /* */
    /*
    unsigned char    myPos = 0x80 | (0 * 0x40) | 0;
    char    myString[21];
    sprintf(myString,"myPos %02x",myPos);
    displayString(0,0,myString);
     */
    /*
    displayCharacter(0,0x80);   //  Line One, Col 0
    displayCharacter(1,'L');    //  Display Something
    displayCharacter(1,'e');    //  Display Something
    displayCharacter(1,'o');    //  Display Something
    displayCharacter(1,'n');    //  Display Something
     
    char    myString[21];
    strcpy(myString,"Julie de Beer");
     */
    // displayString(0,2,"Hello World");
    // displayString(1,0,"Bye Bye World");

    /*     //  Move Cursor (see CGRAM Address)
    displayCharacter(0,0xC0);   //  Line One, Col 0
    displayCharacter(1,'d');    //  Display Something
    displayCharacter(1,'e');    //  Display Something
    displayCharacter(1,' ');    //  Display Something
    displayCharacter(1,'B');    //  Display Something
    displayCharacter(1,'e');    //  Display Something
    displayCharacter(1,'e');    //  Display Something
    displayCharacter(1,'r');    //  Display Something
    */
    //  Switch all the LED's off
    
    /*
    DISPLAY_E_SetLow();
    DISPLAY_RS_SetLow();
    DISPLAY_D4_SetLow();
    DISPLAY_D5_SetLow();
    DISPLAY_D6_SetLow();
    DISPLAY_D7_SetLow();
    */
    
    char        myString[21];
    uint16_t    potValue;
    displayString(0,0,"Pot Value");
    /*
    while (1)
    {
        ADC1_Initialize();
        potValue=ADC1_GetConversion(POT_AN0);
        sprintf(myString,"%04d",potValue);
        displayString(1,0,myString);
        // Add your application code
    }
    */
    int i   = 0;
    int potPercentage = 0;
    while (1) {
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
        potPercentage = (potValue / 10.23) * PWM_TicksPerMs;
        pwmDutyCycle  = PWM_TicksPerMs + ((PWM_TicksPerMs * potPercentage) / 100);
        // pwmDutyCycle = pwmDutyCycle + PWM_TicksPerMs;
        sprintf(myString,"%04d - %.02f",potValue,pwmDutyCycle/PWM_TicksPerMs);
        displayString(1,0,myString);
    }
    
    return 1;
}
/**
 End of File
*/

