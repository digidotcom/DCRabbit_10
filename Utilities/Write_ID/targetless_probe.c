/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
// Board macro discovery to augment TCData.ini:
// Run on attached target and copy the macros to TCData.ini.

debug
void main(void)
{
#ifdef _DC_MD0_
 #if _DC_MD0_
	printf(";MD0=0x%lX", (unsigned long) _DC_MD0_);
	printf(";MD0_ID=0x%lX", (unsigned long) _DC_MD0_ID_);
	printf(";MD0_TYPE=0x%lX", (unsigned long) _DC_MD0_TYPE_);
	printf(";MD0_SIZE=0x%lX", (unsigned long) _DC_MD0_SIZE_);
	printf(";MD0_SECSIZE=0x%lX", (unsigned long) _DC_MD0_SECSIZE_);
	printf(";MD0_SECNUM=0x%lX", (unsigned long) _DC_MD0_SECNUM_);
	printf(";MD0_SPEED=0x%lX", (unsigned long) _DC_MD0_SPEED_);
	printf(";MD0_MBC=0x%lX", (unsigned long) _DC_MD0_MBC_);
 #endif
#endif

#ifdef _DC_MD1_
 #if _DC_MD1_
	printf(";MD1=0x%lX", (unsigned long) _DC_MD1_);
	printf(";MD1_ID=0x%lX", (unsigned long) _DC_MD1_ID_);
	printf(";MD1_TYPE=0x%lX", (unsigned long) _DC_MD1_TYPE_);
	printf(";MD1_SIZE=0x%lX", (unsigned long) _DC_MD1_SIZE_);
	printf(";MD1_SECSIZE=0x%lX", (unsigned long) _DC_MD1_SECSIZE_);
	printf(";MD1_SECNUM=0x%lX", (unsigned long) _DC_MD1_SECNUM_);
	printf(";MD1_SPEED=0x%lX", (unsigned long) _DC_MD1_SPEED_);
	printf(";MD1_MBC=0x%lX", (unsigned long) _DC_MD1_MBC_);
 #endif
#endif

#ifdef _DC_MD2_
 #if _DC_MD2_
	printf(";MD2=0x%lX", (unsigned long) _DC_MD2_);
	printf(";MD2_ID=0x%lX", (unsigned long) _DC_MD2_ID_);
	printf(";MD2_TYPE=0x%lX", (unsigned long) _DC_MD2_TYPE_);
	printf(";MD2_SIZE=0x%lX", (unsigned long) _DC_MD2_SIZE_);
	printf(";MD2_SECSIZE=0x%lX", (unsigned long) _DC_MD2_SECSIZE_);
	printf(";MD2_SECNUM=0x%lX", (unsigned long) _DC_MD2_SECNUM_);
	printf(";MD2_SPEED=0x%lX", (unsigned long) _DC_MD2_SPEED_);
	printf(";MD2_MBC=0x%lX", (unsigned long) _DC_MD2_MBC_);
 #endif
#endif

#ifdef _DC_MD3_
 #if _DC_MD3_
	printf(";MD3=0x%lX", (unsigned long) _DC_MD3_);
	printf(";MD3_ID=0x%lX", (unsigned long) _DC_MD3_ID_);
	printf(";MD3_TYPE=0x%lX", (unsigned long) _DC_MD3_TYPE_);
	printf(";MD3_SIZE=0x%lX", (unsigned long) _DC_MD3_SIZE_);
	printf(";MD3_SECSIZE=0x%lX", (unsigned long) _DC_MD3_SECSIZE_);
	printf(";MD3_SECNUM=0x%lX", (unsigned long) _DC_MD3_SECNUM_);
	printf(";MD3_SPEED=0x%lX", (unsigned long) _DC_MD3_SPEED_);
	printf(";MD3_MBC=0x%lX", (unsigned long) _DC_MD3_MBC_);
 #endif
#endif

#ifdef _DC_MD4_
 #if _DC_MD4_
	printf(";MD4=0x%lX", (unsigned long) _DC_MD4_);
	printf(";MD4_ID=0x%lX", (unsigned long) _DC_MD4_ID_);
	printf(";MD4_TYPE=0x%lX", (unsigned long) _DC_MD4_TYPE_);
	printf(";MD4_SIZE=0x%lX", (unsigned long) _DC_MD4_SIZE_);
	printf(";MD4_SECSIZE=0x%lX", (unsigned long) _DC_MD4_SECSIZE_);
	printf(";MD4_SECNUM=0x%lX", (unsigned long) _DC_MD4_SECNUM_);
	printf(";MD4_SPEED=0x%lX", (unsigned long) _DC_MD4_SPEED_);
	printf(";MD4_MBC=0x%lX", (unsigned long) _DC_MD4_MBC_);
 #endif
#endif

#ifdef _DC_MD5_
 #if _DC_MD5_
	printf(";MD5=0x%lX", (unsigned long) _DC_MD5_);
	printf(";MD5_ID=0x%lX", (unsigned long) _DC_MD5_ID_);
	printf(";MD5_TYPE=0x%lX", (unsigned long) _DC_MD5_TYPE_);
	printf(";MD5_SIZE=0x%lX", (unsigned long) _DC_MD5_SIZE_);
	printf(";MD5_SECSIZE=0x%lX", (unsigned long) _DC_MD5_SECSIZE_);
	printf(";MD5_SECNUM=0x%lX", (unsigned long) _DC_MD5_SECNUM_);
	printf(";MD5_SPEED=0x%lX", (unsigned long) _DC_MD5_SPEED_);
	printf(";MD5_MBC=0x%lX", (unsigned long) _DC_MD5_MBC_);
 #endif
#endif

   // Add more memory device macros here, if ever appropriate.
}