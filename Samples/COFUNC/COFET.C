/*****************************************************

      Cofet.c
      Z-World, 1999
      
      Example of a cofunction with an everytime block in it.
 
      This example demonstrates the correct everytime behaviour
      inside a cofunction.  The everytime block is executed before
      each continuation call of a cofunction.  In this case the
      "i" variable is reset to 4 each time (except the first) CofTest 
      is called.
 
******************************************************/
#class auto

cofunc CofTest()
{
	int i;
	
	everytime {
		i=4;
	}
	
	i=3;
	while(i>0) {
		printf("i=%d\n",i--);
		yield;
	}
}

main()
{
	for(;;) {
		costate {
			wfd CofTest();
		}
	}
}