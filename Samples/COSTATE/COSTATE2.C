/*****************************************************

     costate2.c
     Z-World, 1999

     Example of advanced CoData usage.

     If all your costatements will be using the same code (for example, each
     costatement controls 'N' different machines on an assembly line, but they 
     are all the same type of machine performing the same task), then a more 
     elegant method can be used then repeating the code in 'N' costatements.
     The program below is more space efficient than writing each costatement
     separately.

******************************************************/
#class auto

#define N		10			// number of machines

CoData Machine[N];		// array of CoData blocks
CoData *pMachine;			// pointer to current machine


main()
{
	int	i;
	
	for (i=0; i<N; i++) {
		CoBegin(&Machine[i]);		// enable machine
	}

	for (;;) {							// endless loop

		for (i=0; i<N; i++) {

			pMachine = &Machine[i];				// pMachine points to current machine
			
			costate pMachine always_on {

				// identical code for all machines goes here
				printf("Machine %d executing...\n", i);
			}			

		}			
	}
}


////////////////////////////////////////////////////////////////////////////////


