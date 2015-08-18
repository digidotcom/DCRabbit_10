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
/********************************************************************

ERROR_MESSAGE_TEST.C

This program exercises the error_message function for all error codes
between 0 and 799 inclusive in the positive and negative range. If
the error_message function fails to find the error code, the main
routine will print "Unknown error," otherwise it will print the error
message returned using the handy %ls format specifier.

********************************************************************/


void main()
{
	int i, j;
   unsigned long err_string;

   for (i = 0; i < 800; ++i) {
   	j = i % 2 ? i : -i;
      err_string = error_message(j);
      printf("%5d: ", j);
      if (!err_string) {
      	printf("Unknown error\n");
      }
      else {
	   	printf("%ls\n", err_string);
      }
   }

   for (i = 0; i < 800; ++i) {
   	j = i % 2 ? -i : i;
      err_string = error_message(j);
      printf("%5d: ", j);
      if (!err_string) {
      	printf("Unknown error\n");
      }
      else {
	   	printf("%ls\n", err_string);
      }
   }
}