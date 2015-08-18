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
        form1.c

        An example of a simple HTTP form generation and parsing
        program.  It includes an example of how to handle interdependent
        variables in the form (see the low and high temperature
        variables).
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
 * Only one socket and server are needed for a reserved port
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

#memmap xmem
#use "dcrtcp.lib"
#use "http.lib"

/* the default mime type for '/' must be first */
SSPEC_MIMETABLE_START
	SSPEC_MIME(".html", MIMETYPE_HTML)
SSPEC_MIMETABLE_END

/* The following variables will be included in the form */
int temphi;
int tempnow;
int templo;
float humidity;
char fail[21];

/* This function checks the low temperature against the new value for the
 * high temperature to make sure that the low temperature is not higher
 * than the high temperature.  To check against the new high temperature,
 * it must use the value in the error buffer. */
int checkLowTemp(int newval)
{
	auto char* var;
	auto char* value;
	auto long hightemp;

	var = http_finderrbuf("temphi");
	if (var != NULL) {
		http_nextfverr(var, NULL, &value, NULL, NULL);
		hightemp = _n_strtol(value, NULL, 10);
		if (newval <= hightemp) {
			return 0;
		} else {
			return -1;
		}
	} else {
		if (newval <= temphi) {
			return 0;
		} else {
			return -1;
		}
	}
}

/* This function is like the above function, except that it checks the
 * high temperature. */
int checkHighTemp(int newval)
{
	auto char* var;
	auto char* value;
	auto long lowtemp;

	var = http_finderrbuf("templo");
	if (var != NULL) {
		http_nextfverr(var, NULL, &value, NULL, NULL);
		lowtemp = _n_strtol(value, NULL, 10);
		if (newval >= lowtemp) {
			return 0;
		} else {
			return -1;
		}
	} else {
		if (newval >= templo) {
			return 0;
		} else {
			return -1;
		}
	}
}

void main(void)
{
	// Declare the FormVar array to hold form variable information
	auto FormVar myform[5];
	auto int var;
	auto int form;
	// This array lists the options that are possible for the fail variable
	static const char* fail_options[] = {
		"Email",
		"Page",
		"Email and page",
		"Nothing"
	};

	// Initialize variables
	temphi = 80;
	tempnow = 72;
	templo = 65;
	humidity = 0.3;
	strcpy(fail, "Page");

	// Add the form (array of variables)
	form = sspec_addform("myform.html", myform, 5, SERVER_HTTP);

	// Set the title of the form
	sspec_setformtitle(form, "ACME Thermostat Settings");

	// Add the first variable, and set it up with the form
	var = sspec_addvariable("temphi", &temphi, INT16, "%d", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "High Temp");
	sspec_setfvdesc(form, var, "Maximum in temperature range (60 - 90 &deg;F)");
	sspec_setfvlen(form, var, 5);
	sspec_setfvrange(form, var, 60, 90);
	sspec_setfvcheck(form, var, checkHighTemp);

	// Add the second variable, and set it up with the form
	var = sspec_addvariable("tempnow", &tempnow, INT16, "%d", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "Current Temp");
	sspec_setfvdesc(form, var, "Current temperature in &deg;F");
	sspec_setfvlen(form, var, 5);
	sspec_setfvreadonly(form, var, 1);

	// Add the third variable, and set it up with the form
	var = sspec_addvariable("templo", &templo, INT16, "%d", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "Low Temp");
	sspec_setfvdesc(form, var, "Minimum in temperature range (50 - 80 &deg;F)");
	sspec_setfvlen(form, var, 5);
	sspec_setfvrange(form, var, 50, 80);
	sspec_setfvcheck(form, var, checkLowTemp);

	// Add the fourth variable, and set it up with the form
	var = sspec_addvariable("failure", fail, PTR16, "%s", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "Failure Action");
	sspec_setfvdesc(form, var, "Action to take in case of air-conditioning failure");
	sspec_setfvlen(form, var, 20);
	sspec_setfvoptlist(form, var, fail_options, 4);
	sspec_setfventrytype(form, var, HTML_FORM_PULLDOWN);

	// Add the fifth variable, and set it up with the form
	var = sspec_addvariable("humidity", &humidity, FLOAT32, "%.2f", SERVER_HTTP);
	var = sspec_addfv(form, var);
	sspec_setfvname(form, var, "Humidity");
	sspec_setfvdesc(form, var, "Target humidity (between 0.0 and 1.0)");
	sspec_setfvlen(form, var, 8);
	sspec_setfvfloatrange(form, var, 0.0, 1.0);

	// Create aliases for this form.  This allows the form to be accessed from
	// other locations.
	sspec_aliasspec(form, "index.html");
	sspec_aliasspec(form, "/");

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();
   tcp_reserveport(80);

   while (1) {
      http_handler();
   }
}