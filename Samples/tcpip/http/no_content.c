/*******************************************************************************
   Samples/tcpip/http/no_content.c
	Digi International, Copyright ©2009.  All rights reserved.

	This sample demonstrates use of the "204 No Content" HTTP/1.1 response.
	The HTTP/1.1 specification includes the following in its section (10.2.5)
	on the "204 No Content" response:

		If the client is a user agent, it SHOULD NOT change its document view
		from that which caused the request to be sent. This response is primarily
		intended to allow input for actions to take place without causing a
		change to the user agent's active document view, although any new or
		updated metainformation SHOULD be applied to the document currently in
		the user agent's active view.

	This makes the 204 response useful for a remote control interface, where
	the web browser page doesn't change, but the Rabbit device does something
	based on the command sent.

	In this sample, simple buttons simulate a remote control and just print
	the commands in the STDIO window.  By using the "204 No Content" response,
	the page does not reload/refresh.

*******************************************************************************/

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
 * Web server configuration
 */

/*
 * only one socket and server are needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1

/********************************
 * End of configuration section *
 ********************************/

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

#ximport "samples/tcpip/http/pages/no_content.html"      index_html

/*
 *  In this case the .html is not the first type in the
 *  type table.  This causes the default (no extension)
 *  to assume the shtml_handler.
 *
 */
/* the default for / must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END

int cgi_rewind(HttpState* state)
{
	printf("REWIND\n");
	return cgi_nocontent(state);
}
int cgi_play(HttpState* state)
{
	printf("PLAY\n");
	return cgi_nocontent(state);
}

int cgi_stop(HttpState* state)
{
	printf("STOP\n");
	return cgi_nocontent(state);
}

int cgi_forward(HttpState* state)
{
	printf("FAST FORWARD\n");
	return cgi_nocontent(state);
}


SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", index_html),
	SSPEC_RESOURCE_FUNCTION("/rew.cgi", cgi_rewind),
	SSPEC_RESOURCE_FUNCTION("/play.cgi", cgi_play),
	SSPEC_RESOURCE_FUNCTION("/stop.cgi", cgi_stop),
	SSPEC_RESOURCE_FUNCTION("/ff.cgi", cgi_forward),
SSPEC_RESOURCETABLE_END


void main()
{
	char ipbuf[16];

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
	tcp_reserveport(80);

   printf("\n\nConnect your web browser to http://%s/\n",
   	inet_ntoa( ipbuf, MY_ADDR(IF_DEFAULT)));

   while (1)
   {
      http_handler();
   }
}


