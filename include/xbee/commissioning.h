/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
