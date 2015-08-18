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

  Samples\xalloc_stats.c

  Useful little program to show available xalloc() areas,
  and allows experimentation with the new _xalloc()
  function.

************************************************************/
#class auto

int main()
{
	word a;
   char s[80];
   long sz, szo, addr, max_avail;

   xalloc_stats(xubreak);

   printf("Enter alignment (0-16):\n");
   gets(s);
   a = atoi(s);
   if (a > 16) {
   	printf("Setting alignment to 0.\n");
      a = 0;
   }
   printf("OK, alignment on %ld boundary.\n", 1L<<a);

   for (;;) {
   	printf("\nLargest available blocks (with alignment of 2**%u):\n", a);
      printf("  normal: %ld\n", _xavail(NULL, a, XALLOC_NOTBB));
      printf("  BB RAM: %ld\n", _xavail(NULL, a, XALLOC_BB));
      printf("     any: %ld\n\n", _xavail(NULL, a, XALLOC_ANY));
   	printf("Enter amount to allocate:\n");
      gets(s);
      sz = szo = atol(s);
      if (sz < 0) {
      	printf("Must be non-negative\n");
         continue;
      }
      // limit max_avail to an even number to prevent run-time error due
      // to xalloc rounding up an odd-sized request
      max_avail = _xavail(NULL, a, XALLOC_ANY) & ~1L;
      if (sz > max_avail) {
      	printf("Too much!  Max available is %ld\n", max_avail);
         continue;
      }
      addr = _xalloc(&sz, a, XALLOC_MAYBBB);
      printf("Got %ld bytes (%ld more than requested) at addr = %08lx\n\n",
      	sz, sz - szo, addr);
      xalloc_stats(xubreak);
   }
}