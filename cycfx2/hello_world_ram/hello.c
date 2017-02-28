/*
 * hello_world_ram/hello.c -- FX2 hello world in RAM example. 
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

void main(void)
{
	// We have 16kb of RAM (0x4000) and the program will only occupy the
	// first couple of bytes so the address 0x3000 is surely unused. 
	// To be compatible with the non-LP version (CY7C.. without A suffix) 
	// which has just 8kb RAM, use 0x1000 instead of 0x3000. 
	xdata char *dest=(xdata char*)0x1000;
	
	// Put content into that portion of RAM. This is the hello world 
	// string in Caesar chiffre (offset 1)...
	const char *src="Gdkkn+\x1fvnqkc \x1fSghr\x1fhr\x1fxntq\x1f"
	                "EW1\x1f""an`qc-";
	while(*src)
	{  *dest++ = *src++ + 1;  }
	
	// Loop endlessly...
	for(;;);
}
