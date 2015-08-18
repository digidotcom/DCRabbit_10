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
/**********************************************************

   passphrase.c

   This program is used with Wi-Fi enabled Rabbit devices.

   Description
   ===========
   This program demonstrates how to perform the CPU-intensive
   process of converting an ASCII passphrase into a WPA PSK
   hex key.

   This is motivated by the need to convert passphrases to hex
   keys in order for WPA PSK to work.  For security reasons,
   however, the mapping function is deliberately designed to be
   very CPU intensive, in order to make a dictionary-based attack
   more difficult.  For example, the conversion can take of the
   order of 15 seconds on the RCM5400W.

   Since this may be an unacceptable amount of time to "block"
   the application program, a method of splitting up the
   computation is provided.

   The complete process takes 4096 iterations of a secure hashing
   function.  You can perform the process a few iterations at a
   time.  For example, performing one iteration at a time will
   block the application for only about 1/100sec.  Performing
   10 at a time will block for 1/10 sec, and so on.

   There is only a few percent overhead in breaking up the entire
   process into single iterations.

   The procedure boils down to calling two functions:

	wpa_passphrase_to_psk_init(
		&pps,				// <- state structure, managed by library
		"passphrase",	// <- your passphrase
		"ssid",			// <- target SSID
		ssid_length,	// <- length of the SSID string (since allowed binary)
		key);				// <- returned hex key - must be 32 byte char array.

	followed by this idiom:

	while (wpa_passphrase_to_psk_run(&pps, num_iterations))
	{
		// do something useful for this app while waiting
	}


	The pps parameter is a structure of type wpa_passphrase_to_psk_state
	which is fully managed by the two functions - there is no need for
	the application to be concerned with its contents.  The only requirement
	is that the pps structure contents are not touched by the application
	until the process is finished.

	num_iterations is arbitrary - a lower number gives finer "granularity".
	Each call of wpa_passphrase_to_psk_run() will run for approximately
	num_iterations times 1/100 sec.

	Note: if you wish to perform the complete operation in one uninterrupted
	hit, then use the

		void pbkdf2_sha1(char *passphrase, char *ssid, size_t ssid_len,
			int iterations, char *key, size_t keylen);

	function, passing 4096 for the number of iterations, and 32 for the keylen
	parameter.



   Instructions
   ============
   1. Compile and run this program on a Wi-Fi enabled device.
   There is no network activity; it just prints some info on
   the stdio window.

**********************************************************/
#class auto

// Just bring in the passphrase handling routines
#use "wifi_sha1.lib"

char key[32];

wpa_passphrase_to_psk_state pps;

// This function will print the key in a format that can be copy-and-pasted
// into a define for IFC_WIFI_WPA_PSK_HEXSTR.
void print_key(char *key, int numbytes)
{
	int i;

   printf("\"");
	for (i = 0; i < numbytes; i++) {
		printf("%02X", (unsigned int)key[i]);
	}
   printf("\"\n");
}

int main()
{
	long t0, t1;

	printf("I'm going to try 1 iteration at a time, then 231 at a time,\n");
	printf("then the whole thing.  Each is going to take about 15-30 seconds\n");
	printf("depending on CPU clock speed and memory wait states...\n");

	t0 = MS_TIMER;
	wpa_passphrase_to_psk_init(&pps, "now is the time", "parvati", 7, key);
	// The 2nd parameter determines how many iterations to perform in each
	// increment of the algorithm.
	while (wpa_passphrase_to_psk_run(&pps, 1));
	t1 = MS_TIMER;
	printf("Done 1-by-1, %lums\n", t1-t0);
	print_key(key, 32);


	t0 = MS_TIMER;
	wpa_passphrase_to_psk_init(&pps, "now is the time", "parvati", 7, key);
	while (wpa_passphrase_to_psk_run(&pps, 231));
	t1 = MS_TIMER;
	printf("Done 231 at a time, %lums\n", t1-t0);
	print_key(key, 32);

	t0 = MS_TIMER;
	pbkdf2_sha1("now is the time", "parvati", 7, 4096, key, 32);
	t1 = MS_TIMER;
	printf("Done monolithic, %lums\n", t1-t0);
	print_key(key, 32);


	return 0;
}