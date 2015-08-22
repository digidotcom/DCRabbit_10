/*******************************************************************************
        Samples\TcpIp\HTTP\form2.c
        Rabbit Semiconductor, 2000

        An example of a simple HTTP form generation and parsing
        program with password protection of the form.
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
 * Only one server and socket are needed for a reserved port
 */
#define HTTP_MAXSERVERS 1
#define MAX_TCP_SOCKET_BUFFERS 1

/*
 * Size of the buffer that will be allocated to do error
 * processing during form parsing.  This buffer is allocated
 * in root RAM.  Note that it must be large enough to hold
 * the name, value, and 4 more bytes for each variable in the
 * form.
 *
 * This parameter must be defined in order to use the form
 * generation and parsing functionality.
 */
#define FORM_ERROR_BUF 256

/*
 * Define this if you do not need to use the http_flashspec array.
 * Since this program uses only the ZSERVER.LIB functionality,
 * then it does not need the http_flashspec array.
 */
#define HTTP_NO_FLASHSPEC

/*
 * This program does not need to do any DNS lookups, so it
 * disables them in order to save memory.  Note that resolve()
 * can still handle IP addresses with this option defined, but
 * it can not handle names.
 */
#define DISABLE_DNS


/********************************
 * End of configuration section *
 ********************************/

#define LAUNCHED_REDIRECTTO 		"/launched.html"

#define NELEMENTS(arr)		(sizeof(arr)/sizeof((arr)[0]))


#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

#ximport "samples/tcpip/http/pages/passform.html" index_html
#ximport "samples/tcpip/http/pages/launched.html" launched_html

/* the default mime type for '/' must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML)
SSPEC_MIMETABLE_END

int numpackages;

////////////////////////////////////////////////////////////////////////////

int launched(HttpState* state)
{
	if (numpackages > 0) {
		cgi_redirectto(state, LAUNCHED_REDIRECTTO );
	}
	return 0;
}

void main(void)
{
	auto FormVar myform[5];
	auto int var;
	auto int form;
	auto int function;
	auto int user;
	static const char* city_options[] = {
		"Davis, California",
		"Leland, Mississippi",
		"Redmond, Washington",
		"Fresno, California",
		"Canberra, Australia",
		"Black Rock Desert",
	};
	auto char city[30];

	// Initialize variables
	strcpy(city, city_options[0]);
	numpackages = 1;

	// Add the user
	user = sauth_adduser("foo", "bar", SERVER_HTTP);

	// Add existing entry page and thank-you page (stored in flash).
	sspec_addxmemfile("/", index_html, SERVER_HTTP);
	sspec_addxmemfile("index.html", index_html, SERVER_HTTP);
	sspec_addxmemfile("launched.html", launched_html, SERVER_HTTP);

	// Add the form
	form = sspec_addform("packomatic.html", myform, 5, SERVER_HTTP);
	function = sspec_addfunction("launched", launched, SERVER_HTTP);
	sspec_setformepilog(form, function);
	sspec_setuser(form, user);
	sspec_setrealm(form, "Admin");

	// Set the title of the form
	sspec_setformtitle(form, "Pack-o-matic");

	// Add the first variable, and set it up with the form
	var = sspec_addvariable("city", city, PTR16, "%s", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "Destination");
	sspec_setfvdesc(form, var, "The city you wish to send to");
	sspec_setfvlen(form, var, 30);
	sspec_setfvoptlist(form, var, city_options, NELEMENTS(city_options));
	sspec_setfventrytype(form, var, HTML_FORM_PULLDOWN);

	// Add the second variable, and set it up with the form
	var = sspec_addvariable("numpackages", &numpackages, INT16, "%d", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "Number of packages");
	sspec_setfvdesc(form, var, "The number of packages you want to send to the destination (1-20)");
	sspec_setfvlen(form, var, 4);
	sspec_setfvrange(form, var, 1, 20);

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
	http_init();
	tcp_reserveport(80);

	while (1) {
		http_handler();
	}
}