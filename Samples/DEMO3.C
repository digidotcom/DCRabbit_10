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

     demo3.c
      
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

