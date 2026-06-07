/* SPDX-License-Identifier: MIT */
/*******************************************************************************
 * @file hardware.h
 * @brief Hardware configuration, Timers and GPIOs management.
 ******************************************************************************/

#ifndef HARDWARE_H
#define HARDWARE_H

#include <ch554.h>
#include <stdint.h>   // Fixed-width integer types.

//////////////////////////////////////////////////////////////////////////////////
/* Pin Definitions */
#define SW_PIN  3   // Button Pin P3.3
#define LED_PIN 4   // LED Pin P3.4

SBIT(SW,  0xB0, SW_PIN);
SBIT(LED, 0xB0, LED_PIN);


//////////////////////////////////////////////////////////////////////////////////

/* LED Modes */
typedef enum {
    LED_MODE_LIGHT = 0,
    LED_MODE_READY,
    LED_MODE_BUTTON
} LED_MODE;

/* Global Time & Button Flags (Exported for other modules) */
extern volatile uint8_t led_mode;
extern volatile uint8_t blink_divider;

extern __bit is_usb_ready;
extern __bit button_short_event;

/* Peripheral and System Management */
void CfgFsys(void);
void Hardware_Init(void);
void Timer0_Init(void);
void delay_ms(uint16_t ms);
void UpdateLED(void);
void Timer0IntFunct(void);

#endif // HARDWARE_H