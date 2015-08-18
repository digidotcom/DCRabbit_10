/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup xbee_device
	@{
	@file xbee/device.h
	Device layer for XBee module interface.

	@def XBEE_DEV_MAX_DISPATCH_PER_TICK
		Maximum number of frames to dispatch per call to xbee_tick().
*/

#ifndef __XBEE_DEVICE
#define __XBEE_DEVICE

#include "xbee/platform.h"
#include "xbee/serial.h"
#include "wpan/types.h"
#include "wpan/aps.h"

/** @name
	Flags used by functions in this module.
	@{
*/
#define XBEE_DEV_FLAG_NONE		0x0000
//@}

#ifndef XBEE_DEV_MAX_DISPATCH_PER_TICK
	#define XBEE_DEV_MAX_DISPATCH_PER_TICK 5
#endif

/** Possible values for the \c frame_type field of frames sent to and
	from the XBee module.  Values with the upper bit set (0x80) are frames
	we receive from the XBee module.  Values with the upper bit clear are
	for frames we send to the XBee.
*/
enum xbee_frame_type {
	/// Send an AT Command to the local device (see xbee_atcmd.c,
	/// xbee_header_at_request_t).
	XBEE_FRAME_LOCAL_AT_CMD					= 0x08,

	/// Queue an AT command for batch processing on the local device.
	XBEE_FRAME_LOCAL_AT_CMD_Q				= 0x09,

	/// Send data to a default endpoint and cluster on a remote device.
	XBEE_FRAME_TRANSMIT						= 0x10,

	/// Send data to a specific endpoint and cluster on a remote device
	/// (see xbee_wpan.c).
	XBEE_FRAME_TRANSMIT_EXPLICIT			= 0x11,

	/// Send an AT command to a remote device on the network (see xbee_atcmd.c,
	/// xbee_header_at_request_t).
	XBEE_FRAME_REMOTE_AT_CMD				= 0x17,

	XBEE_FRAME_CREATE_SRC_ROUTE			= 0x21,

	XBEE_FRAME_REG_JOINING_DEV				= 0x24,

	/// Response from local device to AT Command (see xbee_atcmd.c,
	/// xbee_cmd_response_t).
	XBEE_FRAME_LOCAL_AT_RESPONSE			= 0x88,

	/// Current modem status (see xbee_frame_modem_status_t).
	XBEE_FRAME_MODEM_STATUS					= 0x8A,

	XBEE_FRAME_TRANSMIT_STATUS				= 0x8B,

	XBEE_FRAME_RECEIVE						= 0x90,		// ATAO == 0

	/// Data received for specific endpoint/cluster (see xbee_wpan.c).
	XBEE_FRAME_RECEIVE_EXPLICIT			= 0x91,		// ATAO != 0

	XBEE_FRAME_IO_RESPONSE					= 0x92,

	XBEE_FRAME_SENDOR_READ					= 0x94,

	XBEE_FRAME_NODE_ID						= 0x95,

	/// Response from remote device to AT Command (see xbee_atcmd.c,
	/// xbee_cmd_response_t).
	XBEE_FRAME_REMOTE_AT_RESPONSE			= 0x97,

	XBEE_FRAME_FW_UPDATE_STATUS			= 0xA0,

	XBEE_FRAME_ROUTE_RECORD					= 0xA1,

	XBEE_FRAME_DEVICE_AUTHENTICATED		= 0xA2,
	XBEE_FRAME_ROUTE_REQUEST_INDICATOR	= 0xA3,
	XBEE_FRAME_REG_JOINING_DEV_STATUS	= 0xA4,
};

/**
	@name
	Values for the \c options field of many receive frame types.
*/
//@{
#define XBEE_RX_OPT_ACKNOWLEDGED    0x01
#define XBEE_RX_OPT_BROADCAST       0x02
#define XBEE_RX_OPT_APS_ENCRYPT		0x20

//@}

/// Max payload for all supported XBee types is 256 bytes.  Actual firmware used
/// may affect that size, and even enabling encryption can have an affect.
/// Smart Energy and ZigBee are limited to 128 bytes, DigiMesh is 256 bytes.
#ifndef XBEE_MAX_RFPAYLOAD
	#define XBEE_MAX_RFPAYLOAD 128
#endif

/// Max Frame Size, including type, is for 0x91, Receive Explicit.  Note that
/// this is only for received frames -- we send 0x11 frames with 20 byte header.
#define XBEE_MAX_FRAME_LEN		(XBEE_MAX_RFPAYLOAD + 18)


// We need to declare struct xbee_dev_t here so the compiler doesn't treat it
// as a local definition in the parameter lists for the function pointer
// typedefs that follow.
struct xbee_dev_t;


/** @name Function Pointer Prototypes
	Function pointer prototypes, forward declaration using "struct xbee_dev_t"
	instead of "xbee_dev_t" since we use the types in the xbee_dev_t definition.
*/
//@{

/**
/**

	Standard API for an XBee frame handler in xbee_frame_handlers global.
	These functions are only called when xbee_dev_tick() or wpan_tick()
	are called and a complete frame is ready for processing.

	@note 		There isn't an actual xbee_frame_handler_fn function in
					the XBee libraries.  This documentation exists as a template
					for writing frame handlers.

	@param[in] xbee
					XBee device that received frame.
	@param[in] frame
					Pointer to frame data.  Data starts with the frame type (the
					0x7E start byte and frame length are stripped by lower layers
					of the driver).
	@param[in] length
					Number of bytes in frame.
	@param[in] context
					Handler-specific "context" value, chosen when the handler was
					registered with xbee_frame_handler_add.

	@retval	0	successfully processed frame
	@retval	!0	error processing frame
*/
/*
					Possible errors that will need unique -Exxx return values:
	            -	Invalid xbee_dev_t
	            -	No wpan_if assigned to xbee_dev_t
	            -	Invalid length (must be > 0)
	            -	Frame pointer is NULL

	@todo			What will _xbee_frame_dispatch do with those return values?
					Does there need to be a return value equivalent to "thanks
					for that frame, but please remove me from the dispatch table"?
					Right now, the dispatcher is ignoring the return value.
					Could be useful when registering to receive status information
					on outbound frames -- once we have status, we don't need
					to receive any additional notifications.
*/
typedef int (*xbee_frame_handler_fn)(
	struct xbee_dev_t				*xbee,
	const void 				FAR	*frame,
	uint16_t 						length,
	void 						FAR	*context
);

/* START FUNCTION DESCRIPTION ********************************************
*xbee_is_awake_fn                       <device.h>

SYNTAX:
   typedef int (*xbee_is_awake_fn)( struct xbee_dev_t *xbee)

DESCRIPTION:
     Function to check the XBee device's AWAKE pin to see if it is awake.


PARAMETER1:  xbee - XBee device that received frame


RETURNS:  !0	XBee module is awake.
          0        - XBee module is asleep.

**************************************************************************/
typedef int (*xbee_is_awake_fn)( struct xbee_dev_t *xbee);

/* START FUNCTION DESCRIPTION ********************************************
*xbee_reset_fn                          <device.h>

SYNTAX:
   typedef void (*xbee_reset_fn)( struct xbee_dev_t *xbee,  bool_t asserted)

DESCRIPTION:
     Function to toggle the /RESET pin to the XBee device.


PARAMETER1:  xbee - XBee device that received frame
PARAMETER2:  asserted - non-zero to assert /RESET, zero to de-assert

**************************************************************************/
typedef void (*xbee_reset_fn)( struct xbee_dev_t *xbee, bool_t asserted);

//@}

typedef struct xbee_dispatch_table_entry {
	uint8_t						frame_type;	///< if 0, match all frames
	uint8_t						frame_id;	///< if 0, match all frames of this type
	xbee_frame_handler_fn	handler;
	void 					FAR	*context;
} xbee_dispatch_table_entry_t;



enum xbee_dev_rx_state {
	XBEE_RX_STATE_WAITSTART = 0,	///< waiting for initial 0x7E
	XBEE_RX_STATE_LENGTH_MSB,		///< waiting for MSB of length (first byte)
	XBEE_RX_STATE_LENGTH_LSB,		///< waiting for LSB of length (second byte)
	XBEE_RX_STATE_RXFRAME			///< receiving frame and/or trailing checksum
};

enum xbee_dev_flags
{
	XBEE_DEV_FLAG_CMD_INIT			= 0x0001,	///< xbee_cmd_init called
	XBEE_DEV_FLAG_QUERY_BEGIN		= 0x0002,	///< started querying device
	XBEE_DEV_FLAG_QUERY_DONE		= 0x0004,	///< querying completed
	XBEE_DEV_FLAG_QUERY_ERROR		= 0x0008,	///< querying timed out or error
	XBEE_DEV_FLAG_QUERY_REFRESH	= 0x0010,	///< need to re-query device
	XBEE_DEV_FLAG_QUERY_INPROGRESS= 0x0020,	///< query is in progress

	XBEE_DEV_FLAG_IN_TICK			= 0x0080,	///< in xbee_dev_tick

	XBEE_DEV_FLAG_COORDINATOR		= 0x0100,	///< Node Type is Coordinator
	XBEE_DEV_FLAG_ROUTER				= 0x0200,	///< Node Type is Router
	XBEE_DEV_FLAG_ENDDEV				= 0x0400,	///< Node Type is End Device
	XBEE_DEV_FLAG_ZNET				= 0x0800,	///< Firmware is ZNet
	XBEE_DEV_FLAG_ZIGBEE				= 0x1000,	///< Firmware is ZigBee
	XBEE_DEV_FLAG_DIGIMESH			= 0x2000		///< Firmware is DigiMesh
};

enum xbee_dev_mode {
	XBEE_MODE_UNKNOWN = 0,	///< Haven't started communicating with XBee yet
	XBEE_MODE_BOOTLOADER,	/**< XBee is in the bootloader, not running
											firmware */

	// Modes used by "AT firmware" and some bootloaders:
	XBEE_MODE_API,				///< XBee is using API firmware
	XBEE_MODE_IDLE,			///< idle mode, data sent is passed to remote XBee
	XBEE_MODE_PRE_ESCAPE,	///< command mode, can send AT commands to XBee
	XBEE_MODE_POST_ESCAPE,	///< wait for guard-time ms before sending +++
	XBEE_MODE_COMMAND,		///< wait guard-time ms for "OK\r" before command mode
	XBEE_MODE_WAIT_IDLE,		///< waiting for OK response to ATCN command
	XBEE_MODE_WAIT_RESPONSE	///< sent a command and now waiting for a response
};

/**
	@note
	-	This structure must start with a wpan_dev_t so that the device
		can be used with the wpan library.

	-	I have chosen to eliminate the "chip select" pin used on the
		non-existent RCM4500W (like an RCM4510W, but includes an ADC on the same
		serial port as the XBee).  Adds significant complexity, especially when
		we don't have a standard semaphore for shared serial ports.

	-	Uses function pointers for setting the reset pin and reading the awake
		pin on the XBee.  User code should not call the reset function directly;
		use xbee_dev_reset() instead so the various network layers will know
		about the reset.
*/
typedef struct xbee_dev_t
{
	/// Generic WPAN device required by the \ref zigbee layers of the API.
	wpan_dev_t		wpan_dev;

	/// Platform-specific structure required by xbee_serial.c
	xbee_serial_t	serport;

	/// Optional function to control reset pin.
	xbee_reset_fn		reset;

	/// Optional function to read AWAKE pin.
	xbee_is_awake_fn	is_awake;

	/// Value of XBee module's HV register.
	uint16_t				hardware_version;
	/** @name
		Macros related to the \c hardware_version field of xbee_dev_t.
		@{
	*/
		#define XBEE_HARDWARE_MASK				0xFF00
		#define XBEE_HARDWARE_S1				0x1700
		#define XBEE_HARDWARE_S1_PRO			0x1800
		#define XBEE_HARDWARE_S2				0x1900
		#define XBEE_HARDWARE_S2_PRO			0x1A00
		#define XBEE_HARDWARE_900_PRO			0x1B00
		#define XBEE_HARDWARE_868_PRO			0x1D00
		#define XBEE_HARDWARE_S2B_PRO			0x1E00
		#define XBEE_HARDWARE_S2C_PRO			0x2100
		#define XBEE_HARDWARE_S2C				0x2200
	//@}

	/// Value of XBee module's VR register (4-bytes on some devices)
	uint32_t				firmware_version;
	/** @name
		Macros related to the \c firmware_version field of xbee_dev_t.
		@{
	*/
		#define XBEE_PROTOCOL_MASK				0xF000
		// Series 2 (2.4 GHz) hardware
		#define XBEE_PROTOCOL_ZNET				0x1000
		#define XBEE_PROTOCOL_ZB				0x2000
		#define XBEE_PROTOCOL_SMARTENERGY	0x3000
		#define XBEE_PROTOCOL_ZB_S2C			0x4000
		#define XBEE_PROTOCOL_SE_S2C			0x5000
		// Series 4 (900 MHz) hardware
		#define XBEE_PROTOCOL_MESHLESS		0x1000
		#define XBEE_PROTOCOL_DIGIMESH		0x8000

		#define XBEE_NODETYPE_MASK				0x0F00
		#define XBEE_NODETYPE_COORD			0x0100
		#define XBEE_NODETYPE_ROUTER			0x0300
		#define XBEE_NODETYPE_ENDDEV			0x0900
	//@}

	/// Multi-purpose flags for tracking information about this device.
	enum xbee_dev_flags			flags;

	/// Buffer and state variables used for receiving a frame.
	struct rx {
		/// current state of receiving a frame
		enum xbee_dev_rx_state	state;

		/// bytes in frame being read; does not include checksum byte
		uint16_t						bytes_in_frame;

		/// bytes read so far
		uint16_t						bytes_read;

		/// bytes received, starting with frame_type, +1 is for checksum
		uint8_t	frame_data[XBEE_MAX_FRAME_LEN + 1];
	} rx;

	uint8_t		frame_id;				///< last frame_id used for sending

	// Need some state variables here if AT mode is supported (necessary when
	// using modules with AT firmware instead of API firmware, or when doing
	// firmware updates on DigiMesh 900 with API firmware).  Current state:
	// idle mode, command mode, pre-escape guard-time, post-escape guard-time.
	// Current timeout: value of MS_TIMER when guard-time expired or we expect
	// to return to idle mode from command mode.
	// If we track MS_TIMER from the last byte we sent to the XBee, we can
	// potentially skip over the pre-escape guard time and send the escape chars.

	/// Current mode of the XBee device (e.g., boot loader, API, command).
	#ifdef XBEE_DEVICE_ENABLE_ATMODE
		enum xbee_dev_mode	mode;
		uint32_t			mode_timer;		///< MS_TIMER value used for timeouts
		uint16_t			guard_time;		///< value of GT (default 1000) * 1ms
		uint16_t			idle_timeout;	///< value of CT (default 100) * 100ms
		char				escape_char;	///< value of CC (default '+')
	#endif
} xbee_dev_t;

/* START FUNCTION DESCRIPTION ********************************************
XBEE_DEV_STACK_VERSION_ADDR             <device.h>

MACRO SYNTAX:
     XBEE_DEV_STACK_VERSION_ADDR( x)

DESCRIPTION:
     Macro function to get a pointer to the LSB of the radio's firmware version.

     Typically used to define ZCL_STACK_VERSION_ADDR for the Basic cluster.


PARAMETER1:  x - name of xbee_dev_t to reference \c firmware_version from.


RETURNS:  address of the low byte of the firmware version, taking the
                     processor's endian-ness into account

**************************************************************************/
#if BYTE_ORDER == BIG_ENDIAN
	#define XBEE_DEV_STACK_VERSION_ADDR(x)	((uint8_t *)&(x).firmware_version + 3)
#else
	#define XBEE_DEV_STACK_VERSION_ADDR(x)	((uint8_t *)&(x).firmware_version)
#endif

/**
	Static table used for dispatching frames.

	The application needs to define this table, and it should end with
	the XBEE_FRAME_TABLE_END marker.

*/
extern const xbee_dispatch_table_entry_t xbee_frame_handlers[];

#define XBEE_FRAME_TABLE_END		{ 0xFF, 0, NULL, NULL }

/// @addtogroup xbee_device
/// @{
uint8_t xbee_next_frame_id( xbee_dev_t *xbee);

int xbee_dev_init( xbee_dev_t *xbee, const xbee_serial_t *serport,
	xbee_is_awake_fn is_awake, xbee_reset_fn reset);

void xbee_dev_dump_settings( xbee_dev_t *xbee, uint16_t flags);
	#define XBEE_DEV_DUMP_FLAG_NONE			0x0000
#ifndef XBEE_DEV_DUMP_FLAG_DEFAULT
	#define XBEE_DEV_DUMP_FLAG_DEFAULT		XBEE_DEV_DUMP_FLAG_NONE
#endif

int xbee_dev_reset( xbee_dev_t *xbee);

int xbee_dev_tick( xbee_dev_t *xbee);

int xbee_frame_write( xbee_dev_t *xbee, const void FAR *header,
	uint16_t headerlen, const void FAR *data, uint16_t datalen,
	uint16_t flags);
#define XBEE_WRITE_FLAG_NONE		0x0000

/// @}

// private functions exposed for unit testing

void _xbee_dispatch_table_dump( const xbee_dev_t *xbee);

uint8_t _xbee_checksum( const void FAR *bytes, uint_fast8_t length,
	uint_fast8_t initial);

int _xbee_frame_load( xbee_dev_t *xbee);

int _xbee_frame_dispatch( xbee_dev_t *xbee, const void FAR *frame,
	uint16_t length);


typedef struct xbee_frame_modem_status_t {
	uint8_t			frame_type;				//< XBEE_FRAME_MODEM_STATUS (0x8A)
	uint8_t			status;
} xbee_frame_modem_status_t;

/** @name
	Values for \c status member of xbee_frame_modem_status_t.
	@{
*/
#define XBEE_MODEM_STATUS_HW_RESET					0x00
#define XBEE_MODEM_STATUS_WATCHDOG					0x01
#define XBEE_MODEM_STATUS_JOINED						0x02
#define XBEE_MODEM_STATUS_DISASSOC					0x03
#define XBEE_MODEM_STATUS_COORD_START				0x06
#define XBEE_MODEM_STATUS_NETWORK_KEY_UPDATED	0x07
#define XBEE_MODEM_STATUS_WOKE_UP					0x0B		// DigiMesh
#define XBEE_MODEM_STATUS_SLEEPING					0x0C		// DigiMesh
#define XBEE_MODEM_STATUS_KEY_ESTABLISHED			0x10
#define XBEE_MODEM_STATUS_STACK_ERROR				0x80
//@}

/* START FUNCTION DESCRIPTION ********************************************
xbee_frame_dump_modem_status            <device.h>

SYNTAX:
   int xbee_frame_dump_modem_status( xbee_dev_t *xbee,  const void FAR *frame, 
                                     uint16_t length,  void FAR *context)

DESCRIPTION:
     Frame handler for 0x8A (XBEE_FRAME_MODEM_STATUS) frames -- dumps modem status
     to STDOUT for debugging purposes.

     View the documentation of xbee_frame_handler_fn() for this function's
     parameters and return value.

SEE ALSO:  XBEE_FRAME_MODEM_STATUS_DEBUG, xbee_frame_handler_fn()

**************************************************************************/
int xbee_frame_dump_modem_status( xbee_dev_t *xbee,
	const void FAR *frame, uint16_t length, void FAR *context);

/**
	Add this macro to the list of XBee frame handlers to have modem status changes
	dumped to STDOUT.
*/
#define XBEE_FRAME_MODEM_STATUS_DEBUG	\
	{ XBEE_FRAME_MODEM_STATUS, 0, xbee_frame_dump_modem_status, NULL }

///////////////////////////

// If compiling in Dynamic C, automatically #use the appropriate C file.
#ifdef __DC__
   #use "xbee_device.c"
#endif


#endif		// #ifdef __XBEE_DEVICE
