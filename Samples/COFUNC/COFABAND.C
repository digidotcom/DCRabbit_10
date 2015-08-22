/*****************************************************

      Cofaband.c
      Z-World, 1999
   
      Example of a cofunction abandonment.
 
      In this example two parts of main are requesting access
      to SCofTest(int).  The first request is honored and the
      second request is held.  When loop head notices that
      the first caller is not being called each time around
      the loop, it cancels the request, calls the abandonment
      code and allows the second caller in.
 
******************************************************/
#class auto

scofunc SCofTest(int i)
{
	abandon {
		printf("CofTest was abandoned\n");
	}
	
	while(i>0) {
		printf("CofTest(%d)\n",i);
		yield;
	}
}

main()
{
	int x;
	
	for(x=0;x<=10;x++) {
		loophead();
		
		if(x<5) {
			costate {
				wfd SCofTest(1); // first caller
			}
		}
		
		costate {
			wfd SCofTest(2); // second caller
		}
	}
} 