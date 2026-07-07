/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 DPRCZ     */ 
/*******************************************************************************
 * @file  usb_hid_keyboard.c
 * @brief USB HID Keyboard simulation and key emulation
 ******************************************************************************/

#include "hardware.h"
#include "usb.h"
#include "usb_hid_keyboard.h"
#include <string.h>

/* Standard USB HID Modifier bitmask for Left Shift */
#define KEY_MOD_LSHIFT 0x02

/**
 * @brief Sends a single HID key report (Press and Release)
 * @param hid_code Standard USB HID Usage ID for the key
 * @param shift    Set to 1 to apply Left Shift modifier, 0 otherwise
 */
void SendKey(uint8_t hid_code, uint8_t shift) {
    /* 1. Send Key Press Report */
    memset(Ep1Buffer, 0, EP1_SIZE);
    
    if (shift) {
        Ep1Buffer[0] = KEY_MOD_LSHIFT; /* Activate Shift in the modifier byte */
    }
    Ep1Buffer[2] = hid_code; /* Set the primary HID key usage code */
    
    UEP1_T_LEN = EP1_SIZE;
    UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
    while ((UEP1_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK);
    
    /* 2. Send Key Release Report */
    memset(Ep1Buffer, 0, EP1_SIZE); /* Completely clear the report (includes Shift) */
    UEP1_T_LEN = EP1_SIZE;
    UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
    while ((UEP1_CTRL & MASK_UEP_T_RES) == UEP_T_RES_ACK);
}

/**
 * @brief Converts a printable ASCII character to its USB HID code and Shift status.
 * @note  Optimized for US Keyboard Layout and minimum flash footprint.
 */
static void ascii_to_hid_us(uint8_t ascii, uint8_t *hid_code, uint8_t *shift) {
    *shift = 0;
    *hid_code = 0;

    /* 1. Lowercase letters 'a' - 'z' (ASCII 97 - 122) -> HID 0x04 - 0x1D */
    if (ascii >= 'a' && ascii <= 'z') {
        *hid_code = 0x04 + (ascii - 'a');
        return;
    }

    /* 2. Uppercase letters 'A' - 'Z' (ASCII 65 - 90) -> HID 0x04 - 0x1D + Shift */
    if (ascii >= 'A' && ascii <= 'Z') {
        *shift = 1;
        *hid_code = 0x04 + (ascii - 'A');
        return;
    }

    /* 3. Numeric Keypad numbers '1' - '9' -> HID 0x59 - 0x61, '0' -> HID 0x62 */
    if (ascii >= '1' && ascii <= '9') {
        *hid_code = 0x59 + (ascii - '1');
        return;
    }
    if (ascii == '0') { 
        *hid_code = 0x62; 
        return; 
    }

    /* 4. Special cases: Spacebar */
    if (ascii == ' ') { 
        *hid_code = 0x2C; 
        return; 
    }
    
    /* 5. Special characters above numbers via Shift: !, @, #, $, %, ^, &, *, (, ) */
    if (ascii >= '!' && ascii <= ')') {
        *shift = 1;
        if (ascii == '!') { *hid_code = 0x1E; return; } /* Shift + 1 */
        if (ascii == '@') { *hid_code = 0x1F; return; } /* Shift + 2 */
        if (ascii == '#') { *hid_code = 0x20; return; } /* Shift + 3 */
        if (ascii == '$') { *hid_code = 0x21; return; } /* Shift + 4 */
        if (ascii == '%') { *hid_code = 0x22; return; } /* Shift + 5 */
        if (ascii == '^') { *hid_code = 0x23; return; } /* Shift + 6 */
        if (ascii == '&') { *hid_code = 0x24; return; } /* Shift + 7 */
        if (ascii == '*') { *hid_code = 0x25; return; } /* Shift + 8 */
        if (ascii == '(') { *hid_code = 0x26; return; } /* Shift + 9 */
        if (ascii == ')') { *hid_code = 0x27; return; } /* Shift + 0 */
    }

    /* 6. Punctuation and symbols (grouped by physical keys on US Layout) */
    switch (ascii) {
        /* Key - _ */
        case '-': *hid_code = 0x2D; break;
        case '_': *shift = 1; *hid_code = 0x2D; break;
        
        /* Key = + */
        case '=': *hid_code = 0x2E; break;
        case '+': *shift = 1; *hid_code = 0x2E; break;
        
        /* Key [ { */
        case '[': *hid_code = 0x2F; break;
        case '{': *shift = 1; *hid_code = 0x2F; break;
        
        /* Key ] } */
        case ']': *hid_code = 0x30; break;
        case '}': *shift = 1; *hid_code = 0x30; break;
        
        /* Key \ | */
        case '\\': *hid_code = 0x31; break;
        case '|':  *shift = 1; *hid_code = 0x31; break;
        
        /* Key ; : */
        case ';': *hid_code = 0x33; break;
        case ':': *shift = 1; *hid_code = 0x33; break;
        
        /* Key ' " */
        case '\'': *hid_code = 0x34; break;
        case '"':  *shift = 1; *hid_code = 0x34; break;
        
        /* Key ` ~ */
        case '`': *hid_code = 0x35; break;
        case '~': *shift = 1; *hid_code = 0x35; break;
        
        /* Key , < */
        case ',': *hid_code = 0x36; break;
        case '<': *shift = 1; *hid_code = 0x36; break;
        
        /* Key . > */
        case '.': *hid_code = 0x37; break;
        case '>': *shift = 1; *hid_code = 0x37; break;
        
        /* Key / ? */
        case '/': *hid_code = 0x38; break;
        case '?': *shift = 1; *hid_code = 0x38; break;

        default:  *hid_code = 0;    break; /* Unknown or unprintable character */
    }
}

/**
 * @brief Loops through the password string and sends characters using SendKey.
 * @note  Ensures Caps Lock is OFF and Num Lock is ON during transmission,
 * then restores the original keyboard LED states.
 */
void SendPassword(const uint8_t *password) {
    uint8_t i = 0;
    uint8_t hid_code;
    uint8_t shift;
    
    /* Cache the initial lock status to safely restore it later */
    uint8_t initial_state = KeyLockLed;

    /* Adjust keyboard state if necessary (Turn OFF Caps Lock, Turn ON Num Lock) */
    if (initial_state & CAPS_LOCK_LED)   SendKey(HID_KEY_CAPSLOCK, 0);
    if (!(initial_state & NUM_LOCK_LED)) SendKey(HID_KEY_NUMLOCK, 0);
    
    /* Send password characters */
    while (password[i] != '\0') {
        ascii_to_hid_us(password[i], &hid_code, &shift);
        
        if (hid_code != 0) {
            SendKey(hid_code, shift);
        }
        i++;
    }
    
    /* Submit password with Enter key if the string wasn't empty */
    if (i) {
        SendKey(HID_KEY_ENTER, 0);
    }
    
    /* Restore the host keyboard to its initial LED state */
    if (initial_state & CAPS_LOCK_LED)   SendKey(HID_KEY_CAPSLOCK, 0);
    if (!(initial_state & NUM_LOCK_LED)) SendKey(HID_KEY_NUMLOCK, 0);
}