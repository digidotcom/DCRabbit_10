/***********************************************************

      ximport.c
      Z-World, 1999

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

