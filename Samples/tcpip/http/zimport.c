/*
   Copyright (c) 2015, Digi International Inc.

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
        zimport.c

        This program uses the ZIMPORT.LIB library to compress web pages
        that are served by the HTTP server.  It demonstrates a couple of
        different ways in which compressed files can be used with the
        HTTP server.

*******************************************************************************/
#class auto

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
#define TCPCONFIG 1

/*
 * Web server configuration
 */

/*
 * Only one server and socket are needed on a reserved port, but more
 * servers allow more concurrent connections.
 */
#define HTTP_MAXSERVERS 4
#define MAX_TCP_SOCKET_BUFFERS 4

/*
 * INPUT_COMPRESSION_BUFFERS must be defined to be at least as large as
 * HTTP_MAXSERVERS.  Each server instance needs its own compression buffer,
 * because up to HTTP_MAXSERVERS compressed files can be simultaneously
 * uncompressed.  Note that if you are also uncompressing file outside of
 * the web server, you need to account for this in the number of
 * INPUT_COMPRESSION_BUFFERS.
 */
#define INPUT_COMPRESSION_BUFFERS 4

/********************************
 * End of configuration section *
 ********************************/

/*
 * Note that zimport.lib must be #use'd before http.lib (and zserver.lib
 * if you have explicity #use'd it).
 */
#memmap xmem
#use "zimport.lib"
#use "dcrtcp.lib"
#use "http.lib"

#zimport "samples/tcpip/http/pages/zimport.shtml"			zimport_shtml
#zimport "samples/tcpip/http/pages/alice.html"				alice_html
#zimport "samples/tcpip/http/pages/alice-rabbit.jpg"		alice_jpg

/* the default for / must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".shtml", MIMETYPE_HTML, shtml_handler),
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".jpg", MIMETYPE_JPG)
SSPEC_MIMETABLE_END

/*
 * Compressed files, when included in the http_flashspec[] structure, will
 * be automatically uncompressed when they are sent to the client.
 *
 * Note: if a file is compressed (i.e. included using #zimport), use
 * SSPEC_RESOURCE_ZMEMFILE rather than SSPEC_RESOURCE_XMEMFILE.
 */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_ZMEMFILE("/", zimport_shtml),
	SSPEC_RESOURCE_ZMEMFILE("/alice.html", alice_html),
SSPEC_RESOURCETABLE_END

/*
 * The following variables will be used to hold the sizes of the compressed
 * text and image files, respectively.
 */
long text_size;
long image_size;

void main(void)
{
	/*
    * Get the compressed sizes of the files
    */
	xmem2root(&text_size, alice_html, 4);
	text_size &= ZIMPORT_MASK;
	xmem2root(&image_size, alice_jpg, 4);
	image_size &= ZIMPORT_MASK;

	/*
    * When compressed files are added via the sspec_addxmemfile() function,
    * they are automatically detected as such.  This is in distinction to
    * the static resource table setup (i.e. SSPEC_RESOURCE_* macros) which
    * require explicit specification at compile time.
    *
    * Note, that jpeg or gif files (as in the following) do not generally
    * compress well (or at all).  Hence, it is best to leave image files uncompressed.
    */
	sspec_addxmemfile("/alice.jpg", alice_jpg, SERVER_HTTP);
	sspec_addvariable("text_size", &text_size, INT32, "%ld", SERVER_HTTP);
   sspec_addvariable("image_size", &image_size, INT32, "%ld", SERVER_HTTP);

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();

   while (1) {
   	http_handler();
   }
}

