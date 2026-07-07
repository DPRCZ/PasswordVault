/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 DPRCZ     */ 
/*******************************************************************************
 * @file usb_hid_keyboard.h
 * @brief USB HID Keyboard declarations, keycodes, and LED state bitmasks
 ******************************************************************************/

#ifndef USB_HID_KEYBOARD_H
#define USB_HID_KEYBOARD_H

#include <ch554.h>
#include <compiler.h> 
#include <stdint.h>   
#include <ch554_usb.h>

/* USB HID LED State Bitmasks (received via SET_REPORT Output) */
#define NUM_LOCK_LED    1
#define CAPS_LOCK_LED   2
#define SCROLL_LOCK_LED 4

/* Standard USB HID Usage IDs for Keyboard Layout Control */
#define HID_KEY_CAPSLOCK 0x39
#define HID_KEY_NUMLOCK  0x53
#define HID_KEY_ENTER    0x28

/* Global variable holding the current keyboard LED status bits */
extern uint8_t KeyLockLed;

/**
 * @brief Sends a single HID key report (Press and Release)
 */
void SendKey(uint8_t hid_code, uint8_t shift);

/**
 * @brief Loops through the password string and sends characters using SendKey.
 */
void SendPassword(const uint8_t *password);

#endif /* USB_HID_KEYBOARD_H */