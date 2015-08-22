/*****************************************************

     costate1.c
     Z-World, 1999

     Example of costatement use.

     Exactly one iteration of the for loop gets executed on each pass through
     the endless while loop.  The second costatement checks whether 500 milli-
     seconds have passed since the program first entered it.  It will print the
     value of i if 500 milliseconds have passed.

     The result is a loop that does two things concurrentely.  The code will
     output the value of i every half second, and the for loop increments i
     (and might do other things).  The process will go on forever since both
     costatements are in an endless loop.

******************************************************/
#class auto

main()
{
	int	i;
	
	while (1) {				// endless loop
	
		costate {
			for (i=0; i<30000; i++) {
				// do something here, if desired
				yield;								// pass execution on to next costatement
			}
		}			
			
		costate {
			waitfor(DelayMs(500));				// waits until 500 ms have passed since
			printf("i = %d\n", i);				// first called, then prints i
		}
	}
}
