/***************************************************************************

	Samples\predefined.c
	Digi International, Copyright © 2001-2009.  All rights reserved.

    This program demonstrates shows how the contents of the predefined
    macros __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__,
    _DC_GMT_TIMESTAMP_ and CC_VER can be displayed.

***************************************************************************/
#class auto


void main()
{
   printf("Function %s in file %s\n", __FUNCTION__, __FILE__);
	printf("Compiled on %s at %s\n", __DATE__, __TIME__);
	printf("(%lu seconds since January 1, 1980)\n", _DC_GMT_TIMESTAMP_);
   printf("Current line number: %d\n", __LINE__);
   printf("Current line number: %d\n", __LINE__);

   printf("Compiler version %d.%02x\n", CC_VER >> 8, CC_VER & 0x0FF);
}