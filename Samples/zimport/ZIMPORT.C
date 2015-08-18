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

      zimport.c

      Demonstration of the zimport compiler directive.

      Usage:
      #zimport "filename" symbolname

      The usage of #zimport is identical to #ximport,
      except that the imported file is compressed, and
      the top bit of the length quantity is flipped to
      signify the imported file is compressed. The
      zimport.lib library defines a macro to mask out this
      bit called ZIMPORT_MASK.

      In order to read the compressed file, the
      zimport.lib library must be used.

      Load the "filename" file length into xmem at the
      next xcodorg location.  Follow the length with
      the contents of the file.  Create a constant MACRO
      "symbolname" which contains the physical address
      where "filename" length and contents were stored.

		The LZ compression algorithm used by zimport (based
		upon the freely available LZ77 algorithm) utilizes
		a 4KB internal buffer (in RAM) for decompression, and
		a 24K buffer for compression (compression also requires
		a 4K input buffer, which is allocated automatically for
		each output buffer that is defined). The zimport
		compression library handles memory management
		internally by pre-allocating these buffers. The
		default is a single 4KB input (decompression) buffer,
		and no output (compression) buffer.

		The number of buffers to be used can be controlled
		through the use of two macros:
		OUTPUT_COMPRESSION_BUFFERS  (default = 0) and
		INPUT_COMPRESSION_BUFFERS   (default = 1)
		These numbers should coincide with the largest
		possible number of simultaneous open ZFILE's for
		output and input, respectively.

      This program uses zimport to import and compress a
      sample file and then decompresses the imported file
      to stdio.

************************************************************/
#class auto 			// Change default storage class for local variables: on the stack

#memmap xmem
// Include the #zimport support library
#use "zimport.lib"

// Import and compress a file
#zimport "samples/zimport/sample.txt" compressed_file

// The number of bytes to read at a time.
// This can be adjusted to get a little more
// throughput or free up variable space.
// Increase number for speed, decrease for size.
// (This sample only)
#define BYTES2READ      30

main () {
	auto ZFILE inf1; // The ZFILE struct is used for compressed files
	auto int bytes_read;
	auto long length;
	static unsigned char outbuf[BYTES2READ+1];

	// First, read in the length and make sure the file is compressed
	// using the ZIMPORT_MASK to get the actual length and to check
	// the compression flag bit.
	xmem2root(&length,compressed_file,sizeof(long));
	printf("File length: %lx\n",(long)(length & ZIMPORT_MASK));
	if(!(length & ~(ZIMPORT_MASK))) {
		printf("Trying to open an ximport file as a compressed file.");
		exit(1);
	}

	// Open #zimport file for reading
	if(!OpenInputCompressedFile(&inf1, compressed_file)) {
		printf("Error opening #zimport file.\n");
		exit(1);
	}

	// This is the read loop, do decompression on-the-fly
	while(bytes_read = ReadCompressedFile(&inf1, outbuf, BYTES2READ)) {
		outbuf[bytes_read] = 0;  // Null terminate for printf
		printf("%s", outbuf);    // print to stdio
   }

	// Close our file to release memory
	CloseInputCompressedFile(&inf1);

	printf("\n\n*** DONE ***\n");
	exit(0);
}