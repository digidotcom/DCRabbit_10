/*******************************************************************************

   reserved_read_write.c

   Digi International, Copyright (C) 2000-2008.  All rights reserved.

   Sample program to read/write User block reserved area calibration constants.

   This program does not format the data, it merely reads the data into an
   array. It should be run on any board used for development with calibration
   constants so that restoring them, if necessary, will be easy.

   Instructions to read calibration constants from User block:
      1)  Run this program with no modifications       (e.g. F9)
      2)  Acknowledge the Program Terminated dialog    (e.g. Esc)
      3)  Put the Stdio window in focus                (e.g. Ctrl-Tab)
      4)  Highlight the Stdio window contents          (e.g. Ctrl-A)
      5)  Copy the contents to the clipboard           (e.g. Ctrl-C)
      6)  Put this program in focus                    (e.g. Ctrl-Shift-Tab)
      7)  Return to Edit mode                          (e.g. F4)
      8)  Highlight the current ub_array data at the
          "   0x000  // replace with ..." line, below
      9)  Replace with calibration constants data      (e.g. Ctrl-V)
      10) Uncomment #define UB_WRITE, below
      11) Copy this modified file to a safe place      (File | Save As...)

   Instructions to restore calibration constants to User block:
      1)  Retrieve the file saved in step 11 above
      2)  Run                                          (e.g. F9)

*******************************************************************************/

#use "idblock_api.lib"

//#define UB_WRITE

// Use the reserved size that is explicitly set for this controller in its
//  hardware-specific library.
#if (!ZWORLD_RESERVED_SIZE)
   #error "This board has no reserved calibration constants area in its User" \
          " block."
   #fatal "Check your hardware manual to see if User block calibration" \
          " constants are expected."
#endif

#define UB_ADDR (4096*GetIDBlockSize()-ZWORLD_RESERVED_SIZE)
#define UB_SIZE (ZWORLD_RESERVED_SIZE - 6)  // omit markers
#define UB_INTS (UB_SIZE / sizeof(int))
#define UB_LAST (UB_INTS - 1)
#define VALUES_PER_LINE 10                  // not too narrow, not too wide
#ifdef UB_WRITE
// Constants data array to write to User block (when enabled)
const int ub_array[UB_SIZE / 2] =
{
   0x0000  // replace with calibration constants data - steps 8 and 9 above
};
#else
int ub_array[UB_SIZE / 2];  // Variable data array for reading User block
#endif

main()
{
	int i, n;

   #ifdef UB_WRITE
   if (writeUserBlock(UB_ADDR, ub_array, UB_SIZE))
   {
      printf("cannot write User block\n");
   }
   else  // continue with read and report
   #endif
   if (readUserBlock(ub_array, UB_ADDR, UB_SIZE))
   {
      printf("cannot read User block\n");
   }
   else
   {
      for (i = 0 ; i < UB_INTS; ++i)
      {
         if (!(i % VALUES_PER_LINE))
         {
            printf("   ");                  // indent lines for step 9 above
         }

         printf("0x%04x", ub_array[i]);     // All hex ints, no formatting
         if (i < UB_LAST)
         {
            printf(",");
         }

         if (!((i + 1) % VALUES_PER_LINE))  // number of values in full lines
         {
            printf("\n");
         }
      }
   }
}

