/* z0194a.h Sharp z0194a tuner support
*
* Copyright (C) 2008 Igor M. Liplianin (liplianin@me.by)
*
*	This program is free software; you can redistribute it and/or modify it
*	under the terms of the GNU General Public License as published by the
*	Free Software Foundation, version 2.
*
* see Documentation/dvb/README.dvb-usb for more information
*/

#ifndef Z0194A
#define Z0194A

static int sharp_z0194a_set_symbol_rate(struct dvb_frontend *fe,
					 u32 srate, u32 ratio)
{
	u8 aclk = 0;
	u8 bclk = 0;

	if (srate < 1500000) {
		aclk = 0xb7; bclk = 0x47; }
	else if (srate < 3000000) {
		aclk = 0xb7; bclk = 0x4b; }
	else if (srate < 7000000) {
		aclk = 0xb7; bclk = 0x4f; }
	else if (srate < 14000000) {
		aclk = 0xb7; bclk = 0x53; }
	else if (srate < 30000000) {
		aclk = 0xb6; bclk = 0x53; }
	else if (srate < 45000000) {
		aclk = 0xb4; bclk = 0x51; }

	stv0299_writereg(fe, 0x13, aclk);
	stv0299_writereg(fe, 0x14, bclk);
	stv0299_writereg(fe, 0x1f, (ratio >> 16) & 0xff);
	stv0299_writereg(fe, 0x20, (ratio >> 8) & 0xff);
	stv0299_writereg(fe, 0x21, (ratio) & 0xf0);

	return 0;
}

static u8 sharp_z0194a_inittab[] = {
	0x01, 0x15,
	0x02, 0x30,
	0x03, 0x00,
	0x04, 0x7d,   /*                                                 */
	0x05, 0x35,   /*                              */
	0x06, 0x40,   /*                                           */
	0x07, 0x00,   /*         */
	0x08, 0x40,   /*                                          */
	0x09, 0x00,   /*      */
	0x0c, 0x51,   /*                                              */
	0x0d, 0x82,   /*                                            */
	0x0e, 0x23,   /*                             */
	0x10, 0x3f,   /*            */
	0x11, 0x84,
	0x12, 0xb9,
	0x15, 0xc9,   /*                         */
	0x16, 0x00,
	0x17, 0x00,
	0x18, 0x00,
	0x19, 0x00,
	0x1a, 0x00,
	0x1f, 0x50,
	0x20, 0x00,
	0x21, 0x00,
	0x22, 0x00,
	0x23, 0x00,
	0x28, 0x00,  /*                                                */
	0x29, 0x1e,  /*               */
	0x2a, 0x14,  /*               */
	0x2b, 0x0f,  /*               */
	0x2c, 0x09,  /*               */
	0x2d, 0x05,  /*               */
	0x2e, 0x01,
	0x31, 0x1f,  /*               */
	0x32, 0x19,  /*                            */
	0x33, 0xfc,  /*            */
	0x34, 0x93,  /*               */
	0x0f, 0x52,
	0xff, 0xff
};

#endif
