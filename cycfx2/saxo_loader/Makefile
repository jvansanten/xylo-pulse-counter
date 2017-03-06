INCLUDES = -I../include
CC = sdcc -mmcs51
BASENAME = saxo_loader
CYCFX2PROG = cycfx2prog

%.ihx: %.c
	$(CC) $(INCLUDES) $<

tidy:
	rm -f $(BASENAME).{asm,lnk,lst,map,mem,rel,rst,sym,lk}

clean: tidy 
	rm -f $(BASENAME).ihx

.PHONY:run
run: $(BASENAME).ihx
	$(CYCFX2PROG) prg:$(BASENAME).ihx run
