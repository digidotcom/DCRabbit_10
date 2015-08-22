/*
	http client.c
	Digi International, Copyright © 2008.  All rights reserved.

	Description
	===========
	This sample program demonstrates the use of the HTTP client library to
	request files from a remote web server and display them to stdout.

	In addition, it shows how you would read the headers of the HTTP server's
	response, and use the HTTP server's clock to update the Rabbit's RTC.

	Instructions
	============
	Run sample on a Rabbit with an Internet connection.  Enter URLs in the
	STDIO window, and the sample will download and display them to stdout.

*/

///// Configuration Options /////

// define SHOW_HEADERS to display the HTTP headers in addition to the body
//#define SHOW_HEADERS

// define HTTPC_VERBOSE to turn on verbose output from the HTTP Client library
//#define HTTPC_VERBOSE

// define UPDATE_RTC to sync the Rabbit's real-time clock to the web server's
// if more than 10 seconds out of sync.
#define UPDATE_RTC

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

///// End of Configuration Options /////

//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// default functions to xmem
#memmap xmem

#use "dcrtcp.lib"

#use "http_client.lib"

void print_time()
{
	struct tm		rtc;					// time struct

   mktm( &rtc, read_rtc());
   printf( "The current RTC date/time is: %s", asctime( &rtc));
}
void httpc_demo(tcp_Socket *sock)
{
	char	url[256];
	char	body[65];		// buffer for reading body
	long	curr_skew;

	httpc_Socket hsock;
	int retval;
	int is_text;
	char far *value;

	// last clock skew setting
	curr_skew = 0;

   retval = httpc_init (&hsock, sock);
   if (retval)
   {
      printf ("error %d calling httpc_init()\n", retval);
   }
   else
   {
   	is_text = 0;
      printf ("\nEnter a URL to retrieve using the following format:\n");
      printf ("[http://][user:pass@]hostname[:port][/file.html]\n");
      printf ("Items in brackets are optional.  Examples:\n");
      printf ("  http://www.google.com/\n");
      printf ("  www.google.com\n");
      printf ("  google.com\n");
      printf ("  http://checkip.dyndns.org/\n");
      while (1)
      {
         printf ("\n\nEnter URL (blank to exit): ");
         gets (url);
         if (*url == '\0')
         {
         	break;
         }

			// clear screen (first string) and print name of URL to download
         printf ("\x1B[2J" "Retrieving [%s]...\n", url);
         retval = httpc_get_url (&hsock, url);
         if (retval)
         {
            printf ("error %d calling httpc_get_url()\n", retval);
         }
         else
         {
            while (hsock.state == HTTPC_STATE_HEADER)
            {
               retval = httpc_read_header (&hsock, url, sizeof(url));
               if (retval > 0)
               {
	               if ( (value = httpc_headermatch( url, "Content-Type")) )
	               {
	                  is_text = (0 == strncmpi( value, "text/", 5));
	               }
                  #ifdef SHOW_HEADERS
                     #ifndef HTTPC_VERBOSE
                        // echo headers if HTTP client didn't already do so
                        printf (">%s\n", url);
                     #endif
                  #endif
               }
               else if (retval < 0)
               {
                  printf ("error %d calling httpc_read_header()\n", retval);
               }
            }
            printf ("Headers were parsed as follows:\n");
            printf ("  HTTP/%s response = %d, filesize = %lu\n",
               (hsock.flags & HTTPC_FLAG_HTTP10) ? "1.0" :
               (hsock.flags & HTTPC_FLAG_HTTP11) ? "1.1" : "???",
               hsock.response, hsock.filesize);
            if (hsock.flags & HTTPC_FLAG_CHUNKED)
            {
               printf ("  body will be sent chunked\n");
            }
            printf ("  Rabbit's RTC is %ld second(s) off of server's time\n",
               hsock.skew - curr_skew);

            #ifdef UPDATE_RTC
            	if (labs (hsock.skew - curr_skew) > 10)
            	{
            		// only update if off by more than 10 seconds
            		print_time();
	               printf ("  Updating Rabbit's RTC to match web server.\n");
	               write_rtc (SEC_TIMER + hsock.skew);
	               curr_skew = hsock.skew;
	               print_time();
	            }
            #endif

            printf ("\nBody:\n");
            while (hsock.state == HTTPC_STATE_BODY)
            {
               retval = httpc_read_body (&hsock, body, 64);
               if (retval < 0)
               {
                  printf ("error %d calling httpc_read_body()\n", retval);
               }
               else if (retval > 0)
               {
	               if (is_text)
	               {
	                  body[retval] = '\0';
	                  printf ("%s", body);
	               }
	               else
	               {
							mem_dump( body, retval);
	               }
               }
            }
            httpc_close (&hsock);
            tcp_tick(NULL);
         }
      }
   }

}

// It's safer to keep sockets as globals, especially when using uC/OS-II.  If
// your socket is on the stack, and another task (with its own stack, instead
// of your task's stack) calls tcp_tick, tcp_tick won't find your socket
// structure in the other task's stack.
// Even though this sample doesn't use uC/OS-II, using globals for sockets is
// a good habit to be in.
tcp_Socket demosock;

void main()
{
	// initialize tcp_Socket structure before use
	memset( &demosock, 0, sizeof(demosock));

	printf ("http client v" HTTPC_VERSTR "\n");

	printf ("Initializing TCP/IP stack...\n");
	sock_init_or_exit(1);

   httpc_demo(&demosock);
}