/*
             LUFA Library
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

//-----------------------------------------------------------------------------------------
//	Process EP0 comms.
//
// This file, uses the LUFA090810 library, is cut down from a CDC prototype, 
// and adapted to support the two-command structure of the DG8SAQ I2C comms
//
// 2009-08-15 Loftur E. Jonasson, TF3LJ
//-----------------------------------------------------------------------------------------

/** \file
 *
 *  Header file for USB-EP0.c.
 */

#ifndef _USB_EP0_H_
#define _USB_EP0_H_

	/* Includes: */
//		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include "USB-Descriptors.h"
		#include <LUFA/Version.h>                        // Library Version Information
		#include <LUFA/Drivers/USB/USB.h>                // USB Functionality
		#include <LUFA/Scheduler/Scheduler.h>            // Simple scheduler for task management


	/* Type Defines: */

	typedef struct
	{
		uint8_t  NotificationType; /**< Notification type, a mask of REQDIR_*, REQTYPE_* and REQREC_* constants
	    	                        *   from the library StdRequestType.h header
	        	                    */
		uint8_t  Notification; /**< Notification value, a NOTIF_* constant */
		uint16_t wValue; /**< Notification wValue, notification-specific */
		uint16_t wIndex; /**< Notification wIndex, notification-specific */
		uint16_t wLength; /**< Notification wLength, notification-specific */
	} USB_Notification_Header_t;


	/* Global Variables: */
	extern unsigned char *usbMsgPtr;		// Support Si570 app

	// This function, in USB-EP0.c, when called, starts the USB works
	void Initialize_USB(void);


	/* External function prototypes */
	// The three all important functions in Mobo, hooks from the USB works
	extern void usbFunctionWrite(USB_Notification_Header_t *, unsigned char *, unsigned char);
	extern unsigned char usbFunctionSetup(USB_Notification_Header_t *);
	extern void maintask(void);

#endif

