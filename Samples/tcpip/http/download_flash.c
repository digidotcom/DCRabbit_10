/*
   Copyright (c) 2016, Digi International Inc.

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/*******************************************************************************
        download_flash.c

        This program demonstrates some more complex CGI programming, used to
        download a binary image of the serial or parallel boot flash.
        
        Select "Compile to Target/Store Program in RAM" to download an image
        of what's already on the flash without modification.  This program
        was designed for debugging purposes only, and it would be insecure
        to incorporate this functionality into the production version of your
        firmware.

*******************************************************************************/

#if ! (RAM_COMPILE || SUPPRESS_FAST_RAM_COPY)
	#fatal To preserve the flash, this sample must be compiled to RAM.
#endif
#if _SERIAL_BOOT_FLASH_
	#use "BOOTDEV_SF_API.lib"
#endif

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 5

/*
 * Define a number of concurrent web servers, along with the socket buffers
 * needed to support them.
 */
#define HTTP_MAXSERVERS 4
#define MAX_TCP_SOCKET_BUFFERS 4

/*
 * Increases the timeout for an HTTP connection to 60 seconds.
 */
#define HTTP_TIMEOUT 60

// Increase buffer size for MIME type in SSPEC_MIMETABLE_START.
#define SSPEC_MAXNAME 25

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

#if HTTP_MAXBUFFER < 1024
	#error This sample requires a minimum HTTP_MAXBUFFER of 1024.
#endif

// This associates file extensions with file types.  The default for / must
// be first.
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".bin", MIMETYPE_BINARY)
SSPEC_MIMETABLE_END

// This is the prototype for the CGI functions.
int index_html(HttpState *state);
int flash_bin(HttpState *state);

// This structure associates resource names with their locations in memory.
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_FUNCTION("/", index_html),
	SSPEC_RESOURCE_FUNCTION("/boot.bin", flash_bin)
SSPEC_RESOURCETABLE_END

// This code is a catch-all that writes out any data to the HTTP server
// socket that has not yet been written.
int write_http_buffer(HttpState *state)
{
	if (state->length == 0) {
		return 0;
	}
	
	if (state->offset < state->length) {
		state->offset += sock_fastwrite(&state->s,
				state->buffer + (int)state->offset,
				(int)state->length - (int)state->offset);
	}
	if (state->offset == state->length) {
		state->offset = 0;
		state->length = 0;
	}

	// Indicate whether there's still data to write
	return (int) (state->length - state->offset);
}

#if !_SERIAL_BOOT_FLASH_
// Code to access parallel flash copied from BOARD_UPDATE.LIB.
//
// Macros to map in upper 512KB and lower 512KB of parallel boot flash.
// On boards with 512KB, there's only one "half" to map in, so use null macros.
// ! Functions using these macros must be compiled to "useix" since the extra
// PUSH IP in _PBF_LOWER() changes the stack in a way that the compiler
// misses. !
#if !_RUN_FROM_RAM || _FLASH_SIZE_ <= 512/4
	#define _PBF_LOWER()
	#define _PBF_UPPER()
#elif _FLASH_SIZE_ == 1024/4
	#define _PBF_LOWER()	asm push ip $ ipset 3 $ ld hl,MB3CR $ ioi set 4,(hl)
	#define _PBF_UPPER()	asm ld hl,MB3CR $ ioi res 4,(hl) $ pop ip
#else
	#fatal "Library not designed for parallel flash sizes other than 512K/1MB."
#endif
// This function must be declared with "useix" since the PUSH IP in
// _PBF_LOWER() breaks all stack references (C or assembly) using SP.
nodebug useix
int pbf_far_Read(void __far *dest, unsigned long offset, int bytes)
{
	auto long block;
	auto int copy;
	auto int copied;
	auto unsigned char __far *buffer = dest;

#if !_RUN_FROM_RAM
	// Boards running from flash are very easy -- PBF is mapped to 0x000000.
	_f_memcpy(buffer, (void __far *) offset, bytes);

	return bytes;
#else
	#if _RUN_FROM_RAM && (MSB_BIT != 20)
		#error "This code was written on the assumption that memory is mapped"
		#error "in 512KB blocks.  Any change to that assumption should result"
		#fatal "in changes to this code."
	#endif

	// If reading from parallel flash, it's mapped in at MB3CR.
	// On RCM5450W and other devices with 1MB of program flash, need to flip a
	// bit in MB3CR to get at the lower 512KB of the flash.

	copied = 0;

	// Are there any bytes left to read in the lower 512KB of flash?
   block = 512ul * 1024 - offset;
	if (block > 0)
	{
		copy = (block > bytes) ? bytes : (int) block;
		_PBF_LOWER();		// map in lower 512KB of parallel flash
	   // 0x180000 = 1.5MB, the physical address of MB3.
	   _f_memcpy (buffer,
	      (char __far *) (0x180000 + offset), copy);
		_PBF_UPPER();		// map upper half of parallel flash back
	   offset += copy;
	   copied += copy;
	   buffer += copy;
	   bytes -= copy;
	}

	// if the parallel flash is larger than 512KB, check for bytes to read
   #if _FLASH_SIZE_ > 512/4
	   if (bytes)
	   {
			// still have bytes to copy, from upper 512KB of parallel flash
			block = 1024ul * 1024 - offset;
			if (bytes > block)
			{
				bytes = (int) block;
			}
	      // When reading upper 512KB of flash, use 0x100000 as offset since
	      // we've mapped the upper 512KB of the 1MB flash in at 0x180000.
	      // In this code, offset >= 0x080000
	      _f_memcpy (buffer,
	         (char __far *) (0x100000 + offset), bytes);
	      copied += bytes;
		}
   #endif
	return copied ? copied : -EEOF;
#endif
}

#endif // !_SERIAL_BOOT_FLASH_

// States for the CGI state machine
enum {
	FW_HEADER = 0,
	FW_DATA,
	FW_DONE
};

// This is the main CGI function.  It is called whenever the browser requests
// "boot.bin".  The Rabbit web server calls this function repeatedly
// until it indicates that has completed.  A CGI function must return CGI_OK
// when it needs to be called again, and CGI_DONE when it has completed.
int flash_bin(HttpState *state)
{
	auto int retval;		// Used to store the return value from functions
	auto unsigned long offset;
	
	// Check if this CGI function is being called in a "cancel" condition.  This
   // means that the connection is being aborted for some reason.  The most
   // likely reason is that the HTTP connection has timed out.
   if (state->cancel) {
      return CGI_DONE;
   }

	// Don't go past this point until all buffered data written out to socket.
	if (write_http_buffer(state)) {
		return CGI_OK;
	}

   // This is the state machine for this CGI function.  It reads blocks of
   // data from flash and sends them out over the HTTP socket.  The substate
   // member of the HTTP state structure tracks the state of this function,
   // and the subsubstate member tracks the offset into the flash.
   switch (state->substate) {

   case FW_HEADER:
      state->offset = 0;
      state->length = sprintf(state->buffer,
      	"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n", MIMETYPE_BINARY);
      state->length += sprintf(&state->buffer[state->length],
      	"Content-Length: %lu\r\n",
			#if _SERIAL_BOOT_FLASH_
				SBF_FLASH_BYTES
			#else
				_FLASH_SIZE_ * 4096UL
			#endif
      	);
      state->length += sprintf(&state->buffer[state->length],
      	"Content-Disposition: attachment; filename=\"boot.bin\"\r\n\r\n");
      state->substate = FW_DATA;
      state->subsubstate = 0;
      break;

	case FW_DATA:
		offset = 1024ul * state->subsubstate;
		#if _SERIAL_BOOT_FLASH_
			state->length = SBF_FLASH_BYTES - offset;
		#else
			state->length = _FLASH_SIZE_ * 4096UL - offset;
		#endif
		if (state->length <= 0) {
			return CGI_DONE;
		}
		
		if (state->length > 1024) {
			state->length = 1024;
		}
		#if _SERIAL_BOOT_FLASH_
			do {
				retval = sbf_far_Read(state->buffer, offset, (unsigned) state->length);
			} while (retval > 0);
		#else
			pbf_far_Read(state->buffer, offset, (int) state->length);
		#endif
		++state->subsubstate;
		break;
   }

   // By default, indicate that this CGI function needs to be called again.
	return CGI_OK;
}

// This is a simple CGI script to display some information about the board along
// with a link to boot.bin.
int index_html(HttpState *state)
{
	auto char mac_addr[18];
	
	// Don't go past this point until all buffered data written out to socket.
	if (write_http_buffer(state)) {
		return CGI_OK;
	}
	
	if (state->substate) {
		return CGI_DONE;
	}
	
	state->offset = 0;
	state->length = sprintf(state->buffer,
		"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", MIMETYPE_HTML);
	state->length += sprintf(&state->buffer[state->length],
		"<html><body>%s (%s) <a href=\"boot.bin\">boot.bin</a></body></html>",
		_BOARD_NAME_, inet_ethtoa(mac_addr, SysIDBlock.macAddr));
	++state->substate;
	
	return CGI_OK;
}

void main(void)
{
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
	tcp_reserveport(80);

   while (1) {
   	http_handler();
   }
}

