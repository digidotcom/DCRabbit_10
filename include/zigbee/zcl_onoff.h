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
	@addtogroup zcl_onoff
	@{
	@file zcl_onoff.h

	On/Off Cluster: Attributes and commands for switching devices between
	'On' and 'Off' states.

	ZCL General Specification, Section 3.8

	On/Off Switch Configuration Cluster: Attributes and commands for
	configuring On/Off switching devices.

	ZCL General Specification, Section 3.9

	@todo Write code to support OnOff commands.  Create sample program
			demonstrating use of these clusters (perhaps using LEDs and
			switches of XBIB-U-DEV board).  REQUIRES: need to have reporting
			in place before this cluster is really useful.  Devices need to
			receive updates whenever switch status changes.
*/

/// OnOff attribute, Boolean, Read only, Mandatory; Shall be reported
#define ZCL_ONOFF_ATTR_ONOFF				0x0000
#define ZCL_ONOFF_ON							1
#define ZCL_ONOFF_OFF						0

// Commands Received

/// Off command, Mandatory
#define ZCL_ONOFF_CMD_OFF					0x00

/// On command, Mandatory
#define ZCL_ONOFF_CMD_ON					0x01

/// Toggle command, Mandatory
#define ZCL_ONOFF_CMD_TOGGLE				0x02



/// SwitchType, 8-bit Enumeration, Read only, Mandatory
#define ZCL_SWITCH_ATTR_SWITCH_TYPE		0x0000

// values for SwitchType
#define ZCL_SWITCH_TYPE_TOGGLE			0x00
#define ZCL_SWITCH_TYPE_MOMENTARY		0x01

/// SwitchActions, 8-bit Enumeration, Read/Write, Mandatory
#define ZCL_SWITCH_ATTR_SWITCH_ACTIONS	0x0010

// values for SwitchActions
#define ZCL_SWITCH_ACTION_ONOFF			0x00			// default
#define ZCL_SWITCH_ACTION_OFFON			0x01
#define ZCL_SWITCH_ACTION_TOGGLE			0x02
