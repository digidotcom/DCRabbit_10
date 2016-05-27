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
/*

	Simple iDigi sample to demonstrate registration of a custom
	"do_command" target.

	-----------------------------------------------------------------------
	Instructions:
	-----------------------------------------------------------------------

	Visit http://developer.idigi.com to create a test account if you have
	not already done so.  You can also access the iDigi documentation
	at that site.

	Compile and run this sample.  You may need to place the following
	definitions in the Options->ProjectOptions->Defines tab:

		ROOT_SIZE_4K = 7U
		XMEMCODE_SIZE = 0x90000
		_SYS_MALLOC_BLOCKS=10
		_FIRMWARE_NAME_=""
		_FIRMWARE_VERSION_=0x0101
		_PRIMARY_STATIC_IP="10.10.6.100"
      _PRIMARY_NETMASK="255.255.255.0"


	The first 3 options may not be necessary (and can be adjusted up or down
	to suit the particular board being used).  The _FIRMWARE_* macros are
	needed if you enable firmware updates by defining IDIGI_USE_RPU.
	_PRIMARY_STATIC_IP and related network configuration macros may also
	be defined to set an initial "factory default" configuration.  See
	tcpconfig.lib and the network programming manual for details.  The
	default iDigi network configuration uses DHCP, with a fallback to the
	given static IP address.

	When running, navigate to developer.idigi.com, log in, and add the
	board using the '+' button - this is only necessary the first time
	you run iDigi on a given board.  If you defined IDIGI_USE_ADDP, then the
	board will automatically appear in the list of devices which may be
	added.  When the board is added, you can double click on it to view
	and change the network configuration settings.

	Some configuration settings may cause the board to reboot.  This sample
	implements this by calling exit(0).  When using the Dynamic C debugger,
	you can simply hit F9 to run the program again (no need to recompile).
	In a real deployment, exit(0) causes the board to reboot.

	When changing the network configuration using iDigi, the configuration
	is saved in non-volatile storage and will be used next time the program
	(or any other iDigi sample) is run.  The macro settings you define
	such as _PRIMARY_STATIC_IP are only used the first time the iDigi samples
	are run.  Thereafter, it uses any settings set via the iDigi server,
	and saved in flash.

	If you accidentally configure a bad network setting (e.g. by setting a
	non-existent gateway address), then the board will try to use the
	last-known-good configuration if the "current" configuration cannot
	connect to the iDigi server.

	-----------------------------------------------------------------------

	The following gives a brief overview of custom do_command use with iDigi.

	iDigi allows web applications to send commands to some or all devices
	governed by an iDigi account.  The mechanism is to POST the following
	type of command to the iDigi server.  This is an example, and assumes
	use of the Firefox "Poster" plug-in:

	set URL field to
		http://my.devicecloud.com/ws/sci
	userid and password to iDigi account information.

	Post following xml to server (content type application/xml):

	<sci_request version="1.0">
	   <send_message>
	      <targets>
	        <device id="00000000-00000000-0090C2FF-FFE00055"/>
	      </targets>
	      <rci_request version="1.1">
				<do_command target="myTarget">
               <fieldInVar>data</fieldInVar>
               <anotherFieldInVar>data2</anotherFieldInVar>
				</do_command>
	      </rci_request>
	   </send_message>
	</sci_request>


	The important parts of the above are the <targets>, which is a list
	of device IDs to which the iDigi server will send the request.  The
	above example has only one device (and you should be careful to set
	it to the appropriate ID).

	After the <targets> element, the <rci_request> and contained elements
	are what are sent to the target device.  In this case, a <do_command>
	element specifies a custom command target is to be invoked.  The
	target= attribute specifies the target name, which must be registered
	on the device (see the idigi_register_target() API function).

	Inside the <do_command> element, nested elements indicate field names
	in the object registered for that do_command target (which currently must
	be a structure instance).  Several examples of this are given in this
	sample program.

	The data returned by a do_command may be different from the format of
	the request.  The response provided by each device looks like the
	following:

   <rci_reply version="1.1">
      <do_command target="myTarget">
      	<answer>123.456</answer>
      </do_command>
   </rci_request>

   The element <answer> in the above example is a field name inside the
   object registered as the reply object for this do_command target.

	It is up to your web services application to extract and parse the
	reply XML.


	This sample registers three do_command targets.  The first one accepts
	commands and replies in the same structural format, and the other
	shows how to specify a different reply than request.  The third shows
	how to make a command that takes no parameters.

	To test this, POST the following data to the iDigi server:

	<sci_request version="1.0">
	   <send_message>
	      <targets>
	        <!-- fill in with correct device ID (last 6 digits) -->
	        <device id="00000000-00000000-0090C2FF-FF000000"/>
	      </targets>
	      <rci_request version="1.1">
				<do_command target="toupper">
               <value>Hello world, to be returned uppercase!</value>
				</do_command>
	      </rci_request>
	   </send_message>
	</sci_request>

	<sci_request version="1.0">
	   <send_message>
	      <targets>
	        <!-- fill in with correct device ID (last 6 digits) -->
	        <device id="00000000-00000000-0090C2FF-FF000000"/>
	      </targets>
	      <rci_request version="1.1">
				<do_command target="add">
               <a>99</a>
               <b>3000</b>
				</do_command>
	      </rci_request>
	   </send_message>
	</sci_request>

	<sci_request version="1.0">
	   <send_message>
	      <targets>
	        <!-- fill in with correct device ID (last 6 digits) -->
	        <device id="00000000-00000000-0090C2FF-FF000000"/>
	      </targets>
	      <rci_request version="1.1">
				<do_command target="doSomething"/>
	      </rci_request>
	   </send_message>
	</sci_request>


	In the case that an error occurs, the reply XML will contain the following
	structure (or something like it) within the <do_command> element:

	<error id="16" from="egrp">
		<desc>Custom error string</desc>
		<hint>Overflow</hint>
	</error>

	This is created by any web_error() calls in a guard expression.  The
	string parameter to web_error() appears in the <hint> element.

*/


// You can comment out either or both of the following, to make a more spartan
// demo.
//#define IDIGI_USE_TLS		// Required to include TLS support
#define IDIGI_USE_ADDP		// Required to include ADDP support

#define IDIGI_PRODUCT "cloud_do_command.c"
#define IDIGI_VENDOR "Digi International Inc."
#define IDIGI_VENDOR_ID "1234"
#define IDIGI_FIRMWARE_ID "1.01.00"
#define IDIGI_CONTACT "support@digi.com"
#define IDIGI_LOCATION "Planet Earth"
#define IDIGI_DESCRIPTION "Simple iDigi demo"
#define IDIGI_SERVER "my.devicecloud.com"

// Store non-volatile configuration data in the userID block, via the
// Simple UserID Block FileSystem.  You can use SUBFS to also store a limited
// amount of non-iDigi application configuration data.
#define IDIGI_USE_SUBFS
#define SUBFS_RESERVE_START 0
#define SUBFS_RESERVE_END 0

#define ADDP_PASSWORD	"rabbit"

// Comment this out if the Real-Time Clock is set accurately.
#define X509_NO_RTC_AVAILABLE

// Enable the following debugging/diagnostic options when
// developing new applications.
/*
#define IDIGI_DEBUG
#define DCRTCP_DEBUG
#define ADDP_DEBUG
#define PKTDRV_DEBUG
#define IDIGI_VERBOSE
#define DNS_VERBOSE
#define RABBITWEB_VERBOSE
*/
#define IDIGI_IFACE_VERBOSE	// This prints interface status when it changes.

// Required only if using TLS, but not using any static Zserver resources.
#define SSPEC_NO_STATIC

#use "Device_Cloud.lib"



/*==========================================================================
 Here is where we set up the do_command request and reply data structures.
 Note the use of the RabbitWeb compiler enhancement (#web) which is
 necessary for this to work.
 ==========================================================================*/

// You don't need to use macros like this, but it helps to keep everything
// consistent...
#define TARGET_NAME_1  "toupper"
#define REQ_VAR_NAME_1  string
#define REPLY_VAR_NAME_1  REQ_VAR_NAME_1

#define TARGET_NAME_2  "add"
#define REQ_VAR_NAME_2  addOperands
#define REPLY_VAR_NAME_2  addResult

#define TARGET_NAME_3  "doSomething"
#define REQ_VAR_NAME_3  dummy
#define REPLY_VAR_NAME_3  doSomethingResult


// Request (and response which is the same) for do_command target #1
struct {
	char value[81];
} REQ_VAR_NAME_1;
#web REQ_VAR_NAME_1


// Request (and different response) for do_command target #2.
// This example shows use of RabbitWeb guard functions to validate the
// request.  In this case, where the objective is to perform integer
// addition of the fields a and b, we flag an error if the result
// would overflow (in unsigned 16-bit arithmetic).  An elegant way of
// doing this is to craft the guar expression as follows:
//   (expression || web_error("hint string"))
// where "expression" is an expression which returns true if the result is
// OK.
struct {
	unsigned a;
	unsigned b;
} REQ_VAR_NAME_2;
#web REQ_VAR_NAME_2 ((unsigned long)$REQ_VAR_NAME_2.a + \
                     (unsigned long)$REQ_VAR_NAME_2.b < 65536uL || \
                       web_error("Overflow"))

struct {
	unsigned sum;
} REPLY_VAR_NAME_2;
#web REPLY_VAR_NAME_2

// Request (and different response) for do_command target #3.
// Request object is a dummy int.  This is required for commands which
// take no parameter data.  The int is always set to zero when the
// command is received.  It is necessary so that an update callback can
// be associated with it.
// Reply is a more complicated structure, to demonstrate how this maps
// to the XML reply.
int	REQ_VAR_NAME_3;
#web REQ_VAR_NAME_3

struct {
	float answer;
	int sensible;
	struct {
		int x, y;
	} arrayOfStruct[4];
	int simpleArray[4];
} REPLY_VAR_NAME_3;
#web REPLY_VAR_NAME_3

/*==========================================================================
 Here is where we set up the callback functions which are responsible for
 processing the requests, and updating any reply data.

 This makes use of the RabbitWeb #web_update construct.  This works because
 the request which is received from the server is transformed into a
 RabbitWeb structure update transaction.  If the transaction executes
 successfully, it calls the registered update function.

 With iDigi do_commands, the update function will normally perform the
 required operation, then set fields in the reply structure, which is then
 formatted as XML and returned to the requesting server.
 ==========================================================================*/
void action_string(void);
void action_add(void);
void action_doSomething(void);
#web_update REQ_VAR_NAME_1 action_string
#web_update REQ_VAR_NAME_2 action_add
#web_update REQ_VAR_NAME_3 action_doSomething

void action_string(void)
{
	// Since this target is called "toupper", let's do it...
	int i;
	printf("\naction_string()\n\n");
	for (i = 0; i < sizeof(REQ_VAR_NAME_1.value); ++i)
		REQ_VAR_NAME_1.value[i] = toupper(REQ_VAR_NAME_1.value[i]);
}

void action_add(void)
{
	REPLY_VAR_NAME_2.sum = REQ_VAR_NAME_2.a + REQ_VAR_NAME_2.b;
	printf("\naction_add(): %u = %u+%u\n\n",
		REPLY_VAR_NAME_2.sum,
		REQ_VAR_NAME_2.a,
		REQ_VAR_NAME_2.b
		);
}

void action_doSomething(void)
{
	int i;

	printf("\naction_doSomething()\n\n");
	REPLY_VAR_NAME_3.answer = 42.0;
	REPLY_VAR_NAME_3.sensible = 1;
	for (i = 0; i < 4; ++i) {
		REPLY_VAR_NAME_3.arrayOfStruct[i].x = i+1;
		REPLY_VAR_NAME_3.arrayOfStruct[i].y = (i+1)*(i+1);
		REPLY_VAR_NAME_3.simpleArray[i] = (i+1)*(i+1)*(i+1);
	}
}




void main()
{
	int rc;

	if (idigi_init())
		exit(1);

	// This is where we actually register the cursom do_commands.
	// Note use of the '#' (enquote) operator to turn the variable
	// names into strings.  [The double macro indirection is required to
	// get this to work!].  NOTE: this registration must be done after
	// idigi_init(), since idigi_init() clears the do_command table.
#define MKS(x) #x
#define MKSTRING(x) MKS(x)
	idigi_register_target(TARGET_NAME_1,
		MKSTRING(REQ_VAR_NAME_1), MKSTRING(REPLY_VAR_NAME_1));
	idigi_register_target(TARGET_NAME_2,
		MKSTRING(REQ_VAR_NAME_2), MKSTRING(REPLY_VAR_NAME_2));
	idigi_register_target(TARGET_NAME_3,
		MKSTRING(REQ_VAR_NAME_3), MKSTRING(REPLY_VAR_NAME_3));



_restart:

	do {
		rc = idigi_tick();
	} while (!rc);

	printf("Final rc = %d\n", rc);
	if (rc == -NETERR_ABORT) {
		// RCI reboot request was received.  Normally we would use this
		// to shut down cleanly and reboot the board.
		printf("Rebooting via exit(0)!\n");
		exit(0);
	}

	goto _restart;

}