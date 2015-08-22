/*****************************************************

    slice02.c
    Z-World, 1999

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
