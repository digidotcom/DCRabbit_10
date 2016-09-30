/*
  sample_drinks.c - Using JSON interface in combination with
  Drinks Toolkit from GoinCompany (http://www.goincompany.com/drinks.php)
  
  (c) Mircea Neacsu 2016

  It is curently configured for an RCM4200 module that has some voltages
  between 0 and 5V applied to AIN0 to AIN3. Modifying it for a different
  board should be trivial.
  
  The analog voltages are scaled to some real world units and shown on 
  some Drinks instruments. 
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
#use "rcm42xx.lib"

//the HTML pages served by this program
#ximport "pagedrinks.html" pagedrinks_html
#ximport "drinks.js" drinks_js
#ximport "display.js" display_js

//prototypes
int cgi_getpagedrinks (HttpState *state);

//-------------------- HTTP Resources -----------------------------------------
SSPEC_MIMETABLE_START
  SSPEC_MIME("/", "text/html"),  // the default mime type for '/' must be first
  SSPEC_MIME(".html", "text/html"),
  SSPEC_MIME(".css", "text/css"),
  SSPEC_MIME(".gif", "image/gif"),
  SSPEC_MIME(".ico", "image/png"),
  SSPEC_MIME(".png", "image/png"),
  SSPEC_MIME(".jpg", "image/jpg"),
  SSPEC_MIME(".js", "text/javascript"),
  SSPEC_MIME(".cgi", ""),
SSPEC_MIMETABLE_END

SSPEC_RESOURCETABLE_START
  SSPEC_RESOURCE_XMEMFILE("/", pagedrinks_html),
  SSPEC_RESOURCE_XMEMFILE("pagedrinks.html", pagedrinks_html),
  SSPEC_RESOURCE_XMEMFILE("drinks.js", drinks_js),
  SSPEC_RESOURCE_XMEMFILE("display.js", display_js),
  SSPEC_RESOURCE_FUNCTION("getpagedrinks.cgi", cgi_getpagedrinks)
SSPEC_RESOURCETABLE_END
//----------------- End of HTTP Resources -------------------------------------

//variables used to populate server pages

float rpm, psi, hg;

/*
  JSON data dictionary.
  Each variable accessed through JSON libray must be described by a dictionary
  entry. The entry provides the external name of the variable, its type and
  number of elements. If the external name (JSON name) is the same as the
  program name you can use the JSD macro to generate the entry. Otherwise use
  the JSDN macro.
*/
JSD_START
  JSD (rpm,    JT_FLT, 1, 0),
  JSD (psi,    JT_FLT, 1, 0),
  JSD (hg,     JT_FLT, 1, 0),
JSD_END;

//=============================================================================
/*
    JSONify pagedrinks data
*/
int cgi_getpagedrinks (HttpState *state)
{
  //send header and init output buffer
  json_begin (state);

  //format all variables that need to be sent to browser
  jsonify (state, &rpm);
  jsonify (state, &psi);
  jsonify (state, &hg);

  //finish buffer and send it back
  json_end (state);
  return 1;
}


int main ()
{
  int i;
  float v[3];

  brdInit ();
  sock_init ();
  http_init ();
  tcp_reserveport (HTTP_PORT);

  //initial values
  rpm = psi = hg = 0.;

  while (1)
  {
    costate {
      waitfor (IntervalMs(100));  //10 times per sec
      for (i=0; i<3; i++)
        v[i] = anaInVolts (i, 2); //read voltage 0 to 5.6V
      hg = v[0] / 5 * 30.;    //first channel is scaled 0 to 30
      psi = v[1] / 5 * 50.;   // 2nd channel is 0 to 50
      rpm = v[2] /5 * 5000.;  //3rd channel is 0 to 5000      
    }
    
    http_handler ();
  }
  return 0;
}

