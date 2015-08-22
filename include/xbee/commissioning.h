/*
	Copyright (c)2011 Digi International Inc., All Rights Reserved

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
	@addtogroup xbee_commissioning
	@{
	@file xbee/commissioning.h
	Header for XBee module support of ZCL Commissioning Cluster.
*/

#ifndef XBEE_COMMISSIONING_H
#define XBEE_COMMISSIONING_H

#include "xbee/device.h"
#include "zigbee/zcl_commissioning.h"

// XBee has hard-coded values, same as ZCL defaults except for ScanAttempts
// (which is set to 1 instead of 5).
#define XBEE_COMM_SCAN_ATTEMPTS						1
#define XBEE_COMM_TIME_BETWEEN_SCANS				0x0064
#define XBEE_COMM_REJOIN_INTERVAL					0x003C
#define XBEE_COMM_MAX_REJOIN_INTERVAL				0x0E10

void xbee_commissioning_tick( xbee_dev_t *xbee, zcl_comm_state_t *comm_state);
int xbee_commissioning_set( xbee_dev_t *xbee, zcl_comm_startup_param_t *p);

#endif	// XBEE_COMMISSIONING_H defined
