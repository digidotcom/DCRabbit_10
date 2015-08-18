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
/*******************************************************************************

      Samples\RestartVars\BatteryRAM\bbvars.c

      This sample shows how to preserve variables across reboots.  The
      values are stored in battery-backed RAM.  Make sure your battery
      is attached!  When main power is removed, the battery preserves
      the real-time clock inside the Rabbit, and saves the contents of
      the static RAM.

      The trick here is to know if the values are yours, or some garbage.
      For the first time, your code must initialize those values with
      defaults.  If the values are good, then your program can continue
      using them.

      This application is simple and assumes the values are located at
      the same memory location (other samples relax this restriction).
      It is important the variables are located at the same place each
      time the program runs.  If you want to change the code but preserve
      important values, then RAM might not work for you; try Flash.

      A structure of all important values is declared.  The first field
      is a CRC check of the rest of the structure.  A false positive is
      very rare (depends on the size of the structure, though).

      RABBIT MEMORY LAYOUT
      ====================
		Data is stored starting at a fixed point in the logical memory map.
      This is different than most 16-bit and 32-bit programs where the
      "data" origin is right after the code.  Thus Rabbit "code" can grow
      or shrink, and your data variables might not change.

      Because of the MMU in the Rabbit 2000/3000 MPU, data always begins
      about the same memory location.  The lower nibble of SEGSIZE is the
      (root) base for data.  DATASEG provides the upper 16 bits (of the
      20 bit base address) of the physical address where "data" is mapped
      into.

*/
#class auto					/* Change local var storage default to "auto" */



/*
 *  Structure of variables to save.  The first field is the CRC.  It provides
 *  a checksum over the rest of the structure.
 */
struct ImportantValues {
	word		_crc;
   int 		user_id;
   char		message[50];
	/* .. any other fields .. */
};


///////////////////////////////////////////////////////////////////////

/*
 *  Define the structure of important values.  It must be located at the
 *  same memory location every time the program runs.  Or you must add
 *  some indirection to _find_ the structure whenever your program runs.
 */
#if defined BBROOTDATA_SIZE && BBROOTDATA_SIZE > 0
// If we have a choice, use the battery backed SRAM!
bbram struct ImportantValues	system_values;
#else
// Otherwise, assume that all SRAM is battery backed.
struct ImportantValues	system_values;
#endif


///////////////////////////////////////////////////////////////////////

/**
 * 	Check the Important Values to see if they're valid.
 * 	RETURN:	TRUE : values are good, FALSE otherwise.
 */
int  test_important_values()
{
	auto word		chk;

   chk = getcrc((char*)&system_values + sizeof(system_values._crc),
   				 	sizeof(system_values)-sizeof(system_values._crc),
                  0 );
   printf( "chk = %04x   want = %04x\n", chk, system_values._crc );
   return (int)(chk == system_values._crc);
}   /* end test_important_values() */


/**
 * 	Recompute the checksum stored along with the important values.
 * 	Call here is any value changes.
 * 	NOTE: Between time new value generated and when the CRC value is
 * 		stored, the important values are technically "corrupt."  If
 * 		the CPU resets between value change and CRC storing, the block
 * 		will be corrupt.
 */
void rechecksum_important_values()
{

   /*ASSERT:*/ if( sizeof(system_values) > 255 ) { printf("CRC has max size 255 bytes"); exit(2); }
   system_values._crc = getcrc( (char*)&system_values + sizeof(system_values._crc),
   				 			  			sizeof(system_values)-sizeof(system_values._crc),
                     				0 );

}   /* end rechecksum_important_values() */


/**
 * 	Store initial values into the "important values" structure.
 * 	Call when program discovers the important values are corrupt.
 */
void initialize_important_values()
{

	system_values._crc = 0xffff;
   system_values.user_id = -1;
   strcpy( system_values.message, "Rabbit MPU!" );

   rechecksum_important_values();
}   /* end initialize_important_values() */


///////////////////////////////////////////////////////////////////////

void main()
{
	char	xx[ 90 ];
	int 	rc, done;
   char 	ch;


   for( done = FALSE ; ! done ; ) {
   	printf( "\ncommand (? for help) [ ]\010 " );
      ch = getchar();
      printf( "\010%c]\n", ch );

      switch( tolower(ch) ) {
      	case '?' :
         case 'h' :
         			printf( "help restart vars:\n" \
                  			" C = change message, I = initialize,\n" \
                           " P = print message, V = verify, Q = quit\n" );
              		break;

         case 'i' :
						initialize_important_values();
                  break;

			case 'c' :
         			printf( "new message => " );
                  gets( xx );
                  strncpy( system_values.message, xx, sizeof(system_values.message)-1 );
                  rechecksum_important_values();
                  break;

         case 'p' :
         			printf( "message \"%s\"\n", system_values.message );
                  break;

         case 'v' :
						rc = test_important_values();
                  if( !rc) {
                  	printf( "checksum invalid\n");
                  } else {
                  	printf( "checksum is valid\n");
                  }
						break;

         case 'q' :
         			done = TRUE;
                  break;
         case '\0' :
         			break;
         default :
         			printf( "unknown cmd \"%c\" .  Try \"help\"\n", ch );
		}
   }
   exit(0);
}	/* end main() */

