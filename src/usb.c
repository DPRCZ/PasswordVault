/* SPDX-License-Identifier: MIT */
/*******************************************************************************
 * @file usb.c
 * @brief USB core layout declarations, HID reports maps, buffers.
 ******************************************************************************/

#include "usb.h"
#include "usb_descr.h"
#include "hardware.h"
#include <string.h>

/* Extracted standard definitions */
#define UsbSetupBuf ((PUSB_SETUP_REQ)Ep0Buffer)

__xdata __at (0x0000) uint8_t Ep0Buffer[EP0_SIZE];	  
__xdata __at (0x0040) uint8_t Ep1Buffer[EP1_SIZE];   

volatile uint8_t KeyLockLed;
volatile uint8_t SetupReq, IndexReq, SetupLen, UsbConfig;

const uint8_t *pDescr;

__bit is_usb_ready=0;

/**
 * @brief Native USB Hardware Engine initialization setup
 */
void USBDeviceInit(void) {
    IE_USB = 0;                                     // Disable USB interrupt globally during initialization
    USB_CTRL = 0x00;                                // Reset USB control register
    
    // --- Endpoint 0 Configuration (Control Endpoint) ---
    UEP0_DMA = (uint16_t)Ep0Buffer;                 // Set DMA buffer address for Endpoint 0
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;      // OUT (Rx): ACK incoming setups/data | IN (Tx): NAK until data is ready
    
    // --- Endpoint 1 Configuration (e.g., HID IN) ---
    UEP1_DMA = (uint16_t)Ep1Buffer;                 // Set DMA buffer address for Endpoint 1
    UEP4_1_MOD = bUEP1_TX_EN;                       // Enable EP1 TX (IN), keep EP1 RX and EP4 fully disabled
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;      // Enable hardware auto-toggle for DATA0/DATA1 | IN (Tx): NAK by default

    // --- Unused Endpoints Configuration ---
    UEP2_CTRL = UEP_R_RES_STALL | UEP_T_RES_STALL;  // STALL any unexpected host requests on EP2
    UEP3_CTRL = UEP_R_RES_STALL | UEP_T_RES_STALL;  // STALL any unexpected host requests on EP3

    // --- USB Hardware Engine Activation ---
    USB_DEV_AD = 0x00;                              // Set default USB device address to 0 (Unconfigured state)
    UDEV_CTRL = bUD_PD_DIS;                         // Disable internal pull-down resistors on DP/DM lines
    
    // Enable internal pull-up resistor (signals device attachment), enable DMA, and prevent interrupt nesting
    USB_CTRL = bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN; 
    UDEV_CTRL |= bUD_PORT_EN;                       // Enable the USB port physical interface
    
    // --- Interrupt Handling Setup ---
    USB_INT_FG = 0xFF;                              // Clear any stale/historical interrupt flags
    // Enable specific USB interrupts: Bus Suspend, Successful Transfer, and Bus Reset
    USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST; 
    IE_USB = 1;                                     // Enable USB interrupt in the 8051 core
}

void UsbIntFunct(void) {
    uint8_t len = 0;
    
    /* Check for USB Transfer Completion Interrupt */
    if (UIF_TRANSFER) {
        /* Decode the token type and the target endpoint number */
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP)) {
            
            case UIS_TOKEN_IN | 1:
                /* Endpoint 1 IN: Data transmission completed (Interrupt IN) */
                UEP1_T_LEN = 0; /* Clear transmit length for the next transaction */
                /* Reset endpoint state: keep toggle bit, clear response mask, set to NAK */
                UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                break;
                
            case UIS_TOKEN_SETUP | 0:
                /* Endpoint 0 SETUP: Received a Setup packet from Host */
                len = USB_RX_LEN; /* Get the length of received Setup data */
                
                /* Validate standard Setup packet size (must be 8 bytes) */
                if (len == (sizeof(USB_SETUP_REQ))) {
                    SetupLen = UsbSetupBuf->wLengthL; /* Get lower byte of expected data length */
                    
                    /* Safeguard: limit max length to 0x7F to prevent buffer overflow */
                    if (UsbSetupBuf->wLengthH || SetupLen > 0x7F) {
                        SetupLen = 0x7F;
                    }
                    
                    SetupReq = UsbSetupBuf->bRequest;       /* Store request code (e.g., Get_Descriptor) */
                    IndexReq = UsbSetupBuf->wIndexL;         /* Store Interface/Endpoint index number */
                    
                    /* PRE-SET TO ERROR STATE (Fail-Fast approach) to eliminate redundant default cases */
                    len = 0xFF; 
                    
                    /* Check if the request is Class-Specific (e.g., HID commands) or Standard */
                    if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) {
                        /* Handle HID Class-Specific Requests (GetReport, SetReport, etc.) */
                        switch (SetupReq) {
                                
                            case 0x09: /* Set_Report request (Setup Stage) */
                                len = 0; /* Clear error; host will send report data in OUT stage */
                                break;
                        }   
                    } else {
                        /* Handle Standard USB Requests */
                        switch (SetupReq) {
                            case USB_GET_DESCRIPTOR:
                                /* Determine descriptor type from high byte of wValue */
                                switch (UsbSetupBuf->wValueH) {
                                    case 1: /* Device Descriptor */
                                        pDescr = DevDesc; 
                                        len = sizeof(DevDesc); 
                                        break;
                                    case 2: /* Configuration Descriptor */
                                        pDescr = CfgDesc; 
                                        len = sizeof(CfgDesc); 
                                        break;
                                    case 0x22: /* Report Descriptor (HID) */
                                         pDescr = KeyRepDesc; 
                                         len = sizeof(KeyRepDesc); 
                                         break;
                                }
                                
                                /* If a valid descriptor was matched (len updated from 0xFF), process it */
                                if (len != 0xFF) {
                                    if (SetupLen > len) SetupLen = len; /* Bound total length to host request */
                                    /* Calculate packet size for current control transaction (max EP0_SIZE) */
                                    len = SetupLen >= EP0_SIZE ? EP0_SIZE : SetupLen;
                                    memcpy(Ep0Buffer, pDescr, len); /* Load descriptor data into EP0 buffer */
                                    SetupLen -= len; /* Decrement remaining data count */
                                    pDescr += len;   /* Advance data pointer for multi-packet transfers */
                                }
                                break;
                                
                            case USB_SET_ADDRESS: 
                                /* Store the assigned USB address (will be applied in the IN status stage) */
                                SetupLen = UsbSetupBuf->wValueL; 
                                len = 0; /* Clear error, valid standard request */
                                break;
                                
                            case USB_GET_CONFIGURATION: 
                                Ep0Buffer[0] = UsbConfig; /* Return current active configuration state */
                                len = 1; 
                                break;
                                
                            case USB_SET_CONFIGURATION: 
                                UsbConfig = UsbSetupBuf->wValueL; /* Set internal active configuration state */
                                len = 0; /* Clear error */
                                break;
                                
                            case USB_CLEAR_FEATURE:
                                /* Check if target recipient is an Endpoint and index matches Endpoint 1 IN */
                                if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP && UsbSetupBuf->wIndexL == 0x81) {
                                    /* Reset toggle bit and clear STALL condition, resetting EP1 to NAK */
                                    UEP1_CTRL = (UEP1_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    len = 0; /* Clear error */
                                }
                                break;
                                
                            case USB_SET_FEATURE:
                                /* Check if target recipient is Endpoint 1 IN and value is ENDPOINT_HALT (0x00) */
                                if ((UsbSetupBuf->bRequestType & 0x1F) == 0x02 && UsbSetupBuf->wValueH == 0 && UsbSetupBuf->wValueL == 0x00 && UsbSetupBuf->wIndexL == 0x81) {
                                    /* Clear toggle bit and force Endpoint 1 IN into STALL state */
                                    UEP1_CTRL = (UEP1_CTRL & (~bUEP_T_TOG)) | UEP_T_RES_STALL;
                                    len = 0; /* Clear error */
                                }
                                break;
                                
                            case USB_GET_STATUS: 
                                Ep0Buffer[0] = 0x00; /* Bus-powered, no remote-wakeup support */
                                Ep0Buffer[1] = 0x00; 
                                len = SetupLen >= 2 ? 2 : SetupLen; /* Avoid overflowing host buffer */
                                break;
                        }
                    }
                } else {
                    len = 0xFF; /* Setup packet size is invalid */
                }

                /* Control Endpoint 0 Response Generation */
                if (len == 0xFF) {
                    SetupReq = 0xFF; /* Reset request tracker due to error */
                    /* Stall both IN and OUT directions on Endpoint 0 to indicate protocol error */
                    UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
                } else {
                    UEP0_T_LEN = len; /* Set transmission data length */
                    /* Set both RX/TX to ACK. Default data stage packet starts with DATA1 */
                    UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
                }
                break;
                
            case UIS_TOKEN_IN | 0:
                /* Endpoint 0 IN: Transmit stage of Control Read or Status stage of Control Write */
                switch (SetupReq) {
                    case USB_GET_DESCRIPTOR:
                        /* Send next chunk of data if descriptor is larger than maximum packet size */
                        len = SetupLen >= EP0_SIZE ? EP0_SIZE : SetupLen;
                        memcpy(Ep0Buffer, pDescr, len);
                        SetupLen -= len; pDescr += len; UEP0_T_LEN = len;
                        UEP0_CTRL ^= bUEP_T_TOG; /* Manually flip the DATA0/DATA1 toggle bit */
                        break;
                        
                    case USB_SET_ADDRESS:
                        /* Device address must be applied ONLY after the status stage IN token completes */
                        USB_DEV_AD = (USB_DEV_AD & bUDA_GP_BIT) | SetupLen;
                        /* Reconfigure EP0 to default listening state (ACK OUT, NAK IN) */
                        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                        break;
                        
                    default:
                        /* Status stage complete for zero-length/write requests */
                        UEP0_T_LEN = 0;
                        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                        break;
                }
                break;
                
            case UIS_TOKEN_OUT | 0:
                /* Endpoint 0 OUT: Receive stage of Control Write (e.g., Set_Report data phase) */
                len = USB_RX_LEN; /* Read length of payload sent by host */
                if (SetupReq == 0x09) { /* Process payload of HID Set_Report */
                    KeyLockLed = Ep0Buffer[0]; /* Interface 0: Keyboard LEDs (NumLock, CapsLock...) */
                    is_usb_ready=1;
                    led_mode=LED_MODE_READY;
                }
                UEP0_CTRL ^= bUEP_R_TOG; /* Flip RX data toggle bit for the next packet */
                break;
        }
        UIF_TRANSFER = 0; /* Clear the completed transfer flag to acknowledge hardware */
    }
    
    /* Check for USB Bus Reset Interrupt */
    if (UIF_BUS_RST) {
        /* Initialize Control Endpoint 0 state: Respond ACK to OUT, NAK to IN */
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        /* Enable hardware auto-toggling for Endpoint 1 (Interrupt IN) and ready to ACK */
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_R_RES_ACK;
        /*  Endpoint 2 STALL */
        UEP2_CTRL = UEP_R_RES_STALL | UEP_T_RES_STALL;
        UEP3_CTRL = UEP_R_RES_STALL | UEP_T_RES_STALL;
        
        USB_DEV_AD = 0x00; /* Reset device address back to default (0x00) */
        
        /* Clear all hanging interrupt flags during reset state */
        UIF_SUSPEND = 0; 
        UIF_TRANSFER = 0; 
        UIF_BUS_RST = 0; /* Acknowledge Bus Reset flag */
    }
    
    /* Check for USB Suspend / Resume Event Interrupt */
    if (UIF_SUSPEND) {
        UIF_SUSPEND = 0; /* Acknowledge Suspend event flag */
    }
}

