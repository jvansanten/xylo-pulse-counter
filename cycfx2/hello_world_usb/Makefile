INCLUDES = -I../include
CC = sdcc -mmcs51
BASENAME = hello
CYCFX2PROG = cycfx2prog

$(BASENAME).ihx: $(BASENAME).c
	$(CC) $(INCLUDES) $(BASENAME).c

clean:
	rm -f $(BASENAME).{asm,ihx,lnk,lst,map,mem,rel,rst,sym}

.PHONY:run
run: $(BASENAME).ihx
	$(CYCFX2PROG) prg:$(BASENAME).ihx run delay:10 dbulk:6,-512,5
	sleep 1
	$(CYCFX2PROG) dbulk:6,-512
