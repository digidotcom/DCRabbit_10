/**********************************************************

   region_compiletime.c
	Digi International, Copyright (C) 2007-2008.  All rights reserved.

	This program is used with 802.11 wifi devices operating on
   their corresponding prototyping or interface boards.

	Description
	===========
   This program demonstrates how you can setup your wifi device at compile
   time to run in a given region to meet power and channel requirements.

   General Info
	============
   The country you select will automatically set the power and channel
   requirements for operating your wifi device. Recommend checking the
   regulations where your wireless devices will be deployed for any
   other requirements.

	Instructions
	============
   1. Define IFC_WIFI_REGION for region the device will be deployed.
   2. Compile and run program.
   3. View STDIO to see region displayed for your selection.

**********************************************************/
#class auto


 // Uncomment define for region to be selected.
 // Americas, including the US (channels 1-11)
#define IFC_WIFI_REGION  IFPARAM_WIFI_REGION_AMERICAS		// This is default used by the wifi driver

 // Mexico outdoors (channels 1-11)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_MEXICO_INDOORS

 // Mexico outdoors (channels 9-11)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_MEXICO_OUTDOORS

 // Canada (channels 1-11)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_CANADA

 // Europe, Middle East, Africa (channels 1-13), except France
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_EMEA

 // France (channels 10-13)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_FRANCE

 // Israel (channels 3-11)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_ISRAEL

 // China (channels 1-11)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_CHINA

 // Japan (RCM44xxW channels 1-13, RCM54xxW channels 1-13)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_JAPAN

 // Australia (channels 1-11)
 //#define IFC_WIFI_REGION IFPARAM_WIFI_REGION_AUSTRALIA

/***********************************
 * Configuration                   *
 * -------------                   *
 * All fields in this section must *
 * be altered to match your local  *
 * network settings.               *
 ***********************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on  TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

#define VERBOSE
#memmap xmem
#use "dcrtcp.lib"

void display_info(_wifi_country *region_info)
{
   printf("%s, Wifi Ch %2d-%2d, Max pwr < %2d dBm, Pwr_index=%2d, ChMask=%04x \n\n",
      region_info->country,
      region_info->first_channel,
      region_info->last_channel,
      region_info->max_pwr_dBm,
      region_info->max_pwr_index,
      region_info->channel_mask);
}

main()
{
   auto wifi_region region_info;

   //----------------------------------------------------------------------
   //		Initialize TCP stack
   //----------------------------------------------------------------------

   sock_init();

   // Populate structure with region info, then display
   // Note: This IFG_WIFI_REGION_INFO not needed for actual application program,
   // just being used for displaying country information.
   ifconfig (IF_WIFI0, IFG_WIFI_REGION_INFO, &region_info, IFS_END);
   printf("\nCurrent region setting:\n");
   display_info(&region_info);


   // Startup the  wireless interface here...
}









