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

   nflash_log.c

	This program is used with RCM4000 series controllers equipped with a
   supported NAND flash device.


   Description
   ===========
   This program runs a simple web server and stores a log of hits in the nand
   flash.  This log can be viewed and cleared from a web browser.

   The user should be aware of that this sample program does not exhibit ideal
   behavior in its method of writing to the nand flash.  However, the
   inefficiency due to the small amount of data written in each append operation
   is offset somewhat by the expected relative infrequency of these writes, and
   by the sample's method of "walking" through the flash blocks when appending
   data as well as when a log is cleared.

   In this example, there is little difference in the number of nand flash block
   erase operations regardless of whether the NFLASH_USEERASEBLOCKSIZE macro
   value is defined to be 0 (zero) or 1 (one).  It is slightly more efficient to
   have the nand flash driver use larger (16 KB) chunks of data, simply because
   the ratio of header information to log data is slightly lower.  See the
   nf_initDevice() function's help for more information.

   Instructions
   ============
   1. Compile and run this program.
   2. Use a web browser to open the log page (which updates the log), and view
      or clear the log.

*******************************************************************************/

// These defines redirect run mode STDIO to serial port A at 57600 baud.
#define STDIO_DEBUG_SERIAL SADR
#define STDIO_DEBUG_BAUD 57600

// This define adds carriage returns ('\r') to each newline char ('\n') when
//  sending STDIO output to a serial port.
#define STDIO_DEBUG_ADDCR

// Uncomment this define to force both run mode and debug mode STDIO to the
//  serial port specified above.
//#define STDIO_DEBUG_FORCEDSERIAL

#class auto
#if !__SEPARATE_INST_DATA__
// When compiling with separate I&D space enabled, it is often best to allow
//  the compiler more leeway in placing code in root space.
#memmap xmem
#endif

#if (_BOARD_TYPE_ & 0xFF00) == RCM4000
#use "rcm40xx.lib"	// sample library to use with this application
#endif

// if NFLASH_USEERASEBLOCKSIZE is not defined, its value defaults to 1
//  0 == use 512 byte main data page size; 1 == use 16 KB main data page size
#define NFLASH_USEERASEBLOCKSIZE 1

//#define NFLASH_VERBOSE
//#define NFLASH_DEBUG
#use "nflash.lib"	// base nand flash driver library

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#ifndef TCPCONFIG
	#define TCPCONFIG 1
#endif


/*
 *    FORM_ERROR_BUF is required for the zserver form interface in
 *    init_dateform()
 *
 */

#define FORM_ERROR_BUF  256


/*
 * Our web server as seen from the clients.
 * This should be the address that the clients (netscape/IE)
 * use to access your server. Usually, this is your IP address.
 * If you are behind a firewall, though, it might be a port on
 * the proxy, that will be forwarded to the Rabbit board. The
 * commented out line is an example of such a situation.
 */
#define REDIRECTHOST    _PRIMARY_STATIC_IP
//#define REDIRECTHOST  "proxy.domain.com:1212"


/********************************
 * End of configuration section *
 ********************************/


#use "dcrtcp.lib"
#use "http.lib"

/*
 *  REDIRECTTO is used by each ledxtoggle cgi's to tell the
 *  browser which page to hit next.  The default REDIRECTTO
 *  assumes that you are serving a page that does not have
 *  any address translation applied to it.
 *
 */

#define REDIRECTTO      myurl()

#if (_BOARD_TYPE_ & 0xFF00) == RCM4000
#ximport "Samples/RCM4000/NandFlash/pages/log.shtml"     index_shtml
#ximport "Samples/RCM4000/NandFlash/pages/rabbit1.gif"   rabbit1_gif
#ximport "Samples/RCM4000/NandFlash/nflash_log.c"   		flashlog_c
#endif

// global variables
int count;
char far *myMainBuffer;
// protected in separate battery backed /CS2 SRAM, when one is available
protected nf_device nandFlash;

//routines and global variables for accessing log in nand flash

typedef struct
{
   char marker_string[4];
   long datalen;
} log_block_header;

long log_start_block, log_end_block, log_read_block;
long log_block_datasize, log_read_offset;

//
// This routine returns the our IP address in URL format
//
char *myurl() {
	static char URL[64];
   char tmpstr[32];
   long ipval;

   ifconfig(IF_DEFAULT, IFG_IPADDR, &ipval, IFS_END);
   sprintf(URL, "http://%s/index.shtml", inet_ntoa(tmpstr, ipval));
   return URL;
}

void read_log_reset()
{
   log_read_block = log_start_block;
   log_read_offset = 0L;
}

/*
** 1) Look for a nand flash device.
** 2) If device is found, look for an existing log.
** 3) If device is found but log is not found, create a log.
*/
int init_log()
{
   auto int err, i, j;
   auto int found_start, found_end;
   auto long bufSize, ofst, page;
   auto log_block_header header;

   printf("First attempting to use socketed nand flash device init.\n");
   if (i = 1, err = nf_initDevice(&nandFlash, 1)) {
      printf("ERROR %d:  Socketed nand flash device init failed!\n\n", err);
      printf("Now attempting to use soldered-on nand flash device init.\n");
      if (i = 0, err = nf_initDevice(&nandFlash, 0)) {
         printf("ERROR %d:  Soldered-on nand flash device init failed too!\n\n",
                err);
         return -1;	// can't find a good, known device!
      }
   }

   printf("\n%s nand flash device initialized.\n", i ? "Socketed" : "Soldered");
   printf("Number of blocks:   %ld.\n", nf_getPageCount(&nandFlash));
   printf("Size of each block: %ld.\n", nf_getPageSize(&nandFlash));

   // Use a temp variable here, to avoid the possibility of _xalloc attempting
   //  to change nf_getPageSize(&nandFlash)'s value!
   bufSize = nf_getPageSize(&nandFlash);
   // Get application's main data buffer.  (Note that _xalloc will cause a run
   //  time error if there is not sufficient xmem data RAM available.)
#if _BBRAMS_LOCATION != _BBRAMS_NONE
   // If available, use the separate battery backed SRAM for the main data
   //  buffer.  This allows the nand flash driver to possibly recover a write
   //  interrupted by a power cycle.
   myMainBuffer = (char far *) _xalloc(&bufSize, 0, XALLOC_BB);
#else
   myMainBuffer = (char far *) _xalloc(&bufSize, 0, XALLOC_ANY);
#endif

   //calculate space for data in a block
   log_block_datasize = nf_getPageSize(&nandFlash) - sizeof(log_block_header);

   //find beginning of log(if it exists)
   found_start = 0;
   for (page = 0L; page < nf_getPageCount(&nandFlash); ++page) {
#if NFLASH_USEERASEBLOCKSIZE
      printf("Checking block #%ld.\r", page);
#else
      if (!(page % nandFlash.erasepages)) {
         printf("Checking block #%ld.\r", page);
      }
#endif
      err = nf_readPage(&nandFlash, myMainBuffer, page);
      if (err) {
         // just skip over bad pages
         printf("Ignoring block #%ld with read error %d.\n", page, err);
      } else {
         // only check the good pages
         _f_memcpy(&header, myMainBuffer, sizeof(header));
         if (!strncmp(header.marker_string, "SLOG", 4)) {
            log_start_block = page;
            printf("Found log start at block #%ld.\n", page);
            found_start = 1;
            break;
         }
      }
   }

   if (found_start) {
      // look for end
      found_end = 0;
      for (ofst = 0L; ofst < nf_getPageCount(&nandFlash); ++ofst) {
         page = (log_start_block + ofst) % nf_getPageCount(&nandFlash);
#if NFLASH_USEERASEBLOCKSIZE
         printf("Checking block #%ld.\r", page);
#else
         if (!(page % nandFlash.erasepages)) {
            printf("Checking block #%ld.\r", page);
         }
#endif
         if (err = nf_readPage(&nandFlash, myMainBuffer, page)) {
            // just skip over bad pages
            printf("Ignoring block #%ld with read error %d.\n", page, err);
         } else {
            // only check the good pages
            _f_memcpy(&header, myMainBuffer, sizeof(header));
            if ((header.datalen < log_block_datasize) ||
                !strncmp(header.marker_string, "ELOG", 4))
            {
               log_end_block = page;
               printf("Found log end at block #%ld.\n", page);
               found_end = 1;
               break;
            }
         }
      }
      if (!found_end) {
         printf("Log is completely contained in block #%ld.\n",log_start_block);
         log_end_block = log_start_block; //no end found, must be first block
      }
   } else {
      // no log start found, begin good block search at block 0
      log_start_block = 0L;
      while (nf_readPage(&nandFlash, myMainBuffer, log_start_block)) {
         ++log_start_block;
         log_start_block %= nf_getPageCount(&nandFlash);
      }
      log_end_block = log_start_block;
      // begin new log at first good block found
      printf("Creating a new log, starting at block #%ld.\n", log_start_block);
      memcpy(header.marker_string, "SLOG", 4);
      header.datalen = 0L;
      _f_memcpy(myMainBuffer, &header, sizeof(header));
      if (nf_writePage(&nandFlash, myMainBuffer, log_start_block)) {
         return -2;	// write error into a good block!
      }
   }
   read_log_reset();
   return 0; //init OK
}

/*
** Fill buffer with string containing next log contents up to bufsize.
*/
int read_log(far char *buffer, unsigned bufsize)
{
   auto unsigned readsize, string_index;
   auto log_block_header header;

   string_index = 0u;	// start copying to beginning of string
   while (string_index < bufsize) {
      // read current or next good block (skip bad blocks)
      while (nf_readPage(&nandFlash, myMainBuffer, log_read_block)) {
         if (log_read_block == log_end_block) {
            buffer[0] = '\0';	// signal end of log read
            return -1;	// ran out of room (read error at end of log)
         }
         ++log_read_block;
         log_read_block %= nf_getPageCount(&nandFlash);
         if (log_read_block == log_start_block) {
            buffer[0] = '\0';	// signal end of log read
            return -2;	// ran out of room (read error at wrapped start of log)
         }
      }
      _f_memcpy(&header, myMainBuffer, sizeof(header));
      if (log_read_offset >= header.datalen) {
         // reached end of log, we're done (so return an empty buffer now)
         break;
      }
      if ((header.datalen-log_read_offset) >= (long) (bufsize-string_index)) {
         // can fill string with data from this buffer
         readsize = bufsize - string_index;
      } else {
         // read to end of data
         readsize = (unsigned) (header.datalen - log_read_offset);
      }
      _f_memcpy(buffer + string_index,
                myMainBuffer + (long) sizeof(header) + log_read_offset,
                readsize);
      log_read_offset += readsize;
      string_index += readsize;
      if (log_read_offset >= header.datalen) {
         if (log_read_block == log_end_block) {
            // end of log, we're done (except to return empty buffer next time)
            break;
         } else {
            // move on to log's next block
            ++log_read_block;
            log_read_block %= nf_getPageCount(&nandFlash);
            log_read_offset = 0L;
         }
      }
   }
   buffer[string_index] = '\0';	// add NULL to end of string
   return (int) string_index;
}

/*
** 1) Clear existing log by removing its header.
** 2) Start new log in next available good block.
*/
void clear_log()
{
   auto log_block_header header;

   // kill start block header
   memset(&header, 0, sizeof(header));
   _f_memcpy(myMainBuffer, &header, sizeof(header));
   nf_writePage(&nandFlash, myMainBuffer, log_start_block);
   // start at next 'free' good block
   do {
      ++log_start_block;
      log_start_block %= nf_getPageCount(&nandFlash);
   } while (nf_readPage(&nandFlash, myMainBuffer, log_start_block));
   log_end_block = log_start_block;
   // set up header for new start block
   memcpy(header.marker_string, "SLOG", 4);
   header.datalen = 0L;
   _f_memcpy(myMainBuffer, &header, sizeof(header));
   nf_writePage(&nandFlash, myMainBuffer, log_start_block);
}

/*
** Append a new log entry to an existing log.
*/
int append_log(far char *buffer)
{
   auto unsigned string_index, string_size, writesize;
   auto long test_end_block;
   auto log_block_header header;

   string_size = strlen(buffer);
   string_index = 0u;

   if (nf_readPage(&nandFlash, myMainBuffer, log_end_block)) {
      return -1;	// read error
   }
   _f_memcpy(&header, myMainBuffer, sizeof(header));
   while (string_index < string_size) {
      if ((log_block_datasize - header.datalen) >=
          (long) (string_size - string_index))
      {
         // rest of string can fit into this block
         writesize = string_size - string_index;
      } else {
         // fill this block and go to next
         writesize = (unsigned) (log_block_datasize - header.datalen);
         if (log_end_block != log_start_block) {
            // remove end block marker
            memset(&header, 0, 4);
         }
      }
      _f_memcpy(myMainBuffer + (long) sizeof(header) + header.datalen,
                buffer + string_index, writesize);
      header.datalen += writesize;
      _f_memcpy(myMainBuffer, &header, sizeof(header)); //update header
      if (nf_writePage(&nandFlash, myMainBuffer, log_end_block)) {
         return -2;	// write error
      }
      string_index += writesize;
      if (header.datalen == log_block_datasize) {
         // move end to next 'free' good block
         test_end_block = log_end_block;
         do {
            ++test_end_block;
            test_end_block %= nf_getPageCount(&nandFlash);
            if (test_end_block == log_start_block) {
               return -3; // ran out of room
            }
         } while (nf_readPage(&nandFlash, myMainBuffer, test_end_block));
         // next block
         log_end_block = test_end_block;
         memcpy(header.marker_string, "ELOG", 4);
         header.datalen = 0L;
         _f_memcpy(myMainBuffer, &header, sizeof(header)); // update header
         if (nf_writePage(&nandFlash, myMainBuffer, log_end_block)) {
            return -4;	// write error in new block
         }
      }
   }
   return 0; //write OK
}

/*
 *    log_cgi(HttpState*)
 *
 *    This cgi function is called from a log.shtml #exec ssi.
 *    Each time the page is requested it calls this function which
 *    records the IP address and time of the access.
 *
 */

int log_cgi(HttpState* state)
{
   auto int x;
   static far char buffer[512];
   auto int my_xpc;
   auto struct tm time;
   auto sockaddr sock_addr;

   #GLOBAL_INIT { count=0; }

   _f_memset(buffer, 0, sizeof(buffer));

   x=sizeof(sock_addr);
   getpeername((sock_type*)&state->s, &sock_addr,&x);

   tm_rd(&time);
   sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d - %d.%d.%d.%d<br>",
           tm_mon2month(time.tm_mon),
           time.tm_mday,
           time.tm_year + 1900,
           time.tm_hour,
           time.tm_min,
           time.tm_sec,
           *(((char*)&sock_addr.s_ip) + 3),
           *(((char*)&sock_addr.s_ip) + 2),
           *(((char*)&sock_addr.s_ip) + 1),
           *(((char*)&sock_addr.s_ip) + 0));

   x = append_log(buffer);
   count++;
   return 1;
}

/*
 *    int resetlog_cgi(HttpState*)
 *
 *    This cgi function clears the log.
 *
 */

int resetlog_cgi(HttpState* state)
{
   count=0;

   clear_log();

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}

/*
 *    int printlog_cgi(HttpState*)
 *
 *    This cgi function prints the contents of the log.  It uses
 *    the HttpState substate, buffer, offset, and p variables to
 *    maintain the current state.
 *
 */

#define PRTLOG_INIT     0
#define PRTLOG_HEADER   1
#define PRTLOG_PRTITEM  2
#define PRTLOG_FOOTER   3

const char prtlog_header[] =
   "<html><head><title>prtlog.cgi</title></head><body>\r\n" \
   "<h2>Web Log</h2>\r\n";

const char prtlog_footer[] = "</body></html>\r\n";

int printlog_cgi(HttpState* state)
{
   auto int bytes_written;

   switch(state->substate) {
      case PRTLOG_INIT:
      	// Temporarily cast const away since the p member in HttpState is a
         // general purpose pointer
         state->p=(char*)&prtlog_header[0];
         state->substate=PRTLOG_HEADER;
         read_log_reset();

         // intentionally no break

      case PRTLOG_HEADER:
         if(*state->p) {
            bytes_written=sock_fastwrite(&state->s,state->p,strlen(state->p));
            if(bytes_written>0)
               state->p+=bytes_written;
         } else {
            state->p=NULL;
            state->offset=0;
            state->substate=PRTLOG_PRTITEM;
         }
         break;

      case PRTLOG_PRTITEM:
         if(state->p==NULL || *state->p=='\0' || *state->p=='\xff') {
            read_log(state->buffer, 128u);

            if(strlen(state->buffer))
            {
               //still reading out log contents
               state->p=state->buffer;
               state->offset = 0;
            } else {
               state->offset=0;
               // Temporarily cast const away since the p member in HttpState is
               // a general purpose pointer
               state->p=(char*)&prtlog_footer[0];
               state->substate=PRTLOG_FOOTER;
            }
            state->offset++;
         } else {
            bytes_written=sock_fastwrite(&state->s,state->p,strlen(state->p));
            if(bytes_written>0) {
               state->p+=bytes_written;
            }
         }
         break;

      case PRTLOG_FOOTER:
         if(*state->p) {
            bytes_written=sock_fastwrite(&state->s,state->p,strlen(state->p));
            if(bytes_written>0)
               state->p+=bytes_written;
         } else
            return 1;         // done

         break;
   }
   return 0;
}

const HttpType http_types[] =
{
   { ".shtml", "text/html", shtml_handler},  // ssi
   { ".cgi", "", NULL},                      // cgi
   { ".gif", "image/gif", NULL},             // gif
   { ".c", "text/plain", NULL}
};

const HttpSpec http_flashspec[] =
{
   { HTTPSPEC_FILE,  "/",              index_shtml,   NULL, 0, NULL, NULL},
   { HTTPSPEC_FILE,  "/index.shtml",   index_shtml,   NULL, 0, NULL, NULL},
   { HTTPSPEC_FILE,  "/rabbit1.gif",   rabbit1_gif,   NULL, 0, NULL, NULL},
   { HTTPSPEC_FILE,  "nflash_log.c",   flashlog_c,    NULL, 0, NULL, NULL},

   { HTTPSPEC_VARIABLE, "count", 0, &count, INT16, "%d", NULL},

   { HTTPSPEC_FUNCTION, "/log.cgi",       0, log_cgi,       0, NULL, NULL},
   { HTTPSPEC_FUNCTION, "/printlog.cgi",  0, printlog_cgi,  0, NULL, NULL},
   { HTTPSPEC_FUNCTION, "/resetlog.cgi",  0, resetlog_cgi,  0, NULL, NULL},
};

/*
 *    void setdate();
 *
 *    Change the board time to the user specified time
 *    and redirect the user to the front page.
 *
 */

int hour, minute, second, month, day, year, date_lock;

int set_date(HttpState* state)
{
   auto struct tm time;

   if (state->cancel) {
      return 1;
   }
   time.tm_sec=second;
   time.tm_min=minute;
   time.tm_hour=hour;
   time.tm_mon=month2tm_mon(month);
   time.tm_mday=day;
   time.tm_year=year-1900;

   tm_wr(&time);
   SEC_TIMER=mktime(&time);

   date_lock=0;

   cgi_redirectto(state,REDIRECTTO);
   return 0;
}


/*
 *    void update_date()
 *
 *    Update the date from the board clock
 *
 */

void update_date()
{
   auto struct tm time;

   #GLOBAL_INIT { date_lock=0; }

   if(date_lock) return;

   tm_rd(&time);

   month=tm_mon2month(time.tm_mon);
   day=time.tm_mday;
   year=time.tm_year+1900;
   hour=time.tm_hour;
   minute=time.tm_min;
   second=time.tm_sec;
}

/*
 *    int lock_date()
 *
 *    Lock the date structure so we can update it atomically.
 *
 */

int lock_date(HttpState* state)
{
   date_lock=1;
   return 1;
}

/*
 *    void init_dateform()
 *
 *    Initialize the date form using ZServer primitives.
 *
 */

void init_dateform()
{
    auto int var,form;
    static FormVar dateform[7];

    /*
     *   Set up the date form.  To set up your own form you follow a
     *   similar set of steps.  First call sspec_addform.  Then add
     *   the variables creating the variable and adding it to the form.
     *   If you want to restrict the values of the variables you can
     *   use the sspec_setfvlen, sspec_setfvrange, or define a custom
     *   validation function with sspec_fvcheck.
     *
     */

    form = sspec_addform("date.html", dateform, 7, SERVER_HTTP);

    // Set the title of the form
    sspec_setformtitle(form, "Set Date");

    var = sspec_addvariable("hour", &hour, INT16, "%02d", SERVER_HTTP);
    var = sspec_addfv(form, var);
    sspec_setfvname(form, var, "Hour");
    sspec_setfvlen(form, var, 2);
    sspec_setfvrange(form, var, 0, 24);

    var = sspec_addvariable("minute", &minute, INT16, "%02d", SERVER_HTTP);
    var = sspec_addfv(form, var);
    sspec_setfvname(form, var, "Minute");
    sspec_setfvlen(form, var, 2);
    sspec_setfvrange(form, var, 0, 60);

    var = sspec_addvariable("second", &second, INT16, "%02d", SERVER_HTTP);
    var = sspec_addfv(form, var);
    sspec_setfvname(form, var, "Second");
    sspec_setfvlen(form, var, 2);
    sspec_setfvrange(form, var, 0, 60);

    var = sspec_addvariable("month", &month, INT16, "%02d", SERVER_HTTP);
    var = sspec_addfv(form, var);
    sspec_setfvname(form, var, "month");
    sspec_setfvlen(form, var, 2);
    sspec_setfvrange(form, var, 0, 12);

    var = sspec_addvariable("day", &day, INT16, "%02d", SERVER_HTTP);
    var = sspec_addfv(form, var);
    sspec_setfvname(form, var, "day");
    sspec_setfvlen(form, var, 2);
    sspec_setfvrange(form, var, 0, 31);

    var = sspec_addvariable("year", &year, INT16, "%04d", SERVER_HTTP);
    var = sspec_addfv(form, var);
    sspec_setfvname(form, var, "year");
    sspec_setfvlen(form, var, 4);
    sspec_setfvrange(form, var, 1990, 2010);

    /*
     *   lock_date sets a flag that disables the automatic updating
     *   of the hour, minute, second, month, day, and year variables.
     *   The prolog gets called after the form is parsed correctly,
     *   but before the variables are updated.  The set_date function
     *   gets called after the variables are updated.  The lock_date
     *   function is necessary because the update_date function in
     *   main could be called between the time the variables are
     *   updated and the prolog completes its processing.
     *
     */

    var = sspec_addfunction("lock_date", lock_date, SERVER_HTTP);
    sspec_setformprolog(form, var);
    var = sspec_addfunction("set_date", set_date, SERVER_HTTP);
    sspec_setformepilog(form, var);

}

/*
 *    main()
 *
 *    Initialize the flash, set up the date form and start the web
 *    server.
 *
 */

void main()
{
   _sysIsSoftReset();	// restore any protected variables

   sock_init_or_exit(1);
   http_init();
   init_dateform();
   init_log();

   tcp_reserveport(80);       // mark port 80 as a server port.

   for(;;) {
      update_date();
      http_handler();
   }
}