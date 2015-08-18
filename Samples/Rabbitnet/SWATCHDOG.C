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
/*******************************************************************
   swatchdog.c

	This sample program is for any RabbitNet board.

	Description
	===========
	This program demonstrates setting and hitting the software
   watchdog on a RabbitNet device using costatements.

   This program will first look for a device directly connected to
   each controller port using rn_device().  The last device found
	will be used.

  	The software watchdog will be set for 2.5 seconds. The watchdog
   will be hit at every increasing time out until the time out is
   past 2.5 seconds.  A software reset will occur and the software
   watchdog will be disabled.

	Connections
	===========
   Connect RabbitNet and power supply cables to the controller
   and RN boards as described in the RabbitNet Manual.

	Instructions
	============
	1. Compile and run this program.
   2. Watch busy counter and delay time increase until the display
   	for software reset appears at 2500 ms.

*******************************************************************/
#class auto

#define SWTIMEOUT 2.5	//software watchdog timeout

// screen foreground colors
#define	BLACK		"\x1b[30m"
#define	RED		"\x1b[31m"


main()
{
	auto int device0, tmpdev, portnum, i, count, tdelay, statusbyte;
	auto char recbyte, timeleft;
   auto rn_devstruct *devaddr;

	brdInit();                 //initialize controller
   rn_init(RN_PORTS, 1);      //initialize controller RN ports

   device0 = -1;
   //search on ports using physical node address
   for (i=0, portnum=0000; i<RN_MAX_PORT; i++, portnum+=0100)
   {
	   if ((tmpdev = rn_device(portnum)) == -1)
		{
   		printf("\nNo device found on port %d\n", i);
      }
      else
      {
   		devaddr = (rn_devstruct *)tmpdev;
   		printf("\nDevice found on port %d\n", i);
         //rn_devtable is a global table
   		printf("Serial number 0x%02x%02x%02x%02x\n",
         	devaddr->signature[0], devaddr->signature[1],
            devaddr->signature[2], devaddr->signature[3]);
         device0 = tmpdev;
      }
   }

   if (device0 == -1)
   {
		printf("\nNo board connections!\n");
   	exit(0);
   }

	printf("\nUsing last device found ...\n\n");
   count = 0;
	tdelay = 1000;

   // first set SW watchdog timeout
	statusbyte = rn_sw_wdt(device0, SWTIMEOUT);

   // then enable SW watchdog
	statusbyte = rn_enable_wdt(device0, 2);

 	for (;;)
	{
   	costate
      {  //hit software watchdog and increase timeout
			statusbyte = rn_hitwd(device0, &timeleft);
         if (statusbyte == -1)
      	{
				printf("%s\x1B=%c%cNo connection detected, status reports: %d",
         	  	RED, 0x25, 0x32, statusbyte);
         }
         else
         {
				printf("%s\x1B=%c%cWatchdog timeout count %d  ", BLACK, 0x25, 0x2A, timeleft);
   	   	tdelay+=100;
      	   printf("%s\x1B=%c%cIncrease delay to %d ms  ", BLACK, 0x25, 0x2C, tdelay);
      		waitfor(DelayMs(tdelay));
         }
      }

      costate
      {  // busy count
			printf("%s\x1B=%c%cBusy count %d    ", BLACK, 0x25, 0x2E, count++);
      	waitfor(DelayMs(200));
      }

      costate
      {
         if (statusbyte&0x01)		//check WDT bit set
         {
	      	waitfor(DelayMs(1000));
				// reading clears reset status register
				statusbyte = rn_rst_status(device0, &recbyte);
            if (recbyte&0x40)    //check SW RST bit set
            {
					printf("%s\x1B=%c%cSoftware reset occurred, reset register reports:  0x%02x",
               		RED, 0x25, 0x30, recbyte);
            }
         }
      }

	}
}

