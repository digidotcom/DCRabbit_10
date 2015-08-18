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

      Cofet.c
      
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