/*****************************************************

	Samples\qsortdemo.c
     
	Z-World, 2000

	Demonstration of qsort() function

	Output from this program:
	0. -90, 12
	1. -2, 1
	2. 1, 3
	3. 3, -2
	4. 7, 16
	5. 9, 7
	6. 10, 9
	7. 12, 34
	8. 16, -90
	9. 34, 10

*******************************************************************/
#class auto


// initialized data gets put in flash!
const int Q[10]={12,1,3,-2,16,7,9,34,-90,10};

#define  ARRAY_ELEMENT_SIZE	(sizeof(Q[0]))
#define  ARRAY_ELEMENT_COUNT	(sizeof(Q)/sizeof(Q[0]))

int p[ ARRAY_ELEMENT_COUNT ];

// user defined compare must be supplied for qsort
int mycmp(int *p,int *q){
	return (*p - *q);
}

void main()
{
	int i;

	// copy initialized data to RAM
	memcpy(p,Q,sizeof(Q));

	// sort it
	qsort(p,ARRAY_ELEMENT_COUNT,2,mycmp);

	// display results
	for(i=0;i<ARRAY_ELEMENT_COUNT;++i) {
		printf("%4d.  %4d, %4d\n",i,p[i],Q[i]);
	}
}
