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
 *  .
 */


#include "USB-EP0.h"

// Definitions
#define BUFFER_SIZE	24

/* Global Variables */
uint8_t dataReceived[BUFFER_SIZE];



//-----------------------------------------------------------------------------------------
// Start the USB
//-----------------------------------------------------------------------------------------
void Initialize_USB(void)
{
	USB_Init();

	for (;;)
	{
		maintask();		// Start up the Mobo runtime stuff
		USB_USBTask();	// Start up the USB works
	}
}



//-----------------------------------------------------------------------------------------
//	Event handler for the USB_UnhandledControlPacket event
//
//	This is fired whenever the host communicates with the client payload firmware
//-----------------------------------------------------------------------------------------
//void EVENT_USB_UnhandledControlPacket(void)
void EVENT_USB_Device_UnhandledControlRequest(void)
{
	unsigned char replyLen;											// length of reply message

	//-------------------------------------------------------------------------------
	// Process USB Host to Device transmissions.  No result is returned.
	//-------------------------------------------------------------------------------
	if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD))	
	{
    	Endpoint_ClearSETUP();										// Clear for reception of data payload

		while (!Endpoint_IsOUTReceived())							// Wait for packet before attempting to read it
		{
		//	if (USB_DeviceState == DEVICE_STATE_Unattached)
		//	  return;
		}


		replyLen = Endpoint_BytesInEndpoint();						// Read the input packet

		//Endpoint_Read_Control_Stream_LE(&dataReceived, replyLen);// Costly in bytes, hence the code below:
		// We only attempt to read if data is of legitimate size, otherwise - throw away
		if (replyLen<=BUFFER_SIZE)
		{
			// Read one byte at a time
			for (uint8_t i=0;i<replyLen;i++) dataReceived[i] = Endpoint_Read_Byte();
		}

		Endpoint_ClearOUT();										// Done with data, clear endpoint, discard
		// Wait until the host is ready to receive the request confirmation
		// Appears not to be necessary
		//while (!(Endpoint_IsINReady()))
		//{
		//	if (USB_DeviceState == DEVICE_STATE_Unattached)
		//	  return;
		//}
		Endpoint_ClearIN();											// And we're done, handshake

		// This is our all important hook - Do the magic... control Si570 etc...
		usbFunctionWrite((USB_Notification_Header_t *) &USB_ControlRequest, dataReceived,replyLen);
	}

	//-------------------------------------------------------------------------------
	// Process USB query commands and return a result (flexible size data payload)
	//-------------------------------------------------------------------------------
	else if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD))
	{
		// This is our all important hook - Process and execute command, read CW paddle state etc...
		replyLen = usbFunctionSetup((USB_Notification_Header_t *) &USB_ControlRequest);

		Endpoint_ClearSETUP();										// Clear for transmission of reply

		// If we try to write a zero length packet, then everything freezes up
		if (replyLen)
		{
			Endpoint_Write_Control_Stream_LE(usbMsgPtr, replyLen);	// Uh, ohh... send the reply
		}
	   	Endpoint_ClearOUT();										// And we're done, handshake
	}

	// The below should never happen
	//else
	//{
	//	// Debug stuff //PORTB = PORTB | 0x01;	// Set error LED
	//}
}
