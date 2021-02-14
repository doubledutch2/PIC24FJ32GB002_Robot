
/**
  CMP1 Generated Driver File 

  @Company
    Microchip Technology Inc.

  @File Name
    cmp1.c

  @Summary
    This is the generated driver implementation file for the CMP1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides implementations for driver APIs for CMP1. 
    Generation Information : 
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  PIC24FJ32GB002
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB             :  MPLAB X v5.45
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

#include "cmp1.h"

/**
  Section: Driver Interface
*/

void CMP1_Initialize(void)
{   
    
    // CMIDL disabled; 
    CMSTAT = 0x00;
    // CON enabled; CPOL Not Inverted; EVPOL Disabled; COE disabled; CCH Internal selectable reference voltage; CREF CVREF; CEVT disabled; 
    CM1CON = 0x8013 & ~(0x8000);
    

    CMP1_Enable();
}

bool CMP1_OutputStatusGet(void)
{
    return (CM1CONbits.COUT);
}
bool CMP1_EventStatusGet(void)
{   
    return (CM1CONbits.CEVT);      
}
void CMP1_EventStatusReset(void)
{
    CM1CONbits.CEVT = 0;
}

void __attribute__ ((weak)) CMP1_CallBack(void)
{
    // Add your custom callback code here
}

void CMP1_Tasks ( void )
{
	if(IFS1bits.CMIF)
	{
		// CMP1 callback function 
		CMP1_CallBack();

                // Clear the CEVT bit to enable further interrupts
		CMP1_EventStatusReset();
		
		// clear the CMP1 interrupt flag
		IFS1bits.CMIF = 0;
	}
}

/**
  End of File
*/
