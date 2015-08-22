/*****************************************************************************

	TamperDetection_ReadMe.C

	This sample is not included for the RCM43xx family.

   Tamper Detection detects any attempt to enter bootstrap mode.  When this
   is detected, the VBAT RAM is erased.  Since the serial bootloader on the
   RCM43xx family uses bootstrap mode to load the SRAM, this memory is
   always erased on any reset.

*****************************************************************************/

