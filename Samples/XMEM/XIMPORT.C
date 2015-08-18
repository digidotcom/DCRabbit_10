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
/***********************************************************

      ximport.c

      Demonstration of the ximport compiler directive.

      Usage:
      #ximport "filename" symbolname

      Load the "filename" file length into xmem at the
      next xcodorg location.  Follow the length with
      the contents of the file.  Create a constant MACRO
      "symbolname" which contains the physical address 
      where "filename" length and contents were stored.

      This program will print out the physical address 
      where length and contents of tempfile are stored 
      followed by the contents of tempfile.

************************************************************/
#class auto

#ximport "samples\xmem\tempfile.txt" tempfile

void main()
{
	long x,length;
	char buffer[256];
	
	printf("physical address: %06lx\n",(long)tempfile);

	xmem2root(&length,tempfile,sizeof(long));

	if(length>256)  // only print first 256 characters of file
		length=256;
		
	xmem2root(buffer,tempfile+4,(int)length);

	for(x=0;x<length;x++) {
		printf("%c",buffer[(int)x]);
	}
}

