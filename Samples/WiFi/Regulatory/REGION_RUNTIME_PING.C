/**********************************************************

   region_runtime_ping.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

	This program is used with 802.11 wifi devices operating on
   their corresponding prototyping or interface boards.

	Description
	===========
   This program demonstrates how the region setting can be set at runtime
   for configuring the device to meet regional regulations. It also shows
   how you can save and retrieve the region setting from non-volatile memory.

   General Info
	============
   The country you select will automatically set the power and channel
   requirements for operating your wifi device. Recommend checking the
   regulations where your wireless devices will be deployed for any
   other requirements.

	Instructions
	============
   1. Inspect code for ifconfig function usage to see how it
   sets regional settings.
   2. Set network parameters.
   3. Compile and run program.
   4. Step through the menu selections to view user options.
   5. With WIFI_REGION_VERBOSE defined, view STDIO to see channel
   and power settings as the wifi driver uses them.

**********************************************************/
#class auto

#use "idblock_api.lib"

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

// Macro to see channel and power level settings being done by the wifi driver
//#define WIFI_REGION_VERBOSE

/** Remote interface to send PING to (passed to resolve()): **/
/*  Undefine to retrieve default gateway and ping that. */
#define PING_WHO			"10.10.6.1"


/** How often to send PING's (in milliseconds): **/
#define PING_DELAY		500

#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"

/**********************************************************
	Routines to change the LEDs.
	The pingleds_setup routine turns off LED's.  The
	pingoutled routine toggles DS2. The pinginled routine
	toggles DS3.
***********************************************************/
#if RCM4400W_SERIES
	#use "RCM44xxW.lib"
   #define DS2 2
   #define DS3 3

#elif RCM5400W_SERIES
	#use "RCM54xxW.lib"
   #define DS2 2
   #define DS3 3

#elif RCM5600W_SERIES
	#use "RCM56xxW.lib"
   #define DS1 0

#endif

#define LEDON	1
#define LEDOFF	0

int ledstatus;

pingoutled(int onoff)
{
   #if RCM5600W_SERIES
	BitWrPortI(PDDR, &PDDRShadow, onoff, DS1);
   #else
   BitWrPortI(PBDR, &PBDRShadow, onoff, DS2);
   #endif
}

pinginled(int onoff)
{
   #if RCM5600W_SERIES
   BitWrPortI(PDDR, &PDDRShadow, onoff, DS1);
	#else
	BitWrPortI(PBDR, &PBDRShadow, onoff, DS3);
   #endif
}


//------------------------------------------------------------------------
// Write the region setting to userblock area in flash for use
// on subsequent power cycles.
//------------------------------------------------------------------------
void regionInfoEEwr( int region )
{
	auto char buffer[5];
	auto int checksum;
	auto int i;
	auto int *ptr;

	buffer[0] = 0x55;
	buffer[1] = 0xAA;
	buffer[2] = region;
	checksum = 0;
	for(i=0; i < 3; i++)
	{
		checksum += (int)buffer[i];
	}
	ptr = (int*)(&buffer[3]);
	*ptr = checksum;
   i = writeUserBlock(0, buffer, sizeof(buffer));
	if (i) {
   	printf ("write to userblock error: %d\n", i);
   }
}


//------------------------------------------------------------------------
// Check if there's a valid region setting in Flash userblock
//------------------------------------------------------------------------
int regionInfoEErd( void )
{
	auto char buffer[5];
	auto int checksum;
	auto int i;
	auto int *ptr;

	readUserBlock(buffer, 0, sizeof(buffer));

	checksum = 0;
	for(i=0; i < 3; i++)
	{
		checksum += (int)buffer[i];
	}
	ptr = (int*) &buffer[3];
	if((checksum  == *ptr) && (buffer[0] == 0x55) && (buffer[1] == 0xAA))
   {
		return (int)buffer[2];
	}
	else
   {
		printf ("userblock read: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
		return(-1);
	}
}

void display_info(wifi_region *region_info)
{
   printf("%s, Wifi Ch %2d-%2d, Max pwr < %2d dBm, Pwr_index=%2d, ChMask=%04x \n\n",
      region_info->country,
      region_info->first_channel,
      region_info->last_channel,
      region_info->max_pwr_dBm,
      region_info->max_pwr_index,
      region_info->channel_mask);
}


int get_stored_region(wifi_region *region_info)
{
	auto int country;
   auto char data[64];

	if((country = regionInfoEErd()) < 0 )
   {
   	ifconfig (IF_WIFI0, IFG_WIFI_REGION_INFO, region_info, IFS_END);
      display_info(region_info);
      return -1;
   }
   return country;
}


int wifi_config_region(wifi_region *region_info)
{
	auto int country;
   auto char index[10];
   auto char tmpbuf[64];
   auto int i;
   auto int region;

   //----------------------------------------------------------------------
   //		Display Menu for region selection
   //----------------------------------------------------------------------
   printf("\nSelect region:\n");
   for(i=0; i<_END_OF_REGIONS; i++)
   {
      // Set region to populate region stucture with info
      // Note: Interface is down, its OK to iterate through settings to
      //       create menu options
      ifconfig (IF_WIFI0, IFS_WIFI_REGION, i, IFS_END);

      // Get info for menu display
      ifconfig (IF_WIFI0, IFG_WIFI_REGION_INFO, region_info, IFS_END);
      printf("%d %s.\n", i, region_info->country);
   }
   printf("\nSelect Region Index > ");
  	do
  	{
  		country = atoi(gets(tmpbuf));
  	} while (!((country >= 0) && (country < _END_OF_REGIONS)));

   // Write region selection to userblock for subsequent reset/power cycle
   regionInfoEEwr(country);

   // Set Region from previously saved value read from the userblock, this
   // will set the runtime limits to be used by the wifi driver.
   ifconfig (IF_WIFI0, IFS_WIFI_REGION, country, IFS_END);

   printf("Region selection successfully updated\n\n");
   return country;
}


main()
{
	longword seq,ping_who,tmp_seq,time_out;
	char	buffer[100];
   auto wifi_region region_info;
   auto char index[10];
   auto int option;
   auto char tmpbuf[64];
   auto int updateRegion;
   auto int country;
   auto int configured;


	brdInit();				//initialize board for this demo

	seq=0;

	sock_init();			// Initial wifi interface

   // Make sure wifi IF is down to do ifconfig's functions
	printf("\nBringing interface down (disassociate)...\n");
   ifdown(IF_WIFI0);
   while (ifpending(IF_WIFI0) != IF_DOWN) {
     	printf(".");
     	tcp_tick(NULL);
   }
	printf("...Done.\n");

   configured = FALSE;
   updateRegion = FALSE;
   do
   {
   	country = get_stored_region(&region_info);
   	// Check if the region has been previously set.
 		if(country < 0 || updateRegion)
   	{
      	// Populate structure with region info, then display
   		ifconfig (IF_WIFI0, IFG_WIFI_REGION_INFO, &region_info, IFS_END);
   		printf("\nCurrent region setting:\n");
   		display_info(&region_info);

         // Select Region and write value to userblock
    		country = wifi_config_region(&region_info);
         updateRegion = FALSE;

   	}
   	else
   	{
      	// Set Region from previously saved value read from the userblock, this
      	// will set the runtime limits to be used by the wifi driver.

   		ifconfig (IF_WIFI0, IFS_WIFI_REGION, country,
         	IFG_WIFI_REGION_INFO, &region_info, IFS_END);

   		printf("\nRuntime region setting now being used by wifi driver:\n");
         display_info(&region_info);

      	// Region has already been set at runtime, check if it needs to
         // be changed due to country to country roaming.
    		printf("\nRegion already set, select option to continue");
         printf("\n1. Continue.");
         printf("\n2. Select new region.");
         printf("\nSelect > ");
			do
			{
				option = atoi(gets(tmpbuf));
	  		} while (!((option >= 1) && (option <= 2)));
         if(option == 2)
         	updateRegion = TRUE;
         else
         	configured = TRUE;
      }
	}while(!configured);

   // If you are not going to use the defaulted channel and/or power level,
   // then you can use the following functions to set channel and/or the
   // power level. This needs to be done after setting the region/country
   // to meet wifi driver requirements.

   //ifconfig (IF_WIFI0, IFS_WIFI_CHANNEL, 0, IFS_END); // Scan all channels
   //ifconfig (IF_WIFI0, IFS_WIFI_TX_POWER, 8, IFS_END); // Set to Pwr level 8


   // Startup the  wireless interface here...
	printf("Bringing interface back up (associate)...\n");
   ifup(IF_WIFI0);
   while (ifpending(IF_WIFI0) == IF_COMING_UP) {
      tcp_tick(NULL);
   }
	printf("...Done.\n");
	if (ifpending(IF_WIFI0) != IF_UP) {
		printf("Unfortunately, it failed to associate :-(\n");
		exit(1);
	}
   // End of regional setting section, from this point on do standard tcp/ip
   // protocol.


   /*
   // Here is where we gather the statistics...
	// Note that if you get a compile error here, it is because you are not running
	// this sample on a Wifi-equipped board.

	/* Print who we are... */
	printf( "My IP address is %s\n\n", inet_ntoa(buffer, gethostid()) );

	/*
	 *		Get the binary ip address for the target of our
	 *		pinging.
	 */

#ifdef PING_WHO
	/* Ping a specific IP addr: */
	ping_who=resolve(PING_WHO);
	if(ping_who==0) {
		printf("ERROR: unable to resolve %s\n",PING_WHO);
		exit(2);
	}
#else
	/* Examine our configuration, and ping the default router: */
	tmp_seq = ifconfig( IF_ANY, IFG_ROUTER_DEFAULT, & ping_who, IFS_END );
	if( tmp_seq != 0 ) {
		printf( "ERROR: ifconfig() failed --> %d\n", (int) tmp_seq );
		exit(2);
	}
	if(ping_who==0) {
		printf("ERROR: unable to resolve IFG_ROUTER_DEFAULT\n");
		exit(2);
	}
#endif

	for(;;) {
		/*
		 *		It is important to call tcp_tick here because
		 *		ping packets will not get processed otherwise.
		 *
		 */

		tcp_tick(NULL);

		/*
		 *		Send one ping every PING_DELAY ms.
		 */

		costate {
			waitfor(DelayMs(PING_DELAY));
			_ping(ping_who,seq++);
			pingoutled(LEDON);					// flash transmit LED
			waitfor(DelayMs(50));
			pingoutled(LEDOFF);
		}

		/*
		 *		Has a ping come in?  time_out!=0xfffffff->yes.
		 */

		costate {
			time_out=_chk_ping(ping_who,&tmp_seq);
			if(time_out!=0xffffffff) {

#ifdef VERBOSE
				printf("received ping:  %ld\n", tmp_seq);
#endif

				pinginled(LEDON);					// flash receive LED
				waitfor(DelayMs(50));
				pinginled(LEDOFF);
			}
		}
	}
}