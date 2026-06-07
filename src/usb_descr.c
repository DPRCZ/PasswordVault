/* SPDX-License-Identifier: MIT */
 /*******************************************************************************
 * @file usb_descr.c
 * @brief USB Raw static hardware hardware structure layout vectors.
 ******************************************************************************/

#include "usb.h"

__code uint8_t DevDesc[18] = 
{
    // USB --- Device Descriptor ---
    0x12,               // bLength: Size of this descriptor (18 bytes)
    0x01,               // bDescriptorType: Device Descriptor Type (0x01)
    0x10, 0x01,         // bcdUSB: USB Specification Release Number (USB 1.10)
    0x00,               // bDeviceClass: Class code (Defined at Interface level)
    0x00,               // bDeviceSubClass: Subclass code
    0x00,               // bDeviceProtocol: Protocol code
    EP0_SIZE,          // bMaxPacketSize0: Maximum packet size for Endpoint 0
    0xCD, 0xAB,         // idVendor: Vendor ID (0xABCE)
    0x01, 0x00,         // idProduct: Product ID (0x0001)
    0x00, 0x00,         // bcdDevice: Device release number
    0x00,               // iManufacturer: Index of string descriptor for manufacturer
    0x00,               // iProduct: Index of string descriptor for product
    0x00,               // iSerialNumber: Index of string descriptor for serial number
    0x01                // bNumConfigurations: Number of possible configurations (1)
};

__code uint8_t CfgDesc[9+9+9+7] =
{
    // --- Configuration Descriptor ---
    0x09,               // bLength: Descriptor size (9 bytes)
    0x02,               // bDescriptorType: Configuration Descriptor Type (0x02)
    sizeof(CfgDesc),    // wTotalLength: Total length of data returned for this configuration
    0x00,               // (wTotalLength high byte)
    0x01,               // bNumInterfaces: Number of interfaces supported by this configuration (1)
    0x01,               // bConfigurationValue: Value used to select this configuration
    0x00,               // iConfiguration: Index of string descriptor describing this configuration
    0x80,               // bmAttributes: Bus Powered, No Remote Wakeup
    0x32,               // bMaxPower: Maximum power consumption (50 * 2mA = 100mA)

    // --- Interface 0 Descriptor (Keyboard) ---
    0x09,               // bLength: Descriptor size (9 bytes)
    0x04,               // bDescriptorType: Interface Descriptor Type (0x04)
    0x00,               // bInterfaceNumber: Number of this interface (Interface 0)
    0x00,               // bAlternateSetting: Value used to select this alternate setting
    0x01,               // bNumEndpoints: Number of endpoints used by this interface (1)
    0x03,               // bInterfaceClass: Human Interface Device class (HID = 0x03)
    0x01,               // bInterfaceSubClass: Boot Interface Subclass (1)
    0x01,               // bInterfaceProtocol: Keyboard Protocol (1)
    0x00,               // iInterface: Index of string descriptor for this interface

    // --- HID Class Descriptor (Keyboard) ---
    0x09,               // bLength: Descriptor size (9 bytes)
    0x21,               // bDescriptorType: HID Descriptor Type (0x21)
    0x11, 0x01,         // bcdHID: HID Class Specification release number (1.11)
    0x00,               // bCountryCode: Hardware target country code (0x00 = Not supported)
    0x01,               // bNumDescriptors: Number of HID class descriptors to follow (1)
    0x22,               // bDescriptorType: Report Descriptor Type (0x22)
    sizeof(KeyRepDesc), // wDescriptorLength: Total length of Report descriptor
    0x00,               // (wDescriptorLength high byte)

    // --- Endpoint 1 Descriptor (IN Interrupt) ---
    0x07,               // bLength: Descriptor size (7 bytes)
    0x05,               // bDescriptorType: Endpoint Descriptor Type (0x05)
    0x81,               // bEndpointAddress: Endpoint 1, direction IN (0x81)
    0x03,               // bmAttributes: Transfer Type: Interrupt (0x03)
    EP1_SIZE,          // wMaxPacketSize: Maximum packet size this endpoint is capable of
    0x00,               // (wMaxPacketSize high byte)
    0x0a,               // bInterval: Interval for polling endpoint for data transfers (10 ms)

};

__code uint8_t KeyRepDesc[63] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)

    0x05, 0x07,        // Usage Page (Keyboard)

    // Modifier byte (Ctrl, Shift, Alt...)
    0x19, 0xE0,        // Usage Minimum (Left Ctrl)
    0x29, 0xE7,        // Usage Maximum (Right GUI)
    0x15, 0x00,        // Logical Minimum
    0x25, 0x01,        // Logical Maximum
    0x75, 0x01,        // Report Size = 1 bit
    0x95, 0x08,        // Report Count = 8
    0x81, 0x02,        // Input (Data,Var,Abs)

    // Reserved byte
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,        // Constant

    // 6-key rollover
    0x95, 0x06,        // 6 keys
    0x75, 0x08,        // 8 bits per key
    0x15, 0x00,        // Logical Min
    0x25, 0x65,        // Logical Max (101 keys)
    0x05, 0x07,        // Usage Page (Keyboard)
    0x19, 0x00,        // Usage Minimum
    0x29, 0x65,        // Usage Maximum
    0x81, 0x00,        // Input (Array)

    // LED output (Caps/Num/Scroll)
    0x95, 0x05,        // 5 LEDs
    0x75, 0x01,        // 1 bit each
    0x05, 0x08,        // LED Page
    0x19, 0x01,        // Num Lock
    0x29, 0x05,        // Kana
    0x91, 0x02,        // Output (Data,Var,Abs)

    // LED padding
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,        // Constant padding

    0xC0               // End Collection
};

