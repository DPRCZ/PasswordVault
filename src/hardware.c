/* SPDX-License-Identifier: MIT */
/*******************************************************************************
 * @file hardware.c
 * @brief Hardware configuration, Timers, GPIOs, and DataFlash management.
 ******************************************************************************/

#include "hardware.h"

#define BUTTON_DEBOUNCE  3     // 3 * 20ms = 60ms

volatile uint8_t led_mode = LED_MODE_LIGHT;

volatile uint8_t blink_divider = 0;
volatile uint16_t button_hold_counter = 0;
volatile uint8_t button_debounce = 0;
static __bit button_raw = 1;

__bit button_short_event = 0;

static __bit is_button_pressed = 0;
static __bit is_button_stable = 0;


/**
 * @brief Configure System Clock to 24MHz
 */
void CfgFsys(void) {
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_SEL) | 0x06; // 24MHz internal RC
    SAFE_MOD = 0x00;
}

/**
 * @brief Init IO port LED and SWITCH
 */
void Hardware_Init(void) {

    /* Initialize LED Pin (P3.4) as Push-Pull Output */
    P3_MOD_OC &= ~(1 << LED_PIN); 
    P3_DIR_PU |= (1 << LED_PIN);  
 
    /* Initialize Switch Pin (P3.3) with Pull-up Enabled */
    P3_MOD_OC |=  (1 << SW_PIN);
    P3_DIR_PU |=  (1 << SW_PIN);
    P3 |= (1 << SW_PIN);
}

/**
 * @brief Initialize Timer 0 for 20ms periodic interrupts
 */
void Timer0_Init(void) {
    TMOD &= ~MASK_T0_MOD;       // Clear existing T0 modes
    TMOD |= bT0_M0;             // Set Timer 0 to Mode 1 (16-bit)
    
    TH0 = 0x63;                 // Preload for 20ms at 24MHz
    TL0 = 0x90;
    
    ET0 = 1;                    // Enable Timer 0 Interrupt
    TR0 = 1;                    // Start Timer 0
}

/**
 * @brief Software delay implementation
 */
void delay_ms(uint16_t ms) {
    uint16_t i;
    while(ms--) {
        for(i = 0; i < 4000; i++) {
            __asm nop __endasm;
        }
    }
}


/**
 * @brief Handle LED indicators based on current mode
 */
void UpdateLED(void) {
    switch(led_mode) {
        case LED_MODE_LIGHT:
            LED = 1; // Solid ON 
            break;

        case LED_MODE_READY:
            if(blink_divider >= 25) { // 1Hz slow blink
                blink_divider = 0;
                LED = !LED;
            }
            break;

        case LED_MODE_BUTTON:
            LED = 0; // Solid active during active button press
            break;
    }
}


/**
 * @brief Timer 0 ISR (Handles Clock ticking, Button Debounce, and Timeouts)
 */

void Timer0IntFunct(void){
    TH0 = 0x63;
    TL0 = 0x90;

    blink_divider++;

    /* Button Debounce Logic */
    /* --- Button Debounce & Filter Logic --- */
    button_raw = SW;

    if(button_raw != is_button_stable) {
        button_debounce++;
        if(button_debounce >= BUTTON_DEBOUNCE) {
            is_button_stable = button_raw;
            button_debounce = 0;
            
            if(is_button_stable == 0) { // Button Pressed
              is_button_pressed = 1;
              button_hold_counter = 0;
              led_mode = LED_MODE_BUTTON;            
             } else { // Button Released
                  is_button_pressed = 0;
                  button_short_event = 1; 
                  if(is_usb_ready) led_mode = LED_MODE_READY;
		            else led_mode = LED_MODE_LIGHT;
           }
        }
    } else {
        button_debounce = 0;
    }
}