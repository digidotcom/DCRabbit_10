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
/*****************************************************

   virt_wd.c

   This program demonstrates the use of a virtual watchdog

******************************************************/
#class auto


void main()
{
   static int wd;                // ID for a virtual watchdog
	const static int runTime = 30;
	static unsigned long tm0;
	
	tm0 = SEC_TIMER;
	wd  = VdGetFreeWd(33); // wd activated, 9 virtual watchdogs now available
	                       // wd must be hit at least every (33 - 1)/16 = 2 seconds 
	                      
	printf("This program will terminate normally with exit code 0 \n");   
	printf("in %d seconds if no virtual watchdog times out.", runTime);   
	
	while(SEC_TIMER - tm0 < runTime) { // let it run for a little while
		// comment out VdHitWd(wd) to observe the fatal error message after 2 seconds of run time
		VdHitWd(wd);        // decrementing counter corresponding to wd reset to 33
	}
	VdReleaseWd(wd);       // now all 10 virtual watchdogs are available
}
