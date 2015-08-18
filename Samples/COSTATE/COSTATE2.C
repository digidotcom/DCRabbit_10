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
/*****************************************************

     costate2.c

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


