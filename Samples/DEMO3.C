/*****************************************************

     demo3.c
      
     Digi International, Copyright (C) 2000-2008.  All rights reserved.

	  Sample program for Dynamic C Premier tutorial
******************************************************/

main()
{
	int	secs;							// seconds counter

	secs = 0;							// initialize counter
	while (1) {							// endless loop
		
		// First task will print the seconds elapsed
		
		costate {
			++secs;
			waitfor(DelayMs(1000));
			printf("%d secs\n", secs);
		}

		// Second task will respond if any keys are pressed
		
		costate {
			if ( !kbhit() )	abort;	// check if key was pressed

			printf("  key pressed = %c\n", getchar());
		}
		
	}	// end of while loop	
}		// end of main

