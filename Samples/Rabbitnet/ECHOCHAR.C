/*******************************************************************
   echochar.c
   Z-Word Inc 2003

	This sample program is for any RabbitNet board.

	Description
	===========
	This program demonstrates a simple character echo to any RabbitNet
   board.

   This program will first look for a device directly connected to
   each controller port using rn_device().  The last device found
	will echo characters sent by the controller.

	Connections
	===========
   Connect RabbitNet and power supply cables to the controller
   and RN boards as described in the RabbitNet Manual.

	Instructions
	============
 	1. Set PRINTSTATS below to display status byte descriptions.
	2. Compile and run this program.

*******************************************************************/
#class auto

//////
// Define as 1 or 0 to display or not display status bit descriptions
//////
#define PRINTSTATS 1

const char statstr[9][128] = {
	"bit 0 Reset occured, check control register\0",
	"bit 1 Command rejected, try again\0",
	"bit 2 Reserved\0",
	"bit 3 Reserved\0",
	"bit 4 Communication error, check comm status register\0",
	"bit 5 Reserved\0",
	"bit 6 Device Ready\0",
	"bit 7 Device Busy, try again\0",
   "No Connection\0"
   };

void printstat(int statusbyte)
{
	auto int i;

	if (statusbyte == -1)
	   printf("Status %d: %s\n", statusbyte, statstr[8]);
   else
   {
	   printf("Status 0x%02x description:\n", statusbyte);
		for (i=0; i<8; i++)
   	{
   		if ((statusbyte>>i)&1)
	      	printf(" %s\n", statstr[i]);
	   }
   	printf("\n");
   }
}

nodebug
void msDelay(unsigned int delay)
{
	auto unsigned long done_time;

	done_time = MS_TIMER + delay;
   while( (long) (MS_TIMER - done_time) < 0 );
}


main()
{
	auto int device0, tmpdev, portnum, i, statusbyte;
	auto char sendbyte, recbyte;
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

	printf("\nEcho characters on last device found ...\n\n");
	while (1)
	{
		for (sendbyte='A'; sendbyte<='Z'; sendbyte++)
		{
			statusbyte = rn_echo(device0, sendbyte, &recbyte);
			printf("Send %c, receive %c\n", sendbyte, recbyte);
         if (PRINTSTATS) printstat(statusbyte);
         msDelay(500);
		}
	}
}

