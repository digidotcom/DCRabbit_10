/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup util_xmodem
	@{
	@file xbee/xmodem.h
	Xmodem API used with firmware updates.

	Makes use of xbee/serial.h API for sending and receiving serial data.
*/

#ifndef __XBEE_XMODEM
#define __XBEE_XMODEM

#include "xbee/platform.h"
#include "xbee/device.h"

/** @name Xmodem Control Characters
	@{
*/
/// receiver requests XMODEM (with checksum), or did not receive last block
#define XMODEM_NAK	0x15

/// receiver requests XMODEM-CRC
#define XMODEM_CRC	'C'

/// start of 128-byte block
#define XMODEM_SOH	0x01

/// start of 1024-byte block
#define XMODEM_STX	0x02

/// acknowledge receipt of block
#define XMODEM_ACK	0x06

/// cancel transmission
#define XMODEM_CAN	0x18

/// sender is ending transmission
#define XMODEM_EOT	0x04		// end of transmission
//@}

/* START FUNCTION DESCRIPTION ********************************************
*xbee_xmodem_read_fn                    <xmodem.h>

SYNTAX:
   typedef int (*xbee_xmodem_read_fn)( void FAR * context,  void FAR *buffer, 
                                       int16_t bytes)

DESCRIPTION:
     Function to assign to file.read or stream.read member of an
     xbee_xmodem_state_t object.

     Used to read data from the Xmodem receiver
     (ACK/NAK bytes) or the firmware source (e.g., file or embedded array).


PARAMETER1:  context - either \c file.context or \c stream.context
              from the xbee_xmodem_state_t object
PARAMETER2:  buffer - buffer to store read data
PARAMETER3:  bytes - maximum number of bytes to write to \c buffer


RETURNS:  >0       - number of bytes read
          -ENODATA - no more bytes to read
          -EINVAL  - NULL pointer or negative byte count passed to function
          0        - no more bytes to read

**************************************************************************/
typedef int (*xbee_xmodem_read_fn)( void FAR * context,
														void FAR *buffer, int16_t bytes);

/* START FUNCTION DESCRIPTION ********************************************
*xbee_xmodem_write_fn                   <xmodem.h>

SYNTAX:
   typedef int (*xbee_xmodem_write_fn)( void FAR * context, 
                                        const void FAR *buffer,  int16_t bytes)

DESCRIPTION:
     Function to assign to stream.write member of an xbee_xmodem_state_t
     object.

     Used to write data to the Xmodem receiver (blocks of data).


PARAMETER1:  context - \c stream.context
PARAMETER2:  buffer - bytes to send to the receiver
PARAMETER3:  bytes - number of bytes to send


RETURNS:  >=0      - number of bytes sent
          -EINVAL  - NULL pointer or negative byte count passed to function
          <0       - irrecoverable error

**************************************************************************/
typedef int (*xbee_xmodem_write_fn)( void FAR * context,
													const void FAR *buffer, int16_t bytes);

/**
	Values for \c state member of xbee_xmodem_state_t
*/
enum xbee_xmodem_state {
	XBEE_XMODEM_STATE_FLUSH,		///< flush receive buffer and wait
	XBEE_XMODEM_STATE_START,		///< waiting for NAK or CRC char
	XBEE_XMODEM_STATE_SEND,			///< start of another packet
	XBEE_XMODEM_STATE_RESEND,		///< resend last packet
	XBEE_XMODEM_STATE_SENDING,		///< sending bytes of packet
	XBEE_XMODEM_STATE_WAIT_ACK,	///< waiting for ACK of current packet
	XBEE_XMODEM_STATE_EOF,			///< reached end of file, close connection
	XBEE_XMODEM_STATE_FINAL_ACK,	///< waiting for final ACK from receiver
	XBEE_XMODEM_STATE_SUCCESS,		///< completed transfer was successful
	XBEE_XMODEM_STATE_FAILURE,		///< transfer failed
};

/**
	Structure used to track the state of an Xmodem send.
*/
typedef struct xbee_xmodem_state_t
{
	/// flags for tracking state of transfer
	uint16_t							flags;
		/** @name Values for \c flags member of xbee_xmodem_state_t
			@{
		*/
		/// macro for "no flags", used when calling xbee_xmodem_tx_init
		#define XBEE_XMODEM_FLAG_NONE			0x0000
		/// blocks end with a 1-byte checksum
		#define XBEE_XMODEM_FLAG_CHECKSUM	0x0001
		/// blocks end with a 16-bit CRC
		#define XBEE_XMODEM_FLAG_CRC			0x0002
		/// force use of XMODEM-CRC (by ignoring NAK at start of transfer)
		#define XBEE_XMODEM_FLAG_FORCE_CRC	0x0008
		/// mask for determining block size
		#define XBEE_XMODEM_MASK_BLOCKSIZE	0x0300
		/// 128-byte blocks (default setting)
		#define XBEE_XMODEM_FLAG_128			0x0000
		/// 64-byte blocks -- non-standard block size used for OTA updates
		#define XBEE_XMODEM_FLAG_64			0x0100
		/// 1KB blocks
		#define XBEE_XMODEM_FLAG_1K			0x0200
		/// alternate macro name for #XBEE_XMODEM_FLAG_1K
		#define XBEE_XMODEM_FLAG_1024			XBEE_XMODEM_FLAG_1K
#ifdef XBEE_XMODEM_TESTING
		// Macros for testing both sides of the xmodem transfer code
		/// ignore the next ACK that comes in from the receiver
		#define XBEE_XMODEM_FLAG_DROP_ACK	0x1000
		/// don't actually send the next frame (but act like you did)
		#define XBEE_XMODEM_FLAG_DROP_FRAME	0x2000
		/// make the CRC-16 bad (and make sure the other side asks for resend!)
		#define XBEE_XMODEM_FLAG_BAD_CRC		0x4000
#endif

		/// mask of user-settable flags that can be passed to xbee_xmodem_tx_init
		#define XBEE_XMODEM_FLAG_USER			\
			(XBEE_XMODEM_MASK_BLOCKSIZE | XBEE_XMODEM_FLAG_FORCE_CRC)
		///@}

	/// current packet number; starts at 1 and low byte used in block headers
	uint16_t							packet_num;

	/// timer value used to hold low word of xbee_millisecond_timer()
	uint16_t							timer;

	enum xbee_xmodem_state		state;		///< current state of transfer
	uint_fast8_t					tries;		///< # of tries left before giving up
	int								offset;		///< offset into packet being sent
	char 						FAR	*buffer;		///< buffer we can use
	struct {
		xbee_xmodem_read_fn		read;			///< source of bytes to send
		void 					FAR	*context;	///< context for file.read()
	} file;		///< function and context to read source of sent data
	struct {
		xbee_xmodem_read_fn		read;			///< read response bytes from target
		xbee_xmodem_write_fn		write;		///< send blocks to target
		void					FAR	*context;	///< context for stream.read & .write
	} stream;	///< functions and context to communicate with target device
} xbee_xmodem_state_t;

/* START FUNCTION DESCRIPTION ********************************************
xbee_xmodem_use_serport                 <xmodem.h>

SYNTAX:
   int xbee_xmodem_use_serport( xbee_xmodem_state_t *xbxm, 
                                xbee_serial_t *serport)

DESCRIPTION:
     Used for xmodem transfers over a simple serial port.

     Associates the serial
     port with the xbee_xmodem_state_t and sets stream.read and
     stream.write function pointers to helper functions that read/write a
     serial port.

     Must be called before xbee_xmodem_tx_tick() and either before or after
     xbee_xmodem_tx_init().


PARAMETER1:  xbxm - state object to configure for serial read/write
PARAMETER2:  serport - port to use for transfer


RETURNS:  0        - successfully associated \c serport with \c xbxm
          -EINVAL  - NULL parameter passed in
          <0       - error assigning \c serport to \c xbxm



**************************************************************************/
int xbee_xmodem_use_serport( xbee_xmodem_state_t *xbxm, xbee_serial_t *serport);

/* START FUNCTION DESCRIPTION ********************************************
xbee_xmodem_set_source                  <xmodem.h>

SYNTAX:
   int xbee_xmodem_set_source( xbee_xmodem_state_t *xbxm,  void FAR *buffer, 
                               xbee_xmodem_read_fn read, 
                               const void FAR *context)

DESCRIPTION:
     Configure the data source for the Xmodem send.


PARAMETER1:  xbxm - state structure to configure
PARAMETER2:  buffer - buffer for use by xbee_xmodem_tx_tick -- must be at
              least 5 bytes larger than the block size
              passed to xbee_xmodem_tx_init()
PARAMETER3:  read - function used to read bytes to send to the target
PARAMETER4:  context - context passed to \c read function


RETURNS:  0        - successfully configured data source
          -EINVAL  - invalid parameter passed in

**************************************************************************/
int xbee_xmodem_set_source( xbee_xmodem_state_t *xbxm,
	void FAR *buffer, xbee_xmodem_read_fn read, const void FAR *context);

/* START FUNCTION DESCRIPTION ********************************************
xbee_xmodem_set_stream                  <xmodem.h>

SYNTAX:
   int xbee_xmodem_set_stream( xbee_xmodem_state_t *xbxm, 
                               xbee_xmodem_read_fn read, 
                               xbee_xmodem_write_fn write, 
                               const void FAR *context)

DESCRIPTION:
     Configure the stream used to communicate with the target.

     Associates
     function pointers and a context that are used to send data to and receive
     data from the target (device receiving data via Xmodem).


PARAMETER1:  xbxm - state structure to configure
PARAMETER2:  read - function used to read bytes from the target
PARAMETER3:  write - function used to send bytes to the target
PARAMETER4:  context - context passed to \c read and \c write functions


RETURNS:  0        - successfully configured communication path to target
          -EINVAL  - invalid parameter passed in



**************************************************************************/
int xbee_xmodem_set_stream( xbee_xmodem_state_t *xbxm,
	xbee_xmodem_read_fn read, xbee_xmodem_write_fn write,
	const void FAR *context);

/* START FUNCTION DESCRIPTION ********************************************
xbee_xmodem_tx_init                     <xmodem.h>

SYNTAX:
   int xbee_xmodem_tx_init( xbee_xmodem_state_t *xbxm,  uint16_t flags)

DESCRIPTION:
     Initialize state structure for use with xbee_xmodem_tx_tick() to send a
     file via Xmodem.


PARAMETER1:  xbxm - state structure to initialize
PARAMETER2:  flags - one of the following macros, indicating the block size
              to use for the transfer
              -	XBEE_XMODEM_FLAG_NONE
              -	XBEE_XMODEM_FLAG_64
              -	XBEE_XMODEM_FLAG_128
              -	XBEE_XMODEM_FLAG_1024


RETURNS:  -EINVAL  - invalid parameter passed in
          0        - initialized state, can pass it to xbee_xmodem_tx_tick



**************************************************************************/
int xbee_xmodem_tx_init( xbee_xmodem_state_t *xbxm, uint16_t flags);

/* START FUNCTION DESCRIPTION ********************************************
xbee_xmodem_tx_tick                     <xmodem.h>

SYNTAX:
   int xbee_xmodem_tx_tick( xbee_xmodem_state_t *xbxm)

DESCRIPTION:
     Function to drive the Xmodem send state machine.  Call until it returns
     a non-zero result.


RETURNS:  -EINVAL  - invalid parameter passed in
          -ETIMEDOUT - connection timed out waiting for data from target
          <0       - error on send, transfer aborted
          0        - transfer in progress, call function again
          1        - transfer completed successfully


**************************************************************************/
int xbee_xmodem_tx_tick( xbee_xmodem_state_t *xbxm);

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
	#use "xbee_xmodem.c"
#endif

#endif


