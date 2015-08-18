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
/*************************************************************************

   slice01.c

   This example demonstrate the basic functionality of a slice statement.
   The first number in the slice statement is the size of the stack.  It
   needs to be at least large enough for the worst case stack usage by the
   user program and interrupt routines.

   The program alternates incrementing two independent counters (x, y)
   while also incrementing a common counter (x_y) in two tight loops.  The
   first tight loop is allowed to run for 25 ticks and the second for 50
   ticks.  After each tight loop has had a chance to run the values of the
   counters are output to the stdio window and the process is repeated by
   the outside loop.

   The expected behavior is that x is incremented about 1/2 as fast as y.
   This will not be exact because y has less overhead, having only 1/2 as
   many context switches.  The lower the ticks per slice the more the
   overhead will influence the amount of work you can get done in each
   pass.

   Notice the use of shared variables for the two independent counters, x
   and y.  This is _IMPORTANT_ to insure the atomic updating of the values
   at the end of the increment operations.  An increment of either of
   these independent counters may be started at the end of the slice but
   not completed if the slice ends before the store operation actually
   begins.  However, once the store operation has begun on a shared
   variable, it is guaranteed to complete before the slice is permitted to
   end.

   In contrast, the common x_y counter is not declared to be a shared
   variable.  Notice how the x_y counter apparently randomly and
   increasingly lags behind the sum of the two independent counters, x+y.
   The rate of lag is much greater than can be explained by missing an
   occasional single increment store operation; instead, it is explained
   by occasionally half of the unsigned long being updated while the other
   half is not updated at the end of the slice statement.

*************************************************************************/
#class auto

shared unsigned long x,y;
unsigned long x_y;

void main()
{
	x = y = 0;				// initialize the counters
	x_y = 0;

	for (;;) { 				// outside loop

		slice (200, 25) {	// run this section for 25 ticks
			for (;;) {
				x++;
				x_y++;
			}
		}

		slice (200, 50) {	// run this section for 50 ticks
			for (;;) {
				y++;
				x_y++;
			}
		}
		printf("x=%08lu, y=%08lu, x+y=%08lu, x_y=%08lu\n",
		       x, y, x + y, x_y);	// print the results
	}
}

