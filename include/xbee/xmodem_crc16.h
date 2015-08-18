/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/**
	@addtogroup util_xmodem
	@{
	@file xmodem_crc16.h
	Header for crc16_calc() function implemented in xmodem_crc16.c.
*/
#include "xbee/platform.h"

uint16_t crc16_calc( const void FAR *data, uint16_t length, uint16_t current);
