/*****************************************************

	 xdata.c
	 
	 Z-World, Inc, 2001

    Example of using xdata.

    This program demonstrates how to put various types
    of numeric and character data into xdata constants.

    The compiler makes the names following "xdata" unsigned
    long variables which are initialized with the physical
    addresses of the location of the specified data.
        		
    The data can viewed directly in the dump window
    by doing a PHYSICAL memory dump at the value contained
    in chars1 and output by the program.

    Something like this should be seen in the dump
    window:
    
006100 41 42 43 44 45 41 42 43 44 45 46 47 48 49 4A 20 ABCDEABCDEFGHIJ 
006110 20 52 61 62 62 69 74 20 52 75 6C 65 73 21 20 20  Rabbit Rules!  
006120 00 52 00 53 00 54 00 55 55 56 56 52 00 53 00 54  R S T UUVVR S T
006130 00 55 55 55 56 57 57 57 57 58 58 58 58 59 59 59  UUUVWWWWXXXXYYY
006140 59 5A 5A 5A 5A 00 00 80 3F 00 00 00 40 00 00 40 YZZZZ   ?   @  @
006150 40 00 00 80 40 00 00 A0 40 00 00 C0 40

	The program prints out the float values to demonstrate
	accessing the data, and because floats are hard to
	read in their raw hex format.
******************************************************/
#class auto

#define NFLOAT 6

// 1 byte length data
xdata chars1 {(char)65,(char)66,(char)67,(char)68,(char)69};
xdata chars2 {(char)0x41,(char)0x42,(char)0x43,(char)0x44,(char)0x45};
xdata chars3 {'\x46','\x47','\x48','\x49','\x4A','\x20','\x20'};
xdata chars4 {'R','a','b','b','i','t'};
xdata chars5 {" Rules!  "};

//**** 2 byte length data, if an integer fits into one or two
//****   bytes, then it is allocated two bytes
xdata ints1 {0x52,0x53,0x54,0x00005555,0x5656U};
xdata ints2 {82,83,84,21845,22101};

//**** 4 byte length data, if an integer requires 4 bytes,
//****  then it is allocated 4 bytes. Floats are treated as
//****  such and given 4 bytes.
xdata longs {0x57575757L,0x58585858UL,0x59595959,1515870810};
xdata floats {1.0,2.0,(float)3,40e-01,5e00,.6e1};

void main()
{

	float farray[NFLOAT];
	int i;

	printf("xdata starts @ %06lx (physical)\n\n",chars1);

	//**** Copy float data to root array.
	xmem2root(farray,floats,sizeof(float)*NFLOAT);

	for(i=0;i<NFLOAT;i++)
	{
		printf("farray[%d] = %f\n",i,farray[i]);
	}
}
