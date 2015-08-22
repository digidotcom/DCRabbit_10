/*******************************************************************
   hwatchdog.c
   Z-Word Inc 2003

	This sample program is for any RabbitNet board.

	Description
	===========
	This program demonstrates setting the hardware watchdog on a
   RabbitNet device.

   This program will first look for a device directly connected to
   each controller port using rn_device().  The last device found
	will be used.

   The hardware watchdog will be set and a hardware reset should
   occur in approximately 1.5 seconds.  The hardware watchdog
   will be disabled after reset and the hardware reset bit will
   be set.

   Note:  The function demo_read() used in this program only
	demonstrates that the hardware watchdog timeout will reset
   the device.  It should not be used in applications.


	Connections
	===========
   Connect RabbitNet and power supply cables to the controller
   and RN boards as described in the RabbitNet Manual.

	Instructions
	============
	1. Compile and run this program.
   2. Watch busy counter until the display for hardware reset appears.

*******************************************************************/
#class auto

// screen foreground colors
#define	BLACK		"\x1b[30m"
#define	RED		"\x1b[31m"


//////
// This function for this demonstration only by leaving device
// select asserted to force a hardware watchdog timeout
//////
int demo_read(int handle, int regno, char *recdata, int datalen)
{
	auto rn_devstruct *devaddr;
   auto rnDataSend ds;
   auto rnDataRec dr;
	auto unsigned long done_time;

   // slight delay
   done_time = MS_TIMER + 200;
   while( (long) (MS_TIMER - done_time) < 0 );

	devaddr = (rn_devstruct *)handle;

	//assemble data
   ds.cmd = RCMD|regno;
   memset(ds.mosi, ds.cmd, datalen);
   datalen++;

	rn_sp_enable(devaddr->portnum);
 	_mosi_driver(datalen, &ds, &dr, &devaddr->cmdtiming, &rn_spi[devaddr->portnum]);

   return (dr.statusbyte);
}


main()
{
	auto int device0, tmpdev, portnum, i, count, statusbyte;
	auto char recbyte;
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

   // then enable HW watchdog
	statusbyte = rn_enable_wdt(device0, 1);

   // force watchdog timeout
 	statusbyte = demo_read(device0, 0000, &recbyte, 1);

 	for (;;)
	{
      costate
      {  // busy count
			printf("%s\x1B=%c%cBusy count %d    ", BLACK, 0x25, 0x2A, count++);
      	waitfor(DelayMs(200));
      }

      costate
      {
      	waitfor(DelayMs(5000));     //plenty time for HW reset to finish
			rn_sp_disable(--i);   //demo only to deassert device select

			// reading clears reset status register
			statusbyte = rn_rst_status(device0, &recbyte);
         if (statusbyte == -1)
      	{
				printf("%s\x1B=%c%cNo connection detected, status reports: %d",
         	  	RED, 0x25, 0x2D, statusbyte);
         }
         else
         {
	         if (statusbyte&0x01)	//check watchdog timeout bit set
   	      {
					printf("%s\x1B=%c%cWatchdog timeout, status byte reports: 0x%02x",
         	   	RED, 0x25, 0x2D, statusbyte);
	         }
   	      if (recbyte&0x80)    //check hard reset bit set
      	   {
					printf("%s\x1B=%c%cHardware reset occurred, reset register reports: 0x%02x",
            			RED, 0x25, 0x2E, recbyte);
	         }
         }
      }

	}
}

