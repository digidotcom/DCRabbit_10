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
int cgi_postform1 (HttpState *state);
int cgi_getpage2 (HttpState *state);
int cgi_postform2 (HttpState *state);

//A macro that returns number of elements in an array
#define COUNTOF(A) (sizeof(A)/sizeof(A[0]))

//-------------------- HTTP Resources -----------------------------------------
SSPEC_MIMETABLE_START
  SSPEC_MIME("/", "text/html"),  // the default mime type for '/' must be first
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
  SSPEC_RESOURCE_FUNCTION("postform1.cgi", cgi_postform1),
  SSPEC_RESOURCE_FUNCTION("getpage2.cgi", cgi_getpage2),
  SSPEC_RESOURCE_FUNCTION("postform2.cgi", cgi_postform2),
SSPEC_RESOURCETABLE_END
//----------------- End of HTTP Resources -------------------------------------

//variables used to populate server pages

//data for page1
unsigned long myip;
int ival1, ival2, cbox;
float fval;
char str[80];

//data for page2
int iarr[5];
float farr[2];
char iarr_a[80];
char *parr[5];

char sarr[5][80];
const char *varr[] = {"A few strings", "scattered in flash", "shown on WEB page"};

/*
  JSON data dictionary.
  Each variable accessed through JSON libray must be described by a dictionary
  entry. The entry provides the external name of the variable, its type and
  number of elements. If the external name (JSON name) is the same as the
  program name you can use the JSD macro to generate the entry. Otherwise use
  the JSDN macro.
*/
JSD_START
  JSD (myip,    JT_ULONG, 1, 0),
  JSD (ival1,   JT_INT,   1, 0),
  JSD (ival2,   JT_INT,   1, 0),
  JSD (fval,    JT_FLT,   1, 0),
  JSD (cbox,    JT_INT,   1, 0),
  JSD (iarr,    JT_INT,   COUNTOF(iarr), 0),
  JSD (farr,    JT_FLT,   COUNTOF(farr), 0),
  JSD (str,     JT_STR,   1,             sizeof(str)),
  JSD (sarr,    JT_STR,   COUNTOF(sarr), sizeof(sarr[0])),
  JSD (varr,    JT_PSTR,  COUNTOF(varr), 0), 
  JSD (iarr_a,  JT_STR,   1,             sizeof(iarr_a)),
JSD_END;

/* Notes:

  1. Differences between JT_STR and JT_PSTR. 'sarr' is an array of 5 strings
  each 80 characters long and is declared in the data dictionary as a JT_STR
  entry with 5 elements, each 80 bytes long. Meanwhile 'varr' is simply an 
  array of pointers to strings and it is declared as a JT_PSTR entry. The 
  most appropriate representaion depends on application.
  
  2. The size of each element of 'varr' is not important in this case as the strings
  are in flash and cannot be updated. If they would need to be updated the max size
  of each string would need to be specified.
  
  3. 'iarr_a' is included just to show that the parser can handle names with underscores
  that do not conform to <array>_<index> pattern.
  
  4. The 'url_post' parser parses the keys in the order they are received which
  in turn is the order the fields are layed out in the web form. Fileds with
  same name will hence use the value of the LAST field with that name. This is
  important for checkbox type fields who normally have associated a hidden input
  field with the same name and the value corresponding to the unchecked state.
  The hidden field must be placed BEFORE the checkbox field.
*/

//=============================================================================
/*
    JSONify page1 data
*/
int cgi_getpage1 (HttpState *state)
{
  //send header and init output buffer
  json_begin (state);

  //format all variables that need to be sent to browser
  jsonify (state, &myip);
  jsonify (state, &ival1);
  jsonify (state, &ival2);
  jsonify (state, &fval);
  jsonify (state, &str);
  jsonify (state, &cbox);

  //finish buffer and send it back
  json_end (state);
  return 1;
}

/*
    Update form1 (on page1) data
*/
int cgi_postform1 (HttpState *state)
{
  int len;

  //parse POST data
  if ((len=url_post (state)) <= 0)
  {
    if (len = -4)
      return cgi_continue (state, "page1.html"); //not a POST request
    else
      return len;
  }

  //other stuff that needs to be done with received data
  printf ("Web data updated\n ival1=%d\n", ival1);

  //where to go next
  cgi_redirectto (state, "page1.html");
  return 0;
}

/*
    JSONify page2 data
*/
int cgi_getpage2 (HttpState *state)
{
  //send header and init output buffer
  json_begin (state);

  //format all variables that need to be sent to browser
  jsonify (state, iarr);
  jsonify (state, farr);
  jsonify (state, varr);
  jsonify (state, sarr);
  jsonify (state, iarr_a);

  //finish buffer and send it back
  json_end (state);
  return 1;
}

/*
    Update form2 (on page2) data
*/
int cgi_postform2 (HttpState *state)
{
  int len;

  //parse POST data
  if ((len=url_post (state)) <= 0)
    return (len == -4)? cgi_continue (state, "page2.html") : len;

  //other stuff that needs to be done with received data
  printf ("new value iarr_a: %s\n", iarr_a);

  //where to go next
  cgi_redirectto (state, "page2.html");
  return 0;
}

int main ()
{
  int i;

  sock_init ();
  http_init ();
  tcp_reserveport (HTTP_PORT);

  //some values for page1 variables
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

  strcpy (iarr_a, "I am iarr_a");

  strcpy (sarr[0], "The ");
  strcpy (sarr[1], "The quick");
  strcpy (sarr[2], "The quick brown");
  strcpy (sarr[3], "The quick brown fox");
  strcpy (sarr[4], "The quick brown fox jumps");

  while (1)
  {
    http_handler ();
  }
  return 0;
}

