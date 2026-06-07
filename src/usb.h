/* SPDX-License-Identifier: MIT */
/*******************************************************************************
 * @file usb.h
 * @brief USB core layout declarations, HID reports maps, buffers.
 ******************************************************************************/

#ifndef USB_H
#define USB_H

#include <ch554.h>
#include <stdint.h>   // Fixed-width integer types.
#include <ch554_usb.h>

#define EP0_SIZE 64
#define EP1_SIZE 8 // Keyboard HID Endpoint size

/* Direct Mapping Buffers into xRAM Layout offsets */
extern __xdata __at (0x0000) uint8_t Ep0Buffer[EP0_SIZE];	  
extern __xdata __at (0x0040) uint8_t Ep1Buffer[EP1_SIZE];   

/* USB Stack Primitives */
void USBDeviceInit(void);
void UsbIntFunct(void);

#endif // USB_H