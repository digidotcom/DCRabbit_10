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

	 xmemdemo.c

    Example of using the XMEM access functions root2xmem and xmem2root.

	 This program demonstrates the use of the XMEM functions by writing a block
    of data to a physical memory location and then reading it back, first with
    all zeros and then with the entire buffer set to 0x55.

******************************************************/
#class auto

void main()
{
	static char	buffer1[1024];				// buffer for data to write
	static char  buffer2[1024];				// buffer to read data back into
	static unsigned long physaddr;			// physical memory address to write to
		
	physaddr = xalloc(1024);		// physical memory address (SRAM)


	// clear buffer1	
	memset(buffer1, 0x00, 1024);

	// write buffer1 to memory
	printf("Writing buffer1 to memory\n");
	root2xmem(physaddr, buffer1, 1024);
				
	// read data from memory into buffer2
	printf("Reading memory into buffer2\n");
	xmem2root(buffer2, physaddr, 1024);

	// compare the two buffers using string compare fcn strncmp
	//		strncmp returns 0 if strings are identical
	if (strncmp(buffer1, buffer2, 1024))
		printf("Error in write/read to XMEM\n");
	else
		printf("Buffers match\n\n");
			

	// set buffer1	to new value
	memset(buffer1, 0x55, 1024);

	// write buffer1 to memory
	printf("Writing buffer1 to memory\n");
	root2xmem(physaddr, buffer1, 1024);
				
	// read data from memory into buffer2
	printf("Reading memory into buffer2\n");
	xmem2root(buffer2, physaddr, 1024);

	// compare the two buffers using string compare fcn strncmp
	//		strncmp returns 0 if strings are identical
	if (strncmp(buffer1, buffer2, 1024))
		printf("Error in write/read to XMEM\n");
	else
		printf("Buffers match\n\n");

}
