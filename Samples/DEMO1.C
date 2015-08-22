/*****************************************************
            
     demo1.c
     
     Digi International, Copyright (C) 2000-2008.  All rights reserved.

	  Sample program for Dynamic C Premier tutorial
******************************************************/

main() {

	int i, j;

	i = 0;
	
	while (1) {
		++i;

		for (j=0; j<20000; ++j);
		
		printf("i = %d\n", i);
	}
}