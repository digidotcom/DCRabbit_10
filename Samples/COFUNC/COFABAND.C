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

      Cofaband.c
   
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