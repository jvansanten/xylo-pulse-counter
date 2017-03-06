/* -*- mode: C; c-basic-offset: 8; -*-
 *
 * xylo_setup -- minimal FX2 firmware to manage a KNJN Xylo-EM
 *
 * Copyright (c) 2017 by Jakob van Santen <jakob.van.santen@desy.de>
 *
 * based on saxo_loader.c
 * Copyright (c) 2009 by Brent Baccala <cosine@freesoft.org>
 *
 * based on convert_string/convert.c
 * Copyright (c) 2006--2008 by Wolfgang Wieser ] wwieser (a) gmx <*> de [ 
 *
 * Cross-compile with sdcc; the fx2regs.h include file is from
 * Wolfgang Wieser's usb-fx2-local-examples; currently available here:
 *
 * http://www.triplespark.net/elec/periph/USB-FX2/software/local_examples.html
 *
 * Boot loading the compiled program is done with Wolfgang's
 * cycfx2prog; available here:
 *
 * http://www.triplespark.net/elec/periph/USB-FX2/software/
 * 
 * The knjn.com Xylo-EM is based around two chips: the FX2, which is an
 * 8051-based USB-2 device and an Altera Cyclone FPGA.  This program
 * is intended to be cross-compiled for an 8051 architecture using
 * 'sdcc', then boot loaded over USB (by cycfx2prog) to the Saxo.
 * Upon initialization, it sets up the FX2 to handle USB control requests,
 * and puts endpoints 2, 4, 6, and 8 into slave mode so that the FPGA can
 * write to them directly. Unlike the original saxo_loader, this firmware can't
 * be used to program the FPGA directly, as the FX2 on the Xylo-EM board is
 * wired to the FPGA's JTAG pins rather than the passive serial config pins.
 * In particular,
 *
 * TDI = FX2 PD2
 * TDO = FX2 PD1
 * TMS = FX2 PD4
 * TCK = FX2 PD3
 *
 * Key documentation to understand this code is Cypress's FX2
 * Technical Reference Manual (TRM) and Altera's Cyclone Device
 * Handbook, available (as of this writing) at these URLs:
 *
 *    http://www.keil.com/dd/docs/datashts/cypress/fx2_trm.pdf
 *    http://www.altera.com/literature/hb/cyc/cyc_c5v1.pdf
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
#include <limits.h>

// Read TRM p.15-115 for an explanation on this. 
// A single nop is sufficient for default setup but like that we're on 
// the safe side. 
#define	NOP		__asm nop __endasm
#define	SYNCDELAY	NOP; NOP; NOP; NOP

static void ClearFIFOs(void);

static void Initialize(void)
{
	// See TRM 15.10.1
	// disconnect from host
	USBCS &= (1<<3);
	// Set RENUM bit to let default USB device handle control requests
	USBCS &= (1<<1);
	
	CPUCS=0x12;   // 48 MHz, CLKOUT output enabled. 
	SYNCDELAY;

	IFCONFIG=0xe3;  // Internal 48MHz IFCLK; IFCLK pin output enabled
			// slave FIFO in synchronous mode
	SYNCDELAY;
	
	REVCTL=0x03;  // See TRM...
	SYNCDELAY;

	PINFLAGSAB = 0x98;  // FLAGA = EP2 EF (empty flag); FLAGB = EP4 EF
	SYNCDELAY;
	PINFLAGSCD = 0xfe;  // FLAGC = EP6 FF (full flag); FLAGD = EP8 FF
	SYNCDELAY;

	// EP1INCFG = 0xa0; // Could use EP1 to send bulk debug info
	EP2CFG=0xa2;  // 1010 0010 (bulk OUT, 512 bytes, double-buffered)
	EP4CFG=0xa0;  // 1010 0000 (bulk OUT, 512 bytes, double-buffered)
	EP6CFG=0xe2;  // 1110 0010 (bulk IN, 512 bytes, double-buffered)
	EP8CFG=0xe0;  // 1110 0010 (bulk IN, 512 bytes, double-buffered)
	SYNCDELAY;
	
	ClearFIFOs();
	
	// Disable all interrupts except USB resume
	IE = 0;
	// Disable USB resume interrupt, too
	EICON &= ~0x20;
	
	// reconnect
	USBCS &= ~(1<<3);
}

static void ClearFIFOs(void)
{
	FIFORESET = 0x80;  SYNCDELAY;  // NAK all requests from host. 
	FIFORESET = 0x82;  SYNCDELAY;  // Reset individual EP (2,4,6,8)
	FIFORESET = 0x84;  SYNCDELAY;
	FIFORESET = 0x86;  SYNCDELAY;
	FIFORESET = 0x88;  SYNCDELAY;
	FIFORESET = 0x00;  SYNCDELAY;  // Resume normal operation. 

	// Be sure to clear the 2 buffers (double-buffered) (required!).
	EP2FIFOCFG = 0x00; SYNCDELAY;
	OUTPKTEND = 0x82;  SYNCDELAY;
	OUTPKTEND = 0x82;  SYNCDELAY;

	EP4FIFOCFG = 0x00; SYNCDELAY;
	OUTPKTEND = 0x84;  SYNCDELAY;
	OUTPKTEND = 0x84;  SYNCDELAY;
	
	EP6FIFOCFG = 0x00; SYNCDELAY;
	INPKTEND = 0x06;  SYNCDELAY;
	INPKTEND = 0x06;  SYNCDELAY;
	
	EP8FIFOCFG = 0x00; SYNCDELAY;
	INPKTEND = 0x08;  SYNCDELAY;
	INPKTEND = 0x08;  SYNCDELAY;

	EP2FIFOCFG = 0x10; SYNCDELAY; //  AUTOOUT=1; byte-wide operation
	EP4FIFOCFG = 0x10; SYNCDELAY; //  AUTOOUT=1; byte-wide operation
	EP6FIFOCFG = 0x0c; SYNCDELAY; //  AUTOIN=1; byte-wide operation
	EP8FIFOCFG = 0x0c; SYNCDELAY; //  AUTOIN=1; byte-wide operation
}

void main(void)
{
	Initialize();
	
	// In AUTO mode we have nothing to do; the FX2 handles USB
	// control requests and the endpoint FIFOs are slaved to the FPGA.
	// Nonetheless, we have to put a main loop here to keep the FX2
	// from deadlocking after a few reads.
	for (;;) {}
}
