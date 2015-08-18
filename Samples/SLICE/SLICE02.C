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

    slice02.c

    This example shows how slice statements can be used
    to build a custom scheduler.  In this case the first
    and second tasks are allowed 500 ticks to run each
    and the background task it allowed any ticks left
    upto the looptime.

******************************************************/
#class auto


unsigned int looptime, task1slice, task2slice;

int Task1()
{
	; // first task's code
}

int Task2()
{
	; // second task's code
}

BackgroundTask()
{
	; // background task's code
}
 
void main()
{
	long bgtimer,timeleft;
	
	looptime=1000;
	task1slice=500;
	task2slice=500;

	for(;;) {
		bgtimer = TICK_TIMER + looptime;
		
		slice(200,task1slice) {
			Task1();
		}
			
		slice(200,task2slice) {
			Task2();
		}

		timeleft = bgtimer-TICK_TIMER;
			
		if(timeleft>=0) {			
			slice(200,(int)timeleft) {
				BackgroundTask();
			}
		}
	}
}
