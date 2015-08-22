/*
	Copyright (c)2010 Digi International Inc., All Rights Reserved

	This software contains proprietary and confidential information of Digi
	International Inc.  By accepting transfer of this copy, Recipient agrees
	to retain this software in confidence, to prevent disclosure to others,
	and to make no use of this software other than that for which it was
	delivered.  This is a published copyrighted work of Digi International
	Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
	prohibited.

	Restricted Rights Legend

	Use, duplication, or disclosure by the Government is subject to
	restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
	Technical Data and Computer Software clause at DFARS 252.227-7031 or
	subparagraphs (c)(1) and (2) of the Commercial Computer Software -
	Restricted Rights at 48 CFR 52.227-19, as applicable.

	Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
*/
/**
	@addtogroup zcl_time
	@{
	@file zigbee/zcl_time.h
	Header for implementation of ZigBee Time Cluster
	(ZCL Spec section 3.12).

	Numbers in comments refer to sections of ZigBee Cluster Library
	specification (Document 075123r02ZB).
*/

#ifndef __XBEE_ZCL_TIME_H
#define __XBEE_ZCL_TIME_H

#include <time.h>
#include "zigbee/zcl.h"

extern const zcl_attribute_tree_t zcl_time_attribute_tree[];

/** @name
	Attributes of ZCL Time Cluster
	@{
*/
#define ZCL_TIME_ATTR_TIME					0x0000
										///< 3.12.2.2.1 Time Attribute
#define ZCL_TIME_ATTR_TIME_STATUS		0x0001
										///< 3.12.2.2.2 TimeStatus Attribute
#define ZCL_TIME_ATTR_TIME_ZONE			0x0002
										///< 3.12.2.2.3 TimeZone Attribute
#define ZCL_TIME_ATTR_DST_START			0x0003
										///< 3.12.2.2.4 DstStart Attribute
#define ZCL_TIME_ATTR_DST_END				0x0004
										///< 3.12.2.2.5 DstEnd Attribute
#define ZCL_TIME_ATTR_DST_SHIFT			0x0005
										///< 3.12.2.2.6 DstShift Attribute
#define ZCL_TIME_ATTR_STANDARD_TIME		0x0006
										///< 3.12.2.2.7 StandardTime Attribute
#define ZCL_TIME_ATTR_LOCAL_TIME			0x0007
										///< 3.12.2.2.8 LocalTime Attribute
//@}

/** @name
	Bitfields for attribute #ZCL_TIME_ATTR_TIME, TimeStatus.

	Bits 3-7 are reserved.

	Per Section 3.12.2.2.2 of the ZCL spec:
	-	If the Master bit is 1, the value of [the Synchronized bit] is 0.

	-	If both the Master and Synchronized bits are 0, the real time clock
		has no defined relationship to the time standard.
	@{
*/
/// The RTC corresponding to the Time attribute is internally set to the
/// time standard.  This bit should only be set if the host has reliably
/// set its RTC.
#define ZCL_TIME_STATUS_MASTER			0x01

/// The Synchronized bit specifies whether Time has been set over the
/// [ZigBee] network to synchronize it (as close as may be practical)
/// to the time standard (see 3.12.1).
#define ZCL_TIME_STATUS_SYNCHRONIZED	0x02

/// The MasterZoneDst bit specifies whether the TimeZone, DstStart, DstEnd
/// and DstShift attributes are set internally to correct values for the
/// location of the clock.
#define ZCL_TIME_STATUS_MASTERZONEDST	0x04
//@}

// what time is it?
zcl_utctime_t zcl_time_now( void);

// Convert a ZCL timestamp into a struct tm which can be used with
// the Standard C Library's <time.h>.
struct tm *zcl_gmtime( struct tm *tm, zcl_utctime_t timestamp);

// Convert a struct tm to a ZCL timestamp.
zcl_utctime_t zcl_mktime( struct tm *time_rec);

// processes Read Attribute Responses
int zcl_time_client( const wpan_envelope_t FAR *envelope,
	void FAR *context);

// initiate discovery of time servers
int zcl_time_find_servers( wpan_dev_t *dev, uint16_t profile_id);

/// Macro for inserting a standard Time Server (and Client) into an endpoint's
/// cluster list.
#define ZCL_CLUST_ENTRY_TIME_BOTH		\
		{ ZCL_CLUST_TIME,						\
			&zcl_time_client,					\
			zcl_time_attribute_tree,		\
			WPAN_CLUST_FLAG_INOUT | WPAN_CLUST_FLAG_ENCRYPT }

#define ZCL_CLUST_ENTRY_TIME_SERVER		\
		{ ZCL_CLUST_TIME,						\
			&zcl_general_command,									\
			zcl_time_attribute_tree,		\
			WPAN_CLUST_FLAG_SERVER | WPAN_CLUST_FLAG_ENCRYPT }

/// Macro for inserting a standard Time Client (no Server) into an endpoint's
/// cluster list.
#define ZCL_CLUST_ENTRY_TIME_CLIENT		\
		{ ZCL_CLUST_TIME,						\
			&zcl_time_client,					\
			zcl_attributes_none,				\
			WPAN_CLUST_FLAG_CLIENT | WPAN_CLUST_FLAG_ENCRYPT }

#ifdef __DC__
	#use "zcl_time.c"
#endif

#endif	// __XBEE_ZCL_TIME_H
