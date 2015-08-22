/****************************************************************************

   WiFiScanAssociate.c

	Digi International, Copyright (C) 2007-2008.  All rights reserved.

   This code demonstrates how to scan WiFi channels for SSID's using the
   ifconfig IFS_WIFI_SCAN option.  The scan takes a while to complete, so it
   calls a callback function when it is done.

   To run this sample, first configure tcp_config.lib and your TCPCONFIG
   mode.  This process is described in the function help (Ctrl-H) for
   TCPCONFIG.  Note that for this sample it is not particularly important
   to select an SSID, since the program will allow you to do that at
   runtime.

   Compile and run the code.  Follow the menu options:

      s - scan for access points,
      a - scan and associate
      m - dump WIFI MAC state information

   Important features to note:

   -	The ifconfig IFS_WIFI_SCAN option does not return data directly, since
   	the scan takes a fair amount of time.  Instead, callback functions are
      used.  The callback function is passed to ifconfig as the only
      parameter to IFS_WIFI_SCAN.

   -  The data passed to the callback function is ephemeral, since another
      scan may occur immediately after the first has completed.  Thus, the
      data needs to be used (or copied) during the callback function.

   -  While waiting for user input, it is important to keep the network
      alive by regularly calling tcp_tick(NULL).  This drives the scan, as
      well as any other network processes running.

	This sample demonstrates a WiFi specific feature.  Additional networking
	samples for WiFi can be found in the Samples\tcpip directory.

****************************************************************************/

/*
 * NETWORK CONFIGURATION
 * Please see the function help (Ctrl-H) on TCPCONFIG for instructions on
 * compile-time network configuration.
 */
#define TCPCONFIG 1

#use "dcrtcp.lib"

#memmap xmem

// Keeps track of whether or not a scan has just completed
int scan_complete;

/****************************************************************************
	print_macaddress

	Routine to print out mac_addr types.

****************************************************************************/
void print_macaddress(far unsigned char *addr)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2],
	       addr[3], addr[4], addr[5]);
}

/****************************************************************************
	wifi_rates_str

	Routine to convert wifi_status.tx_rate and wifi_status.rx_rate bitfields
   to a string listing speeds in Mbps.

****************************************************************************/
const int rates[] = { 10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540 };
char *wifi_rates_str (char *buffer, word rate)
{
	int i;
	char *p;

   p = buffer;
   for (i = 0; i < (sizeof(rates) / sizeof(rates[0])); i++) {
		if (rate & (1 << i)) {
			p += sprintf (p, "%u.%u ", rates[i] / 10, rates[i] % 10);
      }
   }
   if (p == buffer) strcpy (p, "none");
   else sprintf (p, "Mbps");

   return buffer;
}

/****************************************************************************
	wifi_auths_str

	Routine to convert wifi_status.authen bitfield to a string listing
   authentication types.

****************************************************************************/
char * const auths[] = { "open", "wep shared", "wep 802.1x", "wpa psk", "wpa 802.1x", "leap" };
char *wifi_auths_str (char *buffer, longword auth)
{
	int i;
   char *p;

   p = buffer;
   for (i = 0; i < (sizeof(auths) / sizeof(auths[0])); i++) {
		if (auth & (1 << i)) {
      	if (p != buffer) p += sprintf (p, ", ");
			p += sprintf (p, "%s", auths[i]);
      }
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	wifi_encrs_str

	Routine to convert wifi_status.encrypt bitfield to a string listing
   encryption types.

****************************************************************************/
char * const encrs[] = { "open", "wep", "tkip", "ccmp" };
char *wifi_encrs_str (char *buffer, longword auth)
{
	int i;
   char *p;

   p = buffer;
   for (i = 0; i < (sizeof(encrs) / sizeof(encrs[0])); i++) {
		if (auth & (1 << i)) {
      	if (p != buffer) p += sprintf (p, ", ");
			p += sprintf (p, "%s", encrs[i]);
      }
   }
   if (p == buffer) strcpy (p, "none");

   return buffer;
}

/****************************************************************************
	print_status

	Routine to print out status (wln_status type).

****************************************************************************/
void print_status(wifi_status *status)
{
	char buffer[80];	// 60 should actually be enough

   printf("\nMAC status:\n");
	printf("   state = %d (%s)\n", status->state,
	      status->state == WLN_ST_STOPPED    ? "Driver not running" :
	      status->state == WLN_ST_SCANNING   ? "Scanning for BSS" :
	      status->state == WLN_ST_ASSOC_ESS  ? "Associated with ESS" :
	      status->state == WLN_ST_AUTH_ESS   ? "Authenticated with ESS" :
	      status->state == WLN_ST_JOIN_IBSS  ? "Joined ad-hoc IBSS" :
	      status->state == WLN_ST_START_IBSS ? "Started ad-hoc IBSS" :
	      												 "Unknown/illegal status");
	printf("   ssid = %s\n", status->ssid);
	printf("   ssid_len = %d\n", status->ssid_len);
	printf("   channel = %d\n", status->channel);
	printf("   bss_addr = ");
	print_macaddress(status->bss_addr);
	printf("\n");
	printf("   bss_caps = %04x\n", status->bss_caps);
	printf("   authen = %08lx (%s)\n", status->authen,
   	wifi_auths_str (buffer, status->authen));
	printf("   encrypt = %08lx (%s)\n", status->encrypt,
   	wifi_encrs_str (buffer, status->encrypt));
	printf("   tx_rate = %s\n", wifi_rates_str (buffer, status->tx_rate));
	printf("   rx_rate = %s\n", wifi_rates_str (buffer, status->rx_rate));
	printf("   rx_signal = %d\n", status->rx_signal);
	printf("   tx_power = %d\n", status->tx_power);
}

/****************************************************************************
	rxsignal_cmp

	qsort comparison, based on rx signal strength.

   Inputs:
      a, b  -- far pointers to _wifi_wln_scan_bss, a struct populated by
               the WIFI SCAN, including rx_signal (relative receive signal
               strength).

	Return value: > 0 if a > b
					  < 0 if a < b
					  0   if a == b

****************************************************************************/
int rxsignal_cmp(far _wifi_wln_scan_bss *a, far _wifi_wln_scan_bss *b) {
	return b->rx_signal - a->rx_signal;
}

/****************************************************************************
	scan_callback

   Prints out the sorted results of a BSS scan.
   Called when WIFI SCAN is complete.

   The argument is a pointer to the wifi_scan_data structure generated by
   the scan.

   We use _f_qsort to sort the data since the data is `far.'  _f_qsort
   requires a comparison function, and we use the rxsignal_cmp() function
   above.

   Inputs:
      data  -- far pointer to wifi_scan_data structure, which contains a
               count of the number of responses, and an array of
               _wifi_wln_scan_bss structures, with the first `count'
               containing valid data for the responses.

****************************************************************************/
root void scan_callback(far wifi_scan_data* data)
{
	uint8 i, j;
	far _wifi_wln_scan_bss *bss;
   char ssid_str[33];
   char buffer[80];

	scan_complete = 1;

	bss = data->bss;
	// _wifi_macStatus.ssid is the BSS we are currently associated with _or_
	// currently trying to associate with.
	printf("Current BSS is %*s.\n\n", _wifi_macStatus.ssid_len,
				_wifi_macStatus.ssid);
	// Sort results by signal strength.  Need to use _f_qsort, since bss is
	// far data.
	_f_qsort(bss, data->count, sizeof(bss[0]), rxsignal_cmp);
	// Print out results
   for (i = 0; i < data->count; i++) {
   	wifi_ssid_to_str (ssid_str, bss[i].ssid, bss[i].ssid_len);
      printf("%X: chan %2d; rx_signal %d; MAC ", i, bss[i].channel,
      	bss[i].rx_signal);
      print_macaddress(bss[i].bss_addr);
      printf ("\n   SSID [%s]; ", ssid_str);
      printf ("rates = %s\n", wifi_rates_str (buffer, bss[i].rates_basic));
      printf("   caps = %04x", bss[i].bss_caps);
      if (bss[i].bss_caps & WLN_CAP_PRIVACY) {
      	switch (bss[i].wpa_info[0]) {
      	case _WIFI_ELEM_RSN:
      		printf("; WPA2/RSN %s\n",
      			bss[i].wpa_info[19] == 2 ? "PSK" :
      			bss[i].wpa_info[19] == 1 ? "802.1X" :
      			"VENDOR");
      		break;
      	case _WIFI_ELEM_VENDOR:
      		printf("; WPA %s\n",
      			bss[i].wpa_info[23] == 2 ? "PSK" :
      			bss[i].wpa_info[23] == 1 ? "802.1X" :
      			"VENDOR");
      		break;
      	default:
      		printf("; WEP\n");
      		break;
      	}
      }
      else {
         printf("\n");
      }
   }
}

/****************************************************************************
	scan_assoc_callback

   Much like scan_callback above, this function is called as a result of a
   Wi-Fi scan.  The main difference is that this function gives the user the
   option of associating with one of the BSS's.  It uses scan_callback above
   to sort and print the scan results.

   Inputs:
      data  -- far pointer to wifi_scan_data structure, which contains a
               count of the number of responses, and an array of
               _wifi_wln_scan_bss structures, with the first `count'
               containing valid data for the responses.

****************************************************************************/
root void scan_assoc_callback(far wifi_scan_data* data)
{
	char c, ssid[WLN_SSID_SIZE+1];
	int ssid_len;
	far _wifi_wln_scan_bss *bss;

	// Sort and print the scan results
	scan_callback(data);
	bss = data->bss;
   printf("\nSelect a new BSS or quit ([0-%x, q to quit)]\n", data->count-1);
   while (1) {
		tcp_tick(NULL); // While we're waiting, continue to tick.
		if (kbhit()) {
			c = getchar();
			// Echo the character
			printf("%c\n", c);
			// Convert character to numeric value.
			if ('0' <= c && c <= '9') { c = c - '0'; }
			else if (isxdigit(c)) { c = (tolower(c) - 'a' + 10); }
			else if (tolower(c) == 'q') {
				printf("Quitting scan selection\n");
				break;
			}
			if (c >= data->count) {
				printf("Unlisted option, quitting...\n");
				break;
			}
			// c is now the index of the BSS the user opted to associate with
			bss = &(data->bss[c]);
			ssid_len = bss->ssid_len;
			// Need near copy of SSID to call ifconfig.  ssid will be promoted to
			// far for this call, but the results will be in ssid as a near
			// variable
			_f_memcpy(ssid, bss->ssid, ssid_len);
#ifdef IFC_WIFI_WPA_PSK_PASSPHRASE
		 	printf("Resetting the passphrase--this will take some time.\n");
#endif
			// Set the SSID.  Also, if a passphrase has been defined as a macro,
			// then reconfigure the passphrase.  This is necessary because the
			// passphrase and SSID together are used to generate the key.  If the
			// SSID changes, then the generated key must change.  Note that
			// regenerating the key will take about another 20 seconds on an
			// RCM54xxW.
         if (ifconfig (IF_WIFI0, IFS_WIFI_SSID, ssid_len, ssid,
#ifdef IFC_WIFI_WPA_PSK_PASSPHRASE
			              IFS_WIFI_WPA_PSK_PASSPHRASE, IFC_WIFI_WPA_PSK_PASSPHRASE,
#endif
             IFS_END)) {
				printf (" error setting SSID\n");
         }
			wifi_ssid_to_str (ssid, ssid, ssid_len);
			printf("Selected BSS is [%s].  Wait a bit, then check MAC status\n",
						ssid);
			break;
		}
	}
}

/****************************************************************************
	main

	Print out a menu, wait for keypresses, while calling tcp_tick.

****************************************************************************/
void main(void)
{
	int val0, val1,i, level;
   mac_addr mac;
   char c;
   word waitms, pingit;
   longword pingid;
   wifi_status status;
   int len;
   int printmenu;
   unsigned long int end;

	// Initialize the scan_complete variable
	scan_complete = 0;

	sock_init();

	waitms = _SET_SHORT_TIMEOUT(300);
	pingit = 0;
   printmenu = 1;
   for (;;) {
   	if (printmenu) {
      	printmenu = 0;
	      printf("\nMenu:\n");
	      printf("   Press s to scan available access points\n");
	      printf("   Press a to scan access points and associate\n");
	      printf("   Press m to print WIFI MAC status\n");
	      printf("\n");
      }
		tcp_tick(NULL);
		if (kbhit()) {
      	switch (getchar()) {
         	case 'm':
            case 'M':
	            ifconfig (IF_WIFI0, IFG_WIFI_STATUS, &status, IFS_END);
	            print_status(&status);
	            printmenu = 1;
					break;

            case 's':
            case 'S':
	            // Bring the interface down before starting a scan
	            ifdown(IF_WIFI0);
	            while (ifpending(IF_WIFI0) != IF_DOWN)
	               tcp_tick(NULL);
	            // Set the callback before requesting scan
	            ifconfig(IF_WIFI0, IFS_WIFI_SCAN, scan_callback, IFS_END);
	            printf("Starting scan...\n");
					break;

            case 'a':
            case 'A':
	            // Bring the interface down before starting a scan
	            ifdown(IF_WIFI0);
	            while (ifpending(IF_WIFI0) != IF_DOWN)
	               tcp_tick(NULL);
	            ifconfig(IF_WIFI0, IFS_WIFI_SCAN, scan_assoc_callback, IFS_END);
	            printf("Starting scan...\n");
					break;
			}
		}
		// Check to see if a scan has completed.  If so, then bring the
		// interface back up.
		if (scan_complete) {
			ifup(IF_WIFI0);
			scan_complete = 0;
         printmenu = 1;
		}
   }
}

