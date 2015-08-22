/*****************************************************

   virt_wd.c
   Z-World, 1999

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
