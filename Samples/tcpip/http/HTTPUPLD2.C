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
        Samples\TCPIP\HTTP\httpupld2.c

        Demonstrate the HTTP file upload facility.  This is the same
        demo as httpupld.c, except that it generates the response
        in the CGI function (instead of using http_switchCGI()).

        The CGI merely dumps the action codes and information which is
        presented by the server.

        See the "upld_fat.c" sample for details on using the default
        upload handler CGI, and adding security.

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
 * TCP/IP modification - reduce TCP socket buffer
 * size, to allow more connections. This can be increased,
 * with increased performance, if the number of sockets
 * are reduced.  Note that this buffer size is split in
 * two for TCP sockets--1024 bytes for send and 1024 bytes
 * for receive.
 */
#define TCP_BUF_SIZE 2048

/*
 * Web server configuration
 */

/*
 * only one socket and server are needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1


//#define HTTP_VERBOSE
//#define ZSERVER_VERBOSE
//#define HTTP_DEBUG
//#define ZSERVER_DEBUG
//#define DCRTCP_DEBUG

// Bigger than default: allows us to avoid using CGI_MORE protocol
#define HTTP_MAXBUFFER	512


/********************************
 * End of configuration section *
 ********************************/

#define USE_HTTP_UPLOAD		// Required for this demo, to include upload code.

#define DISABLE_DNS		// No name lookups required

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"


#ximport "samples/tcpip/http/pages/upload.html"    index_html

// This table maps file extensions to the appropriate "MIME" type.  This is
// needed for the HTTP server.
SSPEC_MIMETABLE_START
	SSPEC_MIME(".htm", MIMETYPE_HTML),
	SSPEC_MIME(".html", MIMETYPE_HTML),
	SSPEC_MIME(".gif", MIMETYPE_GIF),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END


int upload_cgi(HttpState * s)
{
	char buf[HTTP_MAXBUFFER];
	int rc;


   if (!http_getState(s)) {
   	http_setState(s, 1);
	   http_genHeader(s, buf, HTTP_MAXBUFFER,
                  200, NULL, 0,
                  "<html><head><title>Upload Information</title></head><body>");
      sprintf(buf + strlen(buf),
        "<P>HTTP version=%s</P>" \
        "<P>HTTP method=%s</P>" \
        "<P>Userid=%d</P>" \
        "<P>URL=%ls</P>\r\n",
        http_getHTTPVersion_str(s),
        http_getHTTPMethod_str(s),
        http_getContext(s)->userid,
        http_getURL(s)
        );
   	rc = CGI_SEND;		// Send the data we put in http_getData().
   }
   else {
   	buf[0] = 0;
   	rc = 0;				// Default to not sending anything
   }


	switch (http_getAction(s)) {
   	case CGI_PROLOG:
      	// Ignore prolog data.

         break;
   	case CGI_HEADER:
      	sprintf(buf+strlen(buf), "<P>HEADER \"%ls\"</P>\r\n", http_getData(s));
         _f_strcpy(http_getData(s), buf);
         rc = CGI_SEND;
         break;
   	case CGI_START:
      	sprintf(http_getData(s),
           "<P>START content_length=%ld<BR>" \
           "field name=%s<BR>" \
           "disposition=%d<BR>" \
           "transfer_encoding=%d<BR>" \
           "content_type=%s<BR></P>\r\n"
           , http_getContentLength(s)
           , http_getField(s)
           , http_getContentDisposition(s)
           , http_getTransferEncoding(s)
           , http_getContentType(s)
           );
         rc = CGI_SEND;
         break;
   	case CGI_DATA:
      	sprintf(http_getData(s), "<P>DATA length=%d (total %ld)</P>\r\n",
            http_getDataLength(s), http_getContentLength(s));
         rc = CGI_SEND;
         break;
   	case CGI_END:
      	sprintf(http_getData(s), "<P>END ----------- actual received length=%ld</P>\r\n",
         	http_getContentLength(s));
         rc = CGI_SEND;
         break;
      case CGI_EPILOG:
      	// Ignore epilog data.
         break;
      case CGI_EOF:
      	sprintf(http_getData(s), "<P>EOF (unused content=%ld)</P></body></html>\r\n", s->content_length);
         // Since we use switchCGI, there is no need to return CGI_DONE.
   		rc = CGI_SEND_DONE;
         break;

      // The following print to the console...
      case CGI_ABORT:
      	printf("ABORT!\n");
         break;
      default:
      	printf("CGI: unknown action code %d\n", http_getAction(s));
         break;
   }
   return rc;
}


// The flash resource table is now initialized with these macros...
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_CGI("upload.cgi", upload_cgi)
SSPEC_RESOURCETABLE_END

void main()
{
	char buf[20];

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
   tcp_reserveport(80);

   printf("Ready: point your browser to http://%s/\n\n", inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));


   while (1) {
      http_handler();
   }
}


