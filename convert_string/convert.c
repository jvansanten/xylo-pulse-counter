/*
 * convert_string/convert.c -- FX2 USB data converter (filter) example. 
 * 
 * Copyright (c) 2006--2008 by Wolfgang Wieser ] wwieser (a) gmx <*> de [ 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define ALLOCATE_EXTERN
#include <fx2regs.h>

// Read TRM p.15-115 for an explanation on this. 
// A single nop is sufficient for default setup but like that we're on 
// the safe side. 
#define	NOP		_asm nop _endasm
#define	SYNCDELAY	NOP; NOP; NOP; NOP


static void Initialize(void)
{
	CPUCS=0x10;   // 48 MHz, CLKOUT output disabled. 
	
	IFCONFIG=0xc0;  // Internal IFCLK, 48MHz; A,B as normal ports. 
	SYNCDELAY;
	
	REVCTL=0x03;  // See TRM...
	SYNCDELAY;
	
	EP6CFG=0xe2;  // 1110 0010 (bulk IN, 512 bytes, double-buffered)
	SYNCDELAY;
	
	EP2CFG=0xa2;  // 1010 0010 (bulk OUT, 512 bytes, double-buffered)
	SYNCDELAY;
	
	FIFORESET = 0x80;  SYNCDELAY;  // NAK all requests from host. 
	FIFORESET = 0x82;  SYNCDELAY;  // Reset individual EP (2,4,6,8)
	FIFORESET = 0x84;  SYNCDELAY;
	FIFORESET = 0x86;  SYNCDELAY;
	FIFORESET = 0x88;  SYNCDELAY;
	FIFORESET = 0x00;  SYNCDELAY;  // Resume normal operation. 
	
	EP2FIFOCFG = 0x00;  // Make sure AUTOOUT=0. 
	SYNCDELAY;
	
	// Be sure to clear the 2 buffers (double-buffered) (required!). 
	OUTPKTEND = 0x82;  SYNCDELAY;
	OUTPKTEND = 0x82;  SYNCDELAY;
}


// This will read the EP2 data and put it into EP6 for transmission. 
static void ProcessSendData(void)
{
	xdata const unsigned char *src=EP2FIFOBUF;
	xdata unsigned char *dest=EP6FIFOBUF;
	unsigned int len = ((int)EP2BCH)<<8 | EP2BCL;
	unsigned int i;
	for(i=0; i<len; i++,src++,dest++)
	{
		if(*src>='a' && *src<='z')
		{  *dest=*src-'a'+'A';  }
		else
		{  *dest=*src;  }
	}
	
	// "Skip" the received OUT packet to "forget" it (see TRM p. 9-26): 
	SYNCDELAY;  OUTPKTEND=0x82;
	// Note: Setting EP2BCL would source a packet into the GPIF side. 
	
	// Arm the endpoint. Be sure to set BCH before BCL because BCL access 
	// actually arms the endpoint. 
	SYNCDELAY;  EP6BCH=len>>8;
	SYNCDELAY;  EP6BCL=len&0xff;
}


void main(void)
{
	Initialize();
	
	for(;;)
	{
		// Wait for input on EP2 (EP2 non-empty). 
		if(!(EP2CS & (1<<2)))
		{
			// Wait for EP6 buffer to become non-full so that we don't 
			// overwrite content. 
			while(EP6CS & (1<<3));
			ProcessSendData();
		}
	}
}
