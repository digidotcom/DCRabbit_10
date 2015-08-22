/************************************************************************

serial_flashlog.c

Digi International Inc., Copyright © 2007-2010.  All rights reserved.

Runs a simple web server and stores a log of hits on the home page of the
server serial flash. The log should be viewed and cleared from a browser.

             *********** IMPORTANT NOTE *************

   This program uses the serial flash directly and will overwrite
   other data stored there, such as a FAT filesystem.  Do not run
   this sample if you have any important data on the flash device.

***********************************************************************/

#class auto

#memmap xmem

#if _BOARD_TYPE_ != RCM5650W && _BOARD_TYPE_ != RCM6650W
   #fatal "Only the RCM5650W/6650W has a serial flash that can store data."
#endif

#if RCM6600W_SERIES
	#use "rcm66xxw.lib"
#else
	#use "rcm56xxw.lib"
#endif

/*
 *  RABBIT RTC EPOCH CONFIGURATION
 *  Rabbit's real time clock epoch starts on 01-Jan-1980, which is different
 *  from the UN*X epoch of 01-Jan-1970 or another common epoch of 01-Jan-1900.
 *
 *  For the purposes of this sample we allow setting any year in Rabbit's epoch.
 *  A custom application should choose its own range of allowable years.
 */

#define MY_FIRST_YEAR_SETTING 1980
#define MY_FINAL_YEAR_SETTING 2048

/*
 *  NETWORK CONFIGURATION
 *  Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 *  compile-time network configuration.
 */

#ifndef TCPCONFIG
   #define TCPCONFIG 1
#endif


/*
 *  FORM_ERROR_BUF is required for the zserver form interface in
 *  init_dateform()
 */


#define FORM_ERROR_BUF 256

/********************************
 * End of configuration section *
 ********************************/

#use "dcrtcp.lib"
#use "http.lib"
#use "sflash.lib"

/*
 *  REDIRECTTO is used by each ledxtoggle cgi's to tell the
 *  browser which page to hit next.  The default REDIRECTTO
 *  assumes that you are serving a page that does not have
 *  any address translation applied to it.
 */

#define REDIRECTTO "/index.shtml"


#ximport "pages/log.shtml"    index_shtml
#ximport "pages/rabbit1.gif"  rabbit1_gif
#ximport "serial_flashlog.c"  flashlog_c

int count;

// routines and global variables for accessing log in serial flash

typedef struct
{
   char marker_string[4];
   int datalen;
} log_block_header;

int log_start_block, log_end_block, log_block_datasize;
long log_read_offset;
sf_device my_dev;


int init_log(void)
{
   auto int err, i, j;
   auto int found_start, found_end;
   auto log_block_header header;

   sfspi_init();
   if (err = sf_initDevice(&my_dev,
                           SF_SPI_CSPORT,
                           &SF_SPI_CSSHADOW,
                           SF_SPI_CSPIN))
   {
      printf("\nERROR - Serial Flash init failed (%d)\n", err);
      return err;
   }
   printf("\nSerial Flash Initialized\n");
   printf("# of blocks: %d\n", my_dev.pages);
   printf("size of blocks: %d\n", my_dev.pagesize);

   //calculate space for data in a block
   log_block_datasize = my_dev.pagesize - sizeof (log_block_header);

   //find beginning of log (if it exists)
   found_start = 0;
   for (i = 0; i < my_dev.pages; ++i)
   {
      sf_readPage(&my_dev, 1, i);
      sf_readDeviceRAM(&my_dev,
                       paddr(&header),
                       0,
                       sizeof header,
                       SF_RAMBANK1);
      if (!strncmp(header.marker_string, "SLOG", 4))
      {
         log_start_block = i;
         printf("Found start at block %d\n", i);
         found_start = 1;
         break;
      }
   }
   if (found_start)
   {
      //look for end
      found_end = 0;
      for (i = 0; i < my_dev.pages; ++i)
      {
         j = (i + log_start_block) % (int) my_dev.pages;
         sf_readPage(&my_dev, 1, j);
         sf_readDeviceRAM(&my_dev,
                          paddr(&header),
                          0,
                          sizeof header,
                          SF_RAMBANK1);
         if (!strncmp(header.marker_string, "ELOG", 4))
         {
            log_end_block = j;
            printf("Found end at block %d\n", j);
            found_end = 1;
            break;
         }
      }
      if (!found_end)
      {
         log_end_block = log_start_block; //no end found, must be first block
      }
   }
   else
   {
      //no start found, begin new log at block 0
      memcpy(header.marker_string, "SLOG", 4);
      header.datalen = 0;
      sf_writeDeviceRAM(&my_dev,
                        paddr(&header),
                        0,
                        sizeof header,
                        SF_RAMBANK1);
      sf_writePage(&my_dev, 1, 0);
      log_start_block = 0;
      log_end_block = 0;
   }
   return 0; //init OK
}


void read_log_reset(void)
{
   log_read_offset = 0;
}


// fill buffer with string containing next log contents up to bufsize
int read_log(far char *buffer_ptr, int bufsize)
{
   auto int current_block, string_index, block_index, readsize;
   auto long buffer_paddr;
   auto log_block_header header;

   // get actual physical address even if buffer_ptr is a promoted near pointer
   buffer_paddr = (long) paddr_far(buffer_ptr);

   current_block = (log_start_block +
                    (int) (log_read_offset / log_block_datasize))
                   % (int) my_dev.pages;
   string_index = 0; //start copying to beginning of string
   block_index = (int) (log_read_offset % log_block_datasize);
   string_index = 0;
   while (string_index < bufsize)
   {
      sf_readPage(&my_dev, 1, current_block);
      sf_readDeviceRAM(&my_dev,
                       paddr(&header),
                       0,
                       sizeof header,
                       SF_RAMBANK1);
      if ((header.datalen - block_index) >= (bufsize - string_index))
      {
         //can fill string with data from this buffer
         readsize = bufsize - string_index;
         sf_readDeviceRAM(&my_dev,
                          buffer_paddr + string_index,
                          block_index + sizeof header,
                          readsize,
                          SF_RAMBANK1);
         log_read_offset += readsize;
         //filled string, were done
         buffer_ptr[bufsize] = 0; //add NULL to end
         return bufsize;
      }
      else
      {
         //go to end of block
         readsize = header.datalen - block_index;
         if (readsize)
         {
            sf_readDeviceRAM(&my_dev,
                             buffer_paddr + string_index,
                             block_index + sizeof header,
                             readsize,
                             SF_RAMBANK1);
            log_read_offset += readsize;
            string_index += readsize;
         }
         if (current_block == log_end_block)
         {
            //reached end of log
            buffer_ptr[string_index] = 0; //add NULL
            return string_index;
         }
         else
         {
            current_block = (current_block + 1) % (int) my_dev.pages;
            block_index = 0;
         }
      }
   }
   //shouldn't reach here
   printf("ERROR - read_log() failed\n");
   buffer_ptr[0] = 0; //return NULL
   return 0;
}


void clear_log(void)
{
   auto log_block_header header;

   //kill start block header
   memset(&header, 0, sizeof header);
   sf_writeDeviceRAM(&my_dev,
                     paddr(&header),
                     0,
                     sizeof header,
                     SF_RAMBANK1);
   sf_writePage(&my_dev, 1, log_start_block);
   sf_writePage(&my_dev, 1, log_end_block);
   //start at next 'free' block
   log_start_block = (log_end_block + 1) % (int) my_dev.pages;
   log_end_block = log_start_block;
   //setup header for new start block
   memcpy(header.marker_string, "SLOG", 4);
   header.datalen = 0;
   sf_writeDeviceRAM(&my_dev,
                     paddr(&header),
                     0,
                     sizeof header,
                     SF_RAMBANK1);
   sf_writePage(&my_dev, 1, log_start_block);
}


int append_log(far char *buffer_ptr)
{
   auto int string_index, string_size, writesize;
   auto long buffer_paddr;
   auto log_block_header header;

   // get actual physical address even if buffer_ptr is a promoted near pointer
   buffer_paddr = (long) paddr_far(buffer_ptr);

   string_size = strlen(buffer_ptr);
   string_index = 0;

   sf_readPage(&my_dev, 1, log_end_block);
   sf_readDeviceRAM(&my_dev,
                    paddr(&header),
                    0,
                    sizeof header,
                    SF_RAMBANK1);
   while (string_index < string_size)
   {
      if ((log_block_datasize - header.datalen) >= (string_size - string_index))
      {
         //rest of string can fit into this block
         writesize = string_size - string_index;
      }
      else
      {
         //fill this block and go to next
         writesize = log_block_datasize - header.datalen;
         if (log_end_block != log_start_block)
         {
            //remove end block marker
            memset(&header, 0, 4);
         }
      }
      sf_writeDeviceRAM(&my_dev,
                        buffer_paddr + string_index,
                        sizeof header + header.datalen,
                        writesize,
                        SF_RAMBANK1);
      header.datalen += writesize;
      //update header
      sf_writeDeviceRAM(&my_dev,
                        paddr(&header),
                        0,
                        sizeof header,
                        SF_RAMBANK1);
      sf_writePage(&my_dev, 1, log_end_block);
      string_index += writesize;
      if (string_index < string_size)
      {
         //next block
         log_end_block = (log_end_block + 1) % (int) my_dev.pages;
         if (log_end_block == log_start_block)
         {
            return -1; //ran out of room
         }
         memcpy(header.marker_string, "ELOG", 4);
         header.datalen = 0;
      }
   }
   return 0; //write OK
}

/*
 *  log_cgi(HttpState *)
 *
 *  This cgi function is called from a log.shtml #exec ssi.
 *  Each time the page is requested it calls this function which
 *  records the IP address and time of the access.
 */

int log_cgi(HttpState *state)
{
   auto int x;
   static far char buffer[512];
   auto int my_xpc;
   auto struct tm time;
   auto sockaddr sock_addr;

   #GLOBAL_INIT { count = 0; }

   _f_memset(buffer, 0, sizeof buffer);

   x = sizeof sock_addr;
   getpeername((sock_type *) &state->s, &sock_addr, &x);

   tm_rd(&time);
   sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d - %d.%d.%d.%d<br>",
           tm_mon2month(time.tm_mon),
           time.tm_mday,
           time.tm_year + 1900,
           time.tm_hour,
           time.tm_min,
           time.tm_sec,
           *(((char *) &sock_addr.s_ip) + 3),
           *(((char *) &sock_addr.s_ip) + 2),
           *(((char *) &sock_addr.s_ip) + 1),
           *(((char *) &sock_addr.s_ip) + 0));

   x = append_log(buffer);
   ++count;
   return 1;
}

/*
 *  int resetlog_cgi(HttpState *)
 *
 *  This cgi function clears the log.
 */

int resetlog_cgi(HttpState *state)
{
   count = 0;
   clear_log();
   cgi_redirectto(state, REDIRECTTO);
   return 0;
}

/*
 *  int printlog_cgi(HttpState *)
 *
 *  This cgi function prints the contents of the log.  It uses
 *  the HttpState substate, buffer, offset, and p variables to
 *  maintain the current state.
 */

#define PRTLOG_INIT		0
#define PRTLOG_HEADER	1
#define PRTLOG_PRTITEM	2
#define PRTLOG_FOOTER	3

const char prtlog_header[] =
   "<html><head><title>prtlog.cgi</title></head><body>\r\n" \
   "<h2>Web Log</h2>\r\n";

const char prtlog_footer[] = "</body></html>\r\n";

int printlog_cgi(HttpState *state)
{
   auto int bytes_written;

   switch (state->substate)
   {
   case PRTLOG_INIT:
      // Temporarily cast const away since the p member in HttpState is a
      // general purpose pointer
      state->p = (char *) &prtlog_header[0];
      state->substate = PRTLOG_HEADER;
      read_log_reset();

      // intentionally no break

   case PRTLOG_HEADER:
      if (*state->p)
      {
         bytes_written = sock_fastwrite(&state->s, state->p, strlen(state->p));
         if (bytes_written > 0)
         {
            state->p += bytes_written;
         }
      }
      else
      {
         state->p = NULL;
         state->offset = 0;
         state->substate = PRTLOG_PRTITEM;
      }
      break;

   case PRTLOG_PRTITEM:
      if (state->p == NULL || *state->p == '\0' || *state->p == '\xff')
      {
         read_log(state->buffer, 128);
         if (strlen(state->buffer))
         {
            //still reading out log contents
            state->p = state->buffer;
            state->offset = 0;
         }
         else
         {
            state->offset = 0;
            // Temporarily cast const away since the p member in HttpState is
            // a general purpose pointer
            state->p = (char *) &prtlog_footer[0];
            state->substate = PRTLOG_FOOTER;
         }
         ++state->offset;
      }
      else
      {
         bytes_written = sock_fastwrite(&state->s, state->p, strlen(state->p));
         if (bytes_written > 0)
         {
            state->p += bytes_written;
         }
      }
      break;

   case PRTLOG_FOOTER:
      if (*state->p)
      {
         bytes_written = sock_fastwrite(&state->s, state->p, strlen(state->p));
         if (bytes_written > 0)
         {
            state->p += bytes_written;
         }
      }
      else
      {
         return 1;   // done
      }
      break;
   }
   return 0;
}


const HttpType http_types[] =
{
   { ".shtml", "text/html",  shtml_handler },   // ssi
   { ".cgi",   "",           NULL },            // cgi
   { ".gif",   "image/gif",  NULL },            // gif
   { ".c",     "text/plain", NULL }
};

const HttpSpec http_flashspec[] =
{
   { HTTPSPEC_FILE,     "/",             index_shtml, NULL, 0, NULL, NULL },
   { HTTPSPEC_FILE,     "/index.shtml",  index_shtml, NULL, 0, NULL, NULL },
   { HTTPSPEC_FILE,     "/rabbit1.gif",  rabbit1_gif, NULL, 0, NULL, NULL },
   { HTTPSPEC_FILE,     "flashlog.c",    flashlog_c,  NULL, 0, NULL, NULL },

   { HTTPSPEC_VARIABLE, "count",         0, &count, INT16, "%d", NULL },

   { HTTPSPEC_FUNCTION, "/log.cgi",      0, log_cgi,      0, NULL, NULL },
   { HTTPSPEC_FUNCTION, "/printlog.cgi", 0, printlog_cgi, 0, NULL, NULL },
   { HTTPSPEC_FUNCTION, "/resetlog.cgi", 0, resetlog_cgi, 0, NULL, NULL },
};

/*
 *  void setdate(HttpState *);
 *
 *  Change the board time to the user specified time
 *  and redirect the user to the front page.
 */

int hour, minute, second, month, day, year, date_lock;

int set_date(HttpState *state)
{
   auto struct tm time;

   if (state->cancel)
   {
      return 1;
   }
   time.tm_sec = second;
   time.tm_min = minute;
   time.tm_hour = hour;
   time.tm_mon = month2tm_mon(month);
   time.tm_mday = day;
   time.tm_year = year - 1900;

   tm_wr(&time);
   SEC_TIMER = mktime(&time);

   date_lock = 0;

   cgi_redirectto(state, REDIRECTTO);
   return 0;
}

/*
 *  void update_date(void)
 *
 *  Update the date from the board clock
 */

void update_date(void)
{
   auto struct tm time;

   #GLOBAL_INIT { date_lock = 0; }

   if (date_lock)
   {
      return;
   }

   tm_rd(&time);

   month = tm_mon2month(time.tm_mon);
   day = time.tm_mday;
   year = time.tm_year + 1900;
   hour = time.tm_hour;
   minute = time.tm_min;
   second = time.tm_sec;
}

/*
 *  int lock_date(HttpState *)
 *
 *  Lock the date structure so we can update it atomically.
 */

int lock_date(HttpState *state)
{
   date_lock = 1;
   return 1;
}

/*
 *  void init_dateform(void)
 *
 *  Initialize the date form using ZServer primitives.
 */

void init_dateform(void)
{
   auto int var, form;
   static FormVar dateform[7];

   /*
    *  Set up the date form.  To set up your own form you follow a
    *  similar set of steps.  First call sspec_addform.  Then add
    *  the variables creating the variable and adding it to the form.
    *  If you want to restrict the values of the variables you can
    *  use the sspec_setfvlen, sspec_setfvrange, or define a custom
    *  validation function with sspec_fvcheck.
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
   sspec_setfvrange(form, var, MY_FIRST_YEAR_SETTING, MY_FINAL_YEAR_SETTING);

   /*
    *  lock_date sets a flag that disables the automatic updating
    *  of the hour, minute, second, month, day, and year variables.
    *  The prolog gets called after the form is parsed correctly,
    *  but before the variables are updated.  The set_date function
    *  gets called after the variables are updated.  The lock_date
    *  function is necessary because the update_date function in
    *  main could be called between the time the variables are
    *  updated and the prolog completes its processing.
    */

   var = sspec_addfunction("lock_date", lock_date, SERVER_HTTP);
   sspec_setformprolog(form, var);
   var = sspec_addfunction("set_date", set_date, SERVER_HTTP);
   sspec_setformepilog(form, var);
}

/*
 *  void main(void)
 *
 *  Initialize the flash, set up the date form and start the web server.
 */

void main(void)
{
   brdInit();
   sock_init_or_exit(1);   // call debug function (provides handy information)
   http_init();
   init_dateform();
   init_log();

   tcp_reserveport(80); // mark port 80 as a server port.

   for (;;)
   {
      update_date();
      http_handler();
   }
}