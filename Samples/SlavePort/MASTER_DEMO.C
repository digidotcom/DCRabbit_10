/*******
	Samples\SlavePort\master_demo.c

	connected to a slave with a looped back serial port on channel 0x42

	This sample program is set up for either the RCM2200 or RCM3000 on its
	prototyping board.  If you have changed the PORT E pin you need to change
	the iobase parameter passed to MSinit.  The base address is conditionally
	defined to 0 or 7 for the RCM2200 or RCM3000 protoboard, respectively.
	Since the RCM3000 is using the new aux I/O bus of the Rabbit 3000, there is
	a conditional define of PORTA_AUX_IO to enable the bus.

************/
#class auto


#if (CPU_ID_MASK(_CPU_ID_) >= R3000)
  #define PORTA_AUX_IO
#endif

#use "master_serial.lib"

#define SP_CHANNEL 0x42

char* const test_string = "Hello There";

void main()
{
	char buffer[100];
	int read_length;

#if (CPU_ID_MASK(_CPU_ID_) < R3000)
	MSinit(0);	// 0 for the RCM2200 Dev Board
#else
	MSinit(7);	// 7 for the RCM3000 Dev Board
#endif

	//comment this line out if talking to a stream handler
	printf("open returned:0x%x\n", MSopen(SP_CHANNEL, 9600));

	while(1)
	{
		costate
		{
			wfd{ cof_MSwrite(SP_CHANNEL, test_string, strlen(test_string)); }
			wfd{ cof_MSwrite(SP_CHANNEL, test_string, strlen(test_string)); }
		}

		costate
		{
			wfd{ read_length = cof_MSread(SP_CHANNEL, buffer, 99, 10); }
			if(read_length > 0)
			{
				buffer[read_length] = 0; //null terminator
				printf("Read:%s\n", buffer);
			}
			else if(read_length < 0)
			{
				printf("Got read error: %d\n", read_length);
			}
			printf("wrfree = %d\n", MSwrFree(SP_CHANNEL));
		}
	}
}