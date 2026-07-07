/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 DPRCZ     */ 
/*******************************************************************************
 * @file main.c
 * @brief Entry point for the CH552 USB Password Token Application.
 ******************************************************************************/

#include "hardware.h"
#include "usb.h"
#include "usb_hid_keyboard.h"


void Timer0_ISR(void) __interrupt (INT_NO_TMR0) 
{
  Timer0IntFunct();
}

void DeviceInterrupt(void) __interrupt (INT_NO_USB)
{
  UsbIntFunct();
}

/* Example password transmitted by the HID token. */
const uint8_t password[] = "kP7$mX9!vF2#zL5*"; 

void main(void) {

    /* Initialize System and Peripherals */
    CfgFsys();                                         
    delay_ms(5);         
                                                    
    /* Initialize IO Pins */
    Hardware_Init();

    /* Initialize USB Device Subsystem and Hardware Timer */
    USBDeviceInit();   
    Timer0_Init();
    
    led_mode = LED_MODE_LIGHT;
    EA = 1; // Globally Enable Interrupts	
   
  
    /* Main Execution Workloop */
    while(1) {
        UpdateLED();
       
        /* Execution path active only if Time Base Synchronized */
        if (is_usb_ready) {
            if(button_short_event) {
                button_short_event = 0;
                SendPassword(password);
            }
        }
    }
}