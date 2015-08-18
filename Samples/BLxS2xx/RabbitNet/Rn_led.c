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
/********************************************************************
	rn_led.c

	This sample program is for any RabbitNet board being used with
	the BLxS2xx series controllers.

	Description:
	============
	This program displays basic information for the given RabbitNet
   board and will continously flash the LED on the attached RabbitNet
   board(s) to indicate communication activity.

	Connections:
	============
   Connect RabbitNet and power supply cables to the controller
   and RN boards as described in the RabbitNet Manual.

	Instructions:
	=============
	1. Compile and run this program.
   2. View STDIO window to see information on the RabbitNet board.

********************************************************************/
//  Set a default of declaring all local variables "auto" (on stack)
#class auto

// include BLxS2xx series library
#use "BLxS2xx.lib"

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


main()
{
	auto int device0, device1, portnum;
   auto char sendbyte;
   auto rn_devstruct *devaddr;

   // Initialize the controller
	brdInit();

   // Initialize controller RN ports
   rn_init(RN_PORTS, 1);

   //search on ports using physical node address
 	portnum=0000;
	if ((device0 = rn_device(portnum)) == NOCONNECT)
	{
   	printf("\n\n*** No device found on port %d\n\n", 0);
   }
   else
   {
		// device0 is actually an address, cast it as such.
   	devaddr = (rn_devstruct *)device0;
   	printf("\n\n*** Device found on port 0\n");
      printf("- Product ID 0x%04x\n", devaddr->productid);
      printf("- Serial number 0x%02x%02x%02x%02x\n",
      	    devaddr->signature[0], devaddr->signature[1],
             devaddr->signature[2], devaddr->signature[3]);
   }

   portnum=0100;
	if ((device1 = rn_device(portnum)) == NOCONNECT)
	{
   	printf("\n\n*** No device found on port 1\n\n");
   }
   else
   {
		// device1 is actually an address, cast it as such.
   	devaddr = (rn_devstruct *)device1;
   	printf("\n\n*** Device found on port 1\n");
      printf("- Product ID 0x%04x\n", devaddr->productid);
      printf("- Serial number 0x%02x%02x%02x%02x\n",
      	    devaddr->signature[0], devaddr->signature[1],
             devaddr->signature[2], devaddr->signature[3]);
   }


   if(device0 == NOCONNECT && device1 == NOCONNECT)
   {
		printf("\nNo board connections!\n");
   	exit(-ETIMEDOUT);
   }

   printf("\n\n");
   printf("Toggling RabbitNet board communication activity LED");
	while(1)
  	{
   	if(device0 != NOCONNECT)
      {
   		// Write to the echo command register
      	rn_write(device0, 0, &sendbyte, 1);
         msDelay(700);
		}
      if(device1 != NOCONNECT)
      {
   		// Write to the echo command register
      	rn_write(device1, 0, &sendbyte, 1);
         msDelay(700);
		}
   }
}



