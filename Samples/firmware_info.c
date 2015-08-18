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
/*
	Samples/firmware_info.c

	Display this program's firmware info contents.  Works on all boards,
	not just the ones supported by Remote Program Update.  View function
	help on firmware_info_t to learn about the elements of that structure,
	and look at the source to fiDump (in firmware_info.lib) to see how
	it makes use of that information.
*/

void main()
{
	firmware_info_t	fi;
	int err;

	err = fiProgramInfo( &fi);
	if (err)
	{
		printf( "error %d calling %s\n", err, "fiProgramInfo");
		exit(err);
	}

	printf( "Firmware information embedded in BIOS:\n");
	fiDump( &fi);
}