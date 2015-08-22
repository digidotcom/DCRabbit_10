/*******************************************************************************
        Samples\TcpIp\RabbitWeb\thermo_graph.c
        Digi International Inc., 2010

        Demonstrates the use of JSON for transferring large data sets
        efficiently.  This sample simulates a thermostat application
        which controls a heater and cooler actuator, and compares the
        current temperature against a setpoint.  It maintains historical
        data which is shown graphically on the web page.

        For the purpose of this sample, the data is artificially generated,
        however by changing the thermo_tick() routine you could interface
        to real sensors and actuators.  (The sample simulates 60x faster
        than it would in "real life").

		  Most of the interesting part of this sample is in the ZHTML file,
		  which includes JavaScript code to manipulate graphical objects.
		  The JS code is in graph.js.  See the two #ximport statements
		  below.

		  NOTE: since this uses the <canvas> HTML tag, including text
		  manipulations, you will need to use a modern browser such as
		  Firefox 3.5, Chrome or Safari.  Microsoft IE 8 is not supported as
		  of the date of writing, however IE 9 is expected to work.  Any browser
		  which supports HTML 5 should work.

		  See:
        samples\tcpip\rabbitweb\pages\thermo_graph.zhtml
        samples\tcpip\rabbitweb\pages\graph.js
        https://developer.mozilla.org/en/HTML/Canvas


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
#define TCPCONFIG 1

/********************************
 * End of configuration section *
 ********************************/

/*
 * This is needed to be able to use the RabbitWeb HTTP enhancements and the
 * ZHTML scripting language.
 */
#define USE_RABBITWEB 1

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"

/*
 * This page contains the ZHTML portion of the selection variable demonstration
 */
#ximport "samples/tcpip/rabbitweb/pages/thermo_graph.zhtml"	therm_graph_zhtml
#ximport "samples/tcpip/rabbitweb/pages/graph.js"	graph_js

/* The default mime type for '/' must be first */
SSPEC_MIMETABLE_START
   // This handler enables the ZHTML parser to be used on ZHTML files...
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".js", "text/javascript")
SSPEC_MIMETABLE_END

/* Associate the #ximported files with the web server */
SSPEC_RESOURCETABLE_START
	SSPEC_RESOURCE_XMEMFILE("/", therm_graph_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/index.zhtml", therm_graph_zhtml),
	SSPEC_RESOURCE_XMEMFILE("/graph.js", graph_js)
SSPEC_RESOURCETABLE_END

/*
 * Structure variable to be registered.  Note that you can use watch
 * expressions or the evaluate expression feature during runtime to ensure that
 * the variables have properly changed values.
 */
#define CONST_H 60		// Heater actuator power (units of 100 Watts)
#define CONST_C 30		// Cooler actuator power (units of 100 Watts)

#define N_THERMO 80		// 80 * 15sec = 20 min
#define N_DECIMATE 36	// Number of raw samples per decimated sample
								// 80*36 * 15 sec = 12 hours
struct thermo_struct {
	// Basic data collection rate is one sample every 15 sec, however this
	// data is decimated (1:48) for the 12 hour history.  This data is
	// read-only to the form.
	int temp[N_THERMO];				// Time history of inside temperature,
	int temp_outside[N_THERMO];	//  outside temperature (thermal load),
	int actuator[N_THERMO];			//  whether heater/cooler activated: 0 if no,
											//   else +ve for heater, -ve for cooler
											//   with units of 100 Watts.
	int setpoint[N_THERMO];			//  desired temperature (setpoint),
	int controller_on[N_THERMO];	//  and whether thermostat turned on.
	// Following vars are read/write and are controllable via a web form.
	int lotemp;				// Lower Y bound for graphical display
	int hitemp;				// Upper Y bound for graphical display
	int interval;			// INTERVAL_20min, INTERVAL_12h
	int displ_units;		// UNITS_C, UNITS_F : display units
								//  (this struct data always in deg F)
	int curr_setpoint;	// Current setpoint
	int enabled;			// Whether thermostat enabled
};

// This is web registered for display
struct thermo_struct thermo;

// These are not web registered, but used to accumulate the time series
// data at the two supported interval rates.
struct thermo_struct thermo_20min;
struct thermo_struct thermo_12h;

void transfer_display();	// forward
/*
 * #web statements
 */
#web thermo groups=all(ro)
#web_update thermo transfer_display
#web thermo.lotemp ($thermo.lotemp >= 20 && $thermo.lotemp <= 80 || \
				web_error("low temp must be 20..80")) groups=all(rw)
#web thermo.hitemp ($thermo.hitemp >= 60 && $thermo.lotemp <= 120 || \
				web_error("high temp must be 60..120")) groups=all(rw)
#web thermo.lotemp ($thermo.lotemp < $thermo.hitemp || \
				web_error("low temp (20..80) must be less than high temp (60..120)"))
#web thermo.hitemp ($thermo.lotemp < $thermo.hitemp || \
				web_error("low temp (20..80) must be less than high temp (60..120)"))
#web thermo.curr_setpoint ($thermo.curr_setpoint >= 50 && $thermo.curr_setpoint <= 90 || \
				web_error("setpoint must be 50..90")) groups=all(rw)
#define INTERVAL_20min 0
#define INTERVAL_12h 1
#web thermo.interval select("20 minutes" = INTERVAL_20min, "12 hours" = INTERVAL_12h) \
				groups=all(rw)
#define UNITS_F 0
#define UNITS_C 1
#web thermo.displ_units select("F" = UNITS_F, "C" = UNITS_C) \
				groups=all(rw)

void transfer_display()
{
	// The actual data needs to be transferred to a #web registered variable
	// for transmission to the browser.  Pick the appropriately selected
	// data interval...  (Only the data is copied, the option settings
	// already live in the #web variable).
	if (thermo.interval == INTERVAL_20min)
		memcpy(thermo.temp, thermo_20min.temp,
				sizeof(thermo.temp) * 5);	// 5 arrays to copy
	else
		memcpy(thermo.temp, thermo_12h.temp,
				sizeof(thermo.temp) * 5);	// 5 arrays to copy
}


void new_sample(int * arry, int val, int n)
{
	// Add a sample to end of given array, and shift the older samples down.
	memcpy(arry, arry+1, sizeof(*arry)*(n-1));
	arry[n-1] = val;
}

int decimate(int * arry, int n_dec, int n)
{
	// Average most recent n samples of given array.
	long sum = 0;
	int i;
	for (i = n - n_dec; i < n; ++i)
		sum += arry[i];
	return (int)((sum + n_dec/2)/n_dec);
}

void add_sample(int inside, int outside, int htr, int coolr, int setpt, int enab)
{
	static int nsamp = 0;

	// This is called every 15 sec (in theory) to add the next data sample, although
	// for this simulation it is called a lot more often than that.
	new_sample(thermo_20min.temp, inside, N_THERMO);
	new_sample(thermo_20min.temp_outside, outside, N_THERMO);
	new_sample(thermo_20min.actuator, htr * CONST_H - coolr * CONST_C, N_THERMO);
	new_sample(thermo_20min.setpoint, setpt, N_THERMO);
	new_sample(thermo_20min.controller_on, enab, N_THERMO);
	if (++nsamp == N_DECIMATE) {
		nsamp = 0;
		new_sample(thermo_12h.temp, decimate(thermo_20min.temp, N_DECIMATE, N_THERMO), N_THERMO);
		new_sample(thermo_12h.temp_outside, decimate(thermo_20min.temp_outside, N_DECIMATE, N_THERMO), N_THERMO);
		new_sample(thermo_12h.actuator, decimate(thermo_20min.actuator, N_DECIMATE, N_THERMO), N_THERMO);
		new_sample(thermo_12h.setpoint, decimate(thermo_20min.setpoint, N_DECIMATE, N_THERMO), N_THERMO);
		new_sample(thermo_12h.controller_on, decimate(thermo_20min.controller_on, N_DECIMATE, N_THERMO), N_THERMO);

	}
}

void add_simulated_sample(void)
{
	static long nsamp = 0;
	static float inside = 70.0;
	// We try to do a simulation which is reasonably close to reality...
	// Outside temperature is a few superimposed sinewaves with a day-long
	// period.  (1 day = 4*60*24 = 5760 samples).
	// Heat load L is then (outside-inside)*k Watts where k is an insulation
	// constant (Watts per deg F).  The thermostat controls a heat pump (P)
	// which can be off (0), add H Watts, or subtract C watts.  The inside
	// temperature is then driven as dT/dt = (L+P)/m where m is the heat
	// capacity of the controlled space (Joules per deg F).
	// The thermostat turns on the heater if inside < setpoint-2 and turns on
	// the cooler if inside > setpoint+2.
#define PERIOD 5760		// Samples per day
#define CONST_k 300		// Insulation thermal conductance Watts/deg F.
#define CONST_m 500000	// Heat capacity of interior (Joules/deg F)
	float outside;
	float a = nsamp*(2*3.14159/PERIOD);
	float L, dT;

	outside = (sin(a) + 0.3*sin(a*2)) * 20 + 70;

	L = (outside - inside)*CONST_k;
	L += thermo_20min.actuator[N_THERMO-1] * 100;	// Convert to Watts
	dT = L * (15.0/CONST_m);
	inside += dT;

	add_sample((int)inside, (int)outside,
					thermo.enabled && inside < thermo.curr_setpoint-2,
					thermo.enabled && inside > thermo.curr_setpoint+2,
					thermo.curr_setpoint, thermo.enabled);
	++nsamp;
}



void main(void)
{
	int i;
	unsigned T;

	// Initialize the #web-registered variables and other data
	memset(&thermo, 0, sizeof(thermo));
	memset(&thermo_20min, 0, sizeof(thermo_20min));
	memset(&thermo_12h, 0, sizeof(thermo_12h));

	thermo.lotemp = 55;
	thermo.hitemp = 85;
	thermo.curr_setpoint = 70;
	thermo.enabled = 1;

	// Fake up an initial interesting dataset (complete half day worth)
	for (i = 0; i < PERIOD/2; ++i)
		add_simulated_sample();
	transfer_display();

	// Initialize the TCP/IP stack and HTTP server
	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);
   http_init();

	// This yields a performance improvement for an HTTP server
	tcp_reserveport(80);

#define UPD_INTVL 250	// ms between updates.  Each update represents
								// 15000 ms, so putting 250 here speeds up the
								// simulation 60x i.e. 1 sec real time is 1 minute
								// simulated time.
	T = _SET_SHORT_TIMEOUT(UPD_INTVL);
   while (1) {
		// Drive the HTTP server
      http_handler();
      if (_CHK_SHORT_TIMEOUT(T)) {
      	T = _SET_SHORT_TIMEOUT(UPD_INTVL);
			add_simulated_sample();
			transfer_display();
      }
   }
}

