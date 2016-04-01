/******************************************************************************
 *
 *    sample_ntplib.c
 *
 *  Test of SNTP client implementation
 *
 *  Author: Mircea Neacsu
 *  Date:   Jan 18, 2015
 *
*******************************************************************************/

//DHCP with static fallback configuration
#define TCPCONFIG 7

/* Some people like to define these in the compiler defines. If you haven't
done that, now is the time to set default values if there is no DHCP server
available */
#ifndef _PRIMARY_STATIC_IP
#define _PRIMARY_STATIC_IP  "192.168.3.101"
#define _PRIMARY_NETMASK    "255.255.255.0"
#define MY_GATEWAY          "192.168.3.1"
#define MY_NAMESERVER       "8.8.8.8"
#endif

//Debug stuff
#define NTP_VERBOSE

#define MAX_UDP_SOCKET_BUFFERS  2

#memmap xmem
#use "dcrtcp.lib"


#ifndef RTC_IS_UTC
/*
  Offset between local time and UTC (in hours) and name of time zone.
  For more explanations see RTCLOCK.LIB
*/
float timezone_offset;
char timezone_str[20];

#define TIMEZONE timezone_offset
#define TZNAME timezone_str

#endif

//Number of servers configured
#define NTP_SERVERS_COUNT          4

/*
  Hostnames (or dotted quad IP addresses) of each time server to query.
  You can "weight" some servers as being more reliable by mentioning the
  same server in more than one entry.
  NOTE: the following addresses were obtained from hosts that were
  denoted as "open access" (see list at http://ntp.org) to NTP.
*/
char* ntp_servers[NTP_SERVERS_COUNT] =
{
   "pool.ntp.org"
  ,"ntp1.cmc.ec.gc.ca"
  ,"time.chu.nrc.ca"
  ,"timelord.uregina.ca"
};
#use "ntp.lib"


char* format_timestamp (uint32 stamp)
{
  static char b[40];
  struct tm daytime;

  mktm(&daytime, stamp);
  strftime(b, sizeof(b), "%a %b %d %Y %T", &daytime);
  return b;
}


int main()
{
  int rc, done;
  long dst_start, dst_end, eur_start, eur_end;
  struct tm tmp;
  long t, adj;

  sock_init();

  //wait for DHCP to bring the interface up
  while (ifpending (IF_ETH0) != IF_UP)
    tcp_tick (NULL);

//

/*=============================================================================
           TEST 1
  Use the nth_dow function (which in turn uses dow function) in NTP.LIB to
  determine the DST start and end times. Assumes RTC has at least the good year
  to get the correct start/end dates for DST. Worst case, just run the program
  twice.
=============================================================================*/
  printf ("Test 1\n======\n");

  mktm(&tmp, read_rtc());

  //Current US and Canada rules: DST starts on second Sunday in March at 02:00
  tmp.tm_mon = 3;
  tmp.tm_mday = nth_dow (tmp.tm_year+1900, 3, 0, 2);
  tmp.tm_hour = 2;
  tmp.tm_min = 0;
  tmp.tm_sec = 0;
  dst_start = mktime(&tmp);
  printf ("In North America DST start is %s\n", format_timestamp(dst_start));

  //DST ends on first Sunday of November at 02:00
  tmp.tm_mon = 11;
  tmp.tm_mday = nth_dow (tmp.tm_year+1900, 11, 0, 1);
  dst_end = mktime(&tmp);
  printf ("DST end is %s\n", format_timestamp(dst_end));

  printf ("\n\n");

/*=============================================================================
           TEST 2
  Test for 'last week' specification in nth_dow function

  In Europe DST (summer time) starts on last Sunday in March at 01:00 and
  ends on last Sunday in October at 01:00.
=============================================================================*/
  printf ("Test 2\n======\n");

  tmp.tm_mon = 3;
  tmp.tm_hour = 1;
  tmp.tm_mday = nth_dow (tmp.tm_year+1900, 3, 0, 5);
  eur_start = mktime(&tmp);
  printf ("In Europe summer time starts %s\n", format_timestamp(eur_start));


  tmp.tm_mon = 10;
  tmp.tm_mday = nth_dow (tmp.tm_year+1900, 10, 0, 5);
  eur_end = mktime(&tmp);
  printf ("...and ends %s\n", format_timestamp(eur_end));

  printf ("\n\n");

/*=============================================================================
           TEST 3
  Sync RTC with NTP servers using the blocking version of library functions.
=============================================================================*/
  printf ("Test 3\n======\n");

  //set timezone for Eastern time
  if (SEC_TIMER > dst_start && SEC_TIMER < dst_end)
  {
    TIMEZONE = -4;
    strcpy (TZNAME, "EDT");
  }
  else
  {
    TIMEZONE = -5;
    strcpy (TZNAME, "EST");
  }

  printf ("Adjusting RTC clcok...\n");
  t = MS_TIMER;
  rc = ntp_set_time (&adj);
  t = MS_TIMER - t;
  if (rc)
    printf ("ntp_set_time returned %d\n", rc);
  else
    printf ("RTC clock adjusted by %ld seconds.\n", adj);
  printf ("Finished in %ldms\n", t);

  printf ("\n\n");

/*=============================================================================
           TEST 4
  Sync RTC with NTP servers using the cofunction (unblocking) version.
=============================================================================*/
  printf ("Test 4\n======\n");

  done = 0;
  t = 0;
  while (!done)
  {
    costate {
      waitfor (IntervalMs(10));
      t++;
    }
    costate {
      wfd rc = cof_ntp_set_time (&adj);
      done = 1;
    }
  }
  if (rc)
    printf ("ntp_set_time returned %d\n", rc);
  else
    printf ("RTC clock adjusted by %ld seconds.\n", adj);
  printf ("The other costate ticked %d intervals of 10ms\n", t);

  printf ("\n\n");
  return 0;
}