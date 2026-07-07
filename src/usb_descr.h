/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 DPRCZ     */ 
/*******************************************************************************
 * @file usb_descr.h
 * @brief USB Descriptors.
 ******************************************************************************/

#ifndef USB_DESCR_H
#define USB_DESCR_H

#include <ch554.h>
#include <stdint.h>   // Fixed-width integer types.

/* Descriptor references built globally */
extern __code uint8_t DevDesc[18];
extern __code uint8_t CfgDesc[9+9+9+7];
extern __code uint8_t KeyRepDesc[63];
extern __code uint8_t ConfigRepDesc[21];

#endif // USB_DESCR_H