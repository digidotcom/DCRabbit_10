/*******************************************************************************

	Samples/RemoteProgramUpdate/upload_firmware.c
	Digi International, Copyright (c) 2009.  All rights reserved.

	Application Note 421 contains full documentation on using the Remote Program
	Update library and samples.

	Demonstrate using the HTTP file upload feature and board_update.lib to
	update the system firmware.  Stores the firmware in a temporary
	location until upload is complete and verified.

	You will need to uncomment one of the four BU_TEMP_USE_ macros at the start
	of the program to select the location for the temp firmware.

	The login to upload firmware is user: admin  password: upload

	Change the user/password by editing ADMIN_USER and ADMIN_PASS in the
	configuration section below.

   NOTE: For the RCM5750, you must include this macro definition in the
   Global Macro Definitions of the Project Options:  BU_ENABLE_MINILOADER

   To use Power-Fail Safe updates on boards with a serial boot flash, define
   BU_ENABLE_SECONDARY in the Global Macro Definitions of the Project Options.
   PFS RPU works by storing two complete firmware images on the boot flash.
   Updates are done by copying new firmware to the non-boot image and then
   using an atomic flash write to enable that firmware for booting.

*******************************************************************************/

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

#define ADMIN_USER "admin"
#define ADMIN_PASS "upload"

/*
 * Unless BU_ENABLE_SECONDARY was defined in the Global Macro Definitions,
 * define one of the following macros to select the temp storage location.
 * On the RCM5600W, the only option is to write new firmware directly to the
 * boot flash.  This is dangerous, as a power failure during the download will
 * result in a board that needs to be reloaded with the RFU or some other
 * direct serial connection on the programming port.
 */
//	#define BU_TEMP_USE_FAT				// use file on FAT filesystem
//	#define BU_TEMP_USE_SBF				// use unused portion of serial boot flash
//	#define BU_TEMP_USE_SFLASH			// use serial data flash (without FAT)
//	#define BU_TEMP_USE_DIRECT_WRITE	// write directly to boot firmware image

/*
 * If using the serial data flash as a target (BU_TEMP_USE_SFLASH), you can
 * specify a page offset (other than the default of 0) for storing the
 * temporary firmware image.
 */
//	#define BU_TEMP_PAGE_OFFSET 0

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

//	Uncomment any combination of these macros to enable verbose debugging
// messages.
//	#define VERBOSE						// messages from this sample
//	#define HTTP_VERBOSE					// messages from web server
//	#define BOARD_UPDATE_VERBOSE		// messages from Remote Program Update

/********************************
 * End of configuration section *
 ********************************/

// Make sure an option has been enabled.
#if ! defined BU_TEMP_USE_FAT && \
	 ! defined BU_TEMP_USE_SBF && \
	 ! defined BU_TEMP_USE_SFLASH && \
	 ! defined BU_TEMP_USE_DIRECT_WRITE && \
	 ! defined BU_ENABLE_SECONDARY
#fatal "You must uncomment a BU_TEMP_USE_xxx macro at the top of this sample."
#endif

#define  STDIO_DEBUG_SERIAL   SADR
#define	STDIO_DEBUG_BAUD		115200
#define	STDIO_DEBUG_ADDCR

#define USE_HTTP_UPLOAD		// Required for this demo, to include upload code.

#define DISABLE_DNS			// No name lookups required

// Define this because we are using a static rule table.
#define SSPEC_FLASHRULES

#ifdef BU_TEMP_USE_FAT
	// This value is required to enable FAT blocking mode for ZServer.lib
	#define FAT_BLOCK

	#define FAT_USE_FORWARDSLASH		// Necessary for zserver.

	#use "fat16.lib"							// Must use FAT before http/zserver
#endif

// Make use of RabbitWeb's scripting commands in HTML files
#define USE_RABBITWEB 1

// Only one server is really needed for the HTTP server as long as
// tcp_reserveport() is called on port 80.
#define HTTP_MAXSERVERS 1

// HTTP upload works better with a large buffer.  HTTP.LIB will xalloc
// HTTP_SOCK_BUF_SIZE bytes of xmem for each HTTP server socket
#define HTTP_SOCK_BUF_SIZE (12 * MAX_MTU)

// use DIGEST authentication (more secure than BASIC authentication)
#define USE_HTTP_DIGEST_AUTHENTICATION 1

#memmap xmem

#use "dcrtcp.lib"
#use "http.lib"
#use "board_update.lib"

// function prototype for cgi function to handle file upload
int firmware_upload(HttpState * s);

#ximport "pages/upload.zhtml"    index_zhtml

// Define a group bit for updating resources...
#define ADMIN_GROUP	0x0002

// This table maps file extensions to the appropriate "MIME" type.  This is
// needed for the HTTP server.
SSPEC_MIMETABLE_START
	SSPEC_MIME_FUNC(".zhtml", "text/html", zhtml_handler),
	SSPEC_MIME(".htm", "text/html"),
	SSPEC_MIME(".html", "text/html"),
	SSPEC_MIME(".gif", "image/gif"),
	SSPEC_MIME(".cgi", "")
SSPEC_MIMETABLE_END

// define the prompt sent to users when asked for a username and password
#define REALM "Firmware Upload"

// This is the access permissions "rule" table.  All pages require a password.
SSPEC_RULETABLE_START
	// Create a rule that only users in ADMIN_GROUP (0x0002) can access pages
	// starting with "/fw/".
   // REALM ("Firmware Upload") is used as the resource realm, which
   // your browser will use to prompt you for a userid and password.
   // Only users in group ADMIN_GROUP can read and write these locations.
   // Only the HTTP server can access this file, and digest (i.e. challenge/
   // response) user/password authentication is specified.
	SSPEC_MM_RULE("/fw/", REALM, ADMIN_GROUP, ADMIN_GROUP,
		SERVER_HTTP, SERVER_AUTH_DIGEST, NULL)
SSPEC_RULETABLE_END

// redirects http://hostname/ to upload page for new firmware
int root_htm(HttpState *http)
{
	cgi_redirectto( http, "/fw/index.zhtml");
	return CGI_OK;
}

// setting up the static Server table
SSPEC_RESOURCETABLE_START
	// Redirect to the /fw/ directory.  In a typical application, you'd have
	// other web pages and entries in this table, and you'd just add the /fw/
	// entries.
   SSPEC_RESOURCE_FUNCTION("/", root_htm),

// "/fw/index.zhtml" - This is the web page which is initially presented.
// It contains the form to be filled out.
	SSPEC_RESOURCE_XMEMFILE("/fw/index.zhtml", index_zhtml),
/*
	"upload.cgi" - Resource name as specified in the HTML for handling the POST.
	firmware_upload - Name of the upload CGI handler.
	"newPages" - The realm associated with upload.cgi.
	ADMIN_GROUP - Read permission must be limited to this group.  The CGI itself
					only needs read permission, but it should be the same group bits
					as specified for the write permission for the target resource
					files (see ruletable entry above).
	0x0000 - No write permission since writing to a CGI does not make sense.
	SERVER_HTTP - Only the HTTP server accesses CGIs.
	SERVER_AUTH_DIGEST - Use the same authentication technique as for the files.
*/
	SSPEC_RESOURCE_P_CGI("/fw/upload.cgi", firmware_upload,
                    REALM, ADMIN_GROUP, 0x0000, SERVER_HTTP, SERVER_AUTH_DIGEST)
SSPEC_RESOURCETABLE_END

void install_firmware();

struct {
	char	running[40];		// information on the currently running firmware
	char	temp[40];			// information on the uploaded file
	int	show_temp;			// 1 == show info on temp firmware (set to 0 for
									// boards that only allow direct uploading)
	int	show_install;		// flag whether to show install button or not
									// 1 == show, 0 == don't show, -1 == show error
	char	error[80];
	int	install;				// flag to initiate install and to indicate whether
									// there was an error (0 == normal, 1 == initiate
									// install, -1 == error attempting install)
} firmware;
#web firmware
#web firmware.install
#web_update firmware.install install_firmware

#ifdef BU_TEMP_USE_FAT
void unmount_all()
{
	int i, rc;

   for (i = 0; i < num_fat_devices * FAT_MAX_PARTITIONS;
   	i += FAT_MAX_PARTITIONS)
   {
      if (fat_part_mounted[i])
      {
         rc = fat_UnmountDevice(fat_part_mounted[i]->dev);
         if (rc < 0)
         {
            printf("Unmount Error on %c: %ls\n", 'A' + i, strerror(rc));
         }
      }
   }
}
#endif

void install_firmware()
{
	int i, result;

	printf ("user clicked install/reboot button\n");

	#ifdef BU_TEMP_USE_DIRECT_WRITE
		// Firmware is already installed, just need to shut down HTTP server
		// and reboot.
      http_shutdown(0);
		exit(0);
	#endif

   result = buOpenFirmwareTemp( BU_FLAG_NONE);

	if (result)
	{
      sprintf (firmware.error, "Error %d opening firmware.", result);
      firmware.install = -1;
	}
	else
	{
	   result = buVerifyFirmwareBlocking();
	   if (result)
	   {
	      while (buCloseFirmware() == -EBUSY);
	      sprintf (firmware.error, "Error %d installing firmware.", result);
	      firmware.install = -1;
	   }
	   else
	   {
	      // shut down HTTP server and install
	      http_shutdown(0);
	      if (buInstallFirmware())
	      {
	         // on error, restart http server
	         http_init();
	      }
	      else
	      {
	         // Install was successful, reboot into new firmware.
	         exit( 0);
	      }
	   }
	}
}

// need to call this after upload completes...
void update_webvars()
{
	firmware_info_t		fi;
	int i, err;

	// load information on the current firmware
	fiProgramInfo( &fi);
   sprintf( firmware.running, "%s v%u.%02x", fi.program_name,
      fi.version >> 8, fi.version & 0xFF);

	#ifdef BU_TEMP_USE_DIRECT_WRITE
		// no temp firmware to show information about
		firmware.show_temp = 0;
	#else
		firmware.show_temp = 1;
	#endif

   // load information on firmware stored in temporary location
   err = buOpenFirmwareTemp( BU_FLAG_NONE);

   if (!err)
   {
	   // buGetInfo is a non-blocking call, and may take multiple attempts
	   // before the file is completely open.
	   i = 0;
	   do {
	      err = buGetInfo(&fi);
	   } while ( (err == -EBUSY) && (++i < 20) );
   }
   if (err)
   {
      sprintf( firmware.temp, "Error %d verifying firmware", err);
      firmware.show_install = -1;
   }
   else
   {
      sprintf( firmware.temp, "%s v%u.%02x", fi.program_name,
         fi.version >> 8, fi.version & 0xFF);
      // firmware is good, try to verify it
      err = buRewindFirmware();
      if (!err)
      {
         err = buVerifyFirmwareBlocking();
      }
      // if no errors, show a button (1) so user can try to install this
      // firmware, or show an error message that image is corrupted (-1)
      firmware.show_install = err ? -1 : 1;
   }
   while (buCloseFirmware() == -EBUSY);

	firmware.install = 0;
	*firmware.error = '\0';
}

int main()
{
	int rc, i;
	char buf[20];
   int uid;

	printf( "Initializing network...\n");

	// Start network and wait for interface to come up (or error exit).
	sock_init_or_exit(1);

#ifdef BU_TEMP_USE_FAT
	printf( "Initializing filesystem...\n");
	// Note: sspec_automount automatically initializes all known filesystems.
   rc = sspec_automount(SSPEC_MOUNT_ANY, NULL, NULL, NULL);
   if (rc)
   {
   	printf( "Failed to mount filesystem, rc=%d\n", rc);
   	exit (rc);
	}
   if (! fat_part_mounted[0])
   {
		printf("Couldn't mount A:\n");
		exit(-EIO);
   }

	// register a function to unmount all FAT volumes on exit
   atexit( unmount_all);
#endif

   printf( "Setting up userids...\n\n");
   // Create a user ID
   uid = sauth_adduser(ADMIN_USER, ADMIN_PASS, SERVER_HTTP);
   if (uid < 0)
   {
   	printf( "Failed to create userid, rc=%d\n", uid);
   	exit (uid);
	}

   // Ensure that that user is a member of ADMIN_GROUP
   sauth_setusermask(uid, ADMIN_GROUP, NULL);

   // Also need to assign individual write access.
   sauth_setwriteaccess(uid, SERVER_HTTP);

   printf( "Userid created successfully: use '%s' with password '%s'\n\n",
      ADMIN_USER, ADMIN_PASS);

	printf( "Initialize RabbitWeb variables...\n\n");
	update_webvars();

   http_init();

/*
 *  tcp_reserveport causes the web server to maintain pending requests
 * whenever there is not a listen socket available
 *
 */
   tcp_reserveport(80);

   printf( "Ready: point your browser to http://%s/\n\n",
   	inet_ntoa(buf, MY_ADDR(IF_DEFAULT)));
   printf( "\nPress any key to bring down the server cleanly.\n");

   while (! kbhit())
   {
      http_handler();
   }

   return 0;
}

// Firmware Upload function, based on http_defaultCGI from http.lib

// Values for http_getState()
#define FW_UPLOAD_INIT			0	// Initial state, set by server (must be 0)
#define FW_UPLOAD_NORM			1	// Normal state (reading client data)

// http_setCond() state variables
#define FW_UPLOAD_ERROR			0	// Error code (<0)
#if HTTP_MAX_COND < 1
	#fatal "HTTP_MAX_COND must be >= 1 if using firmware_upload()"
#endif

#ifndef HTTP_UPLOAD_SSL_SUPRESS_WARNING
#ifdef __SSL_LIB__
	#warns "HTTP Upload is not currently compatible with SSL. Make sure that"
	#warns "you are not using HTTP upload on SSL-secured ports. To eliminate"
	#warns "this warning, include '#define HTTP_UPLOAD_SSL_SUPRESS_WARNING'"
	#warns "in your program before the line '#use HTTP.LIB'."
#endif
#endif

int firmware_upload(HttpState * s)
{
	auto char * fname;		// Pointer to field (i.e. resource to upload) name
   auto int rc, offset, mode, wlen, error;
#ifdef VERBOSE
	static long ms;			// In verbose mode, keep track of throughput.
#endif

	if (!(mode = http_getState(s)))
	{
   	// Initial state (FW_UPLOAD_INIT).  Do things we want to do only
      // once at the start.
#ifdef VERBOSE
      printf("HTTPU: init:\n");
      printf("  HTTP version=%s\n", http_getHTTPVersion_str(s));
      printf("  HTTP method=%s\n", http_getHTTPMethod_str(s));
      printf("  Userid=%d\n", http_getContext(s)->userid);
      printf("  URL=%ls\n", http_getURL(s));
#endif
   	// First time through.  Generate HTTP header
      http_setState(s, FW_UPLOAD_NORM);		// Next state is "normal"
      http_setCond(s, FW_UPLOAD_ERROR, -1);	// Temp file hasn't been opened
      // Note: it is safe to use the getData buffer, since the first call to
      // the CGI will not have any incoming data in that buffer.
	   http_genHeader(s, http_getData(s), HTTP_MAXBUFFER,
                  200, NULL, 0,
                  "<html><head><title>Upload status</title></head><body>");
      return CGI_SEND;	// Write string to client.
   }

   // Default return code.
   rc = CGI_OK;

   // Main switch depending on the current CGI action code
	switch (http_getAction(s))
	{
   	case CGI_START:
      	// Start of a new part of the multi-part data.
      	fname = http_getField(s);
#ifdef VERBOSE
      	printf("HTTPU: START content_length=%ld\n", http_getContentLength(s));
         printf("  field name=%s\n", fname);
         printf("  disposition=%d\n", http_getContentDisposition(s));
         printf("  transfer_encoding=%d\n", http_getTransferEncoding(s));
         printf("  content_type=%s\n", http_getContentType(s));
         ms = MS_TIMER;
#endif
         if (fname[0] != '/')
         {
         	http_skipCGI(s);	// Not a file: skip to next part.
         }
         else
         {
         	// Yes, it's a file to upload
				while ( (error = buTempCreate()) == -EBUSY);
            http_setCond(s, FW_UPLOAD_ERROR, error); // Save error code
            if (error < 0)
            {
      			sprintf( http_getData(s),
                 "<h1><font color=red>%s</h1><h2>Error code %d</h2>\n",
                 "Failed: could not open file.", error);
               http_skipCGI(s);	// Skip to next part after sending this buffer
      		}
            else
            {
            	// Try writing a string to the client.  Most browsers will be
            	// able to display this straight away.  This will give some
            	// confirmation that something is happening.
	            _f_strcpy( http_getData(s), "<h2>Loading firmware...</h2>");
            }
            rc = CGI_SEND;
         }
         break;

   	case CGI_DATA:
      	// This is data to write to the resource.
         if (http_getCond(s, FW_UPLOAD_ERROR) < 0)
         {
         	// This should never happen (since we always skip data if there is
         	// an error opening the resource), but it doesn't hurt to check!
         	return 0;
			}
#ifdef VERBOSE
      	printf("HTTPU: DATA length=%d (total %ld)\n", http_getDataLength(s),
      		http_getContentLength(s));
#endif
			offset = 0;
			do {
	         wlen = buTempWrite( http_getData(s) + offset,
	            http_getDataLength(s) - offset);
	         if (wlen == -EBUSY)
	         {
	         	// ignore BUSY error and try again
					wlen = 0;
	         }
	         else if (wlen < 0)
	         {
	            #ifdef BU_TEMP_USE_DIRECT_WRITE
	               // need to restore original firmware before continuing
	               if (wlen != -ENODATA)
	               {
	               	buRestoreFirmware( 0);
	               }
	            #endif
	            http_setCond(s, FW_UPLOAD_ERROR, wlen); // Save error code
      			sprintf( http_getData(s),
                 "<h1><font color=red>%s</h1><h2>Error code %d</h2>\n",
                 "Upload failed: could not write.", error);
	            http_skipCGI(s);  // Skip to next part
	            return CGI_SEND;  // ...after sending this message
	         }
	         offset += wlen;
			} while (offset != http_getDataLength(s));
         break;

   	case CGI_END:
      	// End of this file.  Close it.
#ifdef VERBOSE
			ms = MS_TIMER - ms;
         if (ms < 1)
         {
         	ms = 1;
         }
      	printf("HTTPU: END ----------- " \
      		"actual received length=%ld, %f sec, %f bytes/sec\n",
         	http_getContentLength(s), 0.001*ms,
         	http_getContentLength(s)*1000.0/ms);
#endif
			while (buTempClose() == -EBUSY);
      	http_setCond(s, FW_UPLOAD_ERROR, -1);	// No open resource now
         update_webvars();
         sprintf(http_getData(s),
           "<h1><font color=\"#009900\">Firmware uploaded successfully.</font>"\
           "</h1><h2>%ld bytes, %s</h2>",
           http_getContentLength(s), firmware.temp);
         rc = CGI_SEND;
			break;

      case CGI_EOF:
      	// Normal end of entire set of parts.
#ifdef VERBOSE
      	printf("HTTPU: EOF (unused content=%ld) \"%ls\"\n", s->content_length,
      		http_getData(s));
#endif
			if (firmware.show_install == -1)
			{
	         _f_strcpy( http_getData(s),
					"<p>Image corrupted or not compatible with this hardware.</p>" \
	         	"<p><a href=\"/\">Home</a></p></body></html>\n");
	         #ifdef BU_TEMP_USE_DIRECT_WRITE
					// need to restore original firmware before continuing
					buRestoreFirmware( 0);
	         #endif
			}
			else if (firmware.show_install == 1)
			{
				// On boards without temp staging area, firmware is installed
				// at this point and the user can only reboot.
				sprintf( http_getData(s),
					"<form action=\"index.zhtml\" method=\"POST\">"			\
					"<input type=\"hidden\" name=\"firmware.install\" value=\"1\">" \
					"<input type=\"submit\" value=\"%s\"></form>\n",
	            #ifdef BU_TEMP_USE_DIRECT_WRITE
	               "Reboot"
	            #else
	               "Install"
	            #endif
					);
			}
			else
			{
	         _f_strcpy( http_getData(s),
	         	"<p><a href=\"/\">Home</a></p></body></html>\n");
			}

         // Send this last message, and we are done!
         rc = CGI_SEND_DONE;
         break;

      case CGI_ABORT:
      	// Server had to abort the connection.
#ifdef VERBOSE
      	printf("HTTPU: ABORT CGI\n");
#endif
			// Clean up resource if one was open.
			while (buTempClose() == -EBUSY);
         break;
   }

   // Return one of
   //  CGI_OK : normal continuation
   //  CGI_MORE : call back with CGI_CONTINUE since we hadn't finished something
   //  CGI_DONE : terminate the connection normally.
   //  CGI_SEND : send what's in the buffer
   //  CGI_SEND_DONE : send what's in the buffer, and we're done.
   return rc;
}

