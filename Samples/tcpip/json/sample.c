/*
  sample.c - Sample use of JSON interface
  (c) Mircea Neacsu 2016
  
  This program shows how to use the JSON library functions to effciently
  create a Web user interface for Rabbit processors.
  
  It serves two web pages (page1.html and page2.html) each one containing
  a form filled with data from server. The second page shows how JSON library
  deals with data arrays.
*/

//standard stuff for network and HTTP server configuration
#define TCPCONFIG             1
#define _PRIMARY_STATIC_IP    "192.168.3.101"
#define _PRIMARY_NETMASK      "255.255.255.0"
#define MY_NAMESERVER         "8.8.8.8"
#define MY_GATEWAY            "192.168.3.1"

#define TIMEZONE              -5
#define HTTP_PORT             80
#define HTTP_MAXBUFFER        2048

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"
#use "json.lib"

//the HTML pages served by this program
#ximport "page1.html" page1_file
#ximport "page2.html" page2_file

//prototypes
int cgi_getpage1 (HttpState *state);
int cgi_setpage1 (HttpState *state);
int cgi_getpage2 (HttpState *state);
int cgi_setpage2 (HttpState *state);

//A macro that returns number of elements in an array
#define COUNTOF(A) (sizeof(A)/sizeof(A[0]))

//varaibles used to populate server pages

//data for page1
unsigned long myip;
int ival1, ival2, cbox;
float fval;
char str[80];

//data for page2
int iarr[5], varr[6];
float farr[2];

//-------------------- HTTP Resources -----------------------------------------
/* the default mime type for '/' must be first */
SSPEC_MIMETABLE_START
  SSPEC_MIME("/", "text/html"),
  SSPEC_MIME(".html", "text/html"),
  SSPEC_MIME(".css", "text/css"),
  SSPEC_MIME(".gif", "image/gif"),
  SSPEC_MIME(".ico", "image/png"),
  SSPEC_MIME(".png", "image/png"),
  SSPEC_MIME(".jpg", "image/jpg"),
  SSPEC_MIME(".cgi", ""),
SSPEC_MIMETABLE_END

SSPEC_RESOURCETABLE_START
  SSPEC_RESOURCE_XMEMFILE("/", page1_file),
  SSPEC_RESOURCE_XMEMFILE("page1.html", page1_file),
  SSPEC_RESOURCE_XMEMFILE("page2.html", page2_file),
  SSPEC_RESOURCE_FUNCTION("getpage1.cgi", cgi_getpage1),
  SSPEC_RESOURCE_FUNCTION("setpage1.cgi", cgi_setpage1),
  SSPEC_RESOURCE_FUNCTION("getpage2.cgi", cgi_getpage2),
  SSPEC_RESOURCE_FUNCTION("setpage2.cgi", cgi_setpage2),
SSPEC_RESOURCETABLE_END

/*
  JSON data dictionary.
  Each variable accessed through JSON libray must be described by a dictionary
  entry. The entry provides the external name of the variable, its type and
  number of elements.
*/
JSD_START
  JSD (myip,     1,             JT_ULONG),
  JSD (ival1,    1,             JT_INT),
  JSD (ival2,    1,             JT_INT),
  JSD (fval,     1,             JT_FLT),
  JSD (str,      1,             JT_STR),
  JSD (cbox,     1,             JT_INT),
  JSD (iarr,     COUNTOF(iarr), JT_INT),
  JSD (farr,     COUNTOF(farr), JT_FLT),
  JSD (varr,     COUNTOF(varr), JT_INT),
JSD_END;


/*
    JSONify page1 data
*/
int cgi_getpage1 (HttpState *state)
{
  //send header and init output buffer
  json_begin (state);

  //format all variables that need to be sent back
  jsonify (state, &myip);
  jsonify (state, &ival1);
  jsonify (state, &ival2);
  jsonify (state, &fval);
  jsonify (state, &str);
  jsonify (state, &cbox);

  //finish buffer and send it back to user
  json_end (state);
  return 1;
}

/*
    Update page1 data
*/
int cgi_setpage1 (HttpState *state)
{
  int len;

  //make sure this is a POST request
  if (http_getHTTPMethod (state) != HTTP_METHOD_POST)
  	return cgi_continue (state, "page1.html");

  //parse POST data
  if ((len=urlpost (state)) <= 0)
    return len;

  //other stuff that needs to be done with received data
  printf ("Web data updated\n ival1=%d\n", ival1);

  //where to go next
  return cgi_continue (state, "page1.html");
}

/*
    JSONify page1 data
*/
int cgi_getpage2 (HttpState *state)
{
  //send header and init output buffer
  json_begin (state);

  //format all variables that need to be sent back
  jsonify (state, iarr);
  jsonify (state, farr);
  jsonify (state, varr);

  //finish buffer and send it back to user
  json_end (state);
  return 1;
}

/*
    Update page2 data
*/
int cgi_setpage2 (HttpState *state)
{
  int len;

  //make sure this is a POST request
  if (http_getHTTPMethod (state) != HTTP_METHOD_POST)
  	return cgi_continue (state, "page2.html");

  //parse POST data
  if ((len=urlpost (state)) <= 0)
    return len;

  //other stuff that needs to be done with received data
  ;

  //where to go next
  return cgi_continue (state, "page2.html");
}

int main ()
{
  int i;
  
  sock_init ();
  http_init ();
  tcp_reserveport (HTTP_PORT);

  //some values page1 variables
  myip = IPADDR(192,168,3,101);
  ival1 = 123;
  ival2 = 456;
  fval = 789.123;
  cbox = 1;
  strcpy (str, "The quick brown fox jumps over lazy dog");

  //same stuff for page2
  for (i=0; i<COUNTOF(iarr); i++)
    iarr[i] = i*10+i+11;
  for (i=0; i<COUNTOF(farr); i++)
    farr[i] = (i+1) + (i+1)/10.;
  for (i=0; i<COUNTOF(varr); i++)
    varr[i] = i+1;
  
  
  while (1)
  {
    http_handler ();
  }
  return 0;
}

