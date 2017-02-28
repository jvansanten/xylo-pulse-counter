INCLUDES = -I../include
CC = sdcc -mmcs51
BASENAME = convert
CYCFX2PROG = cycfx2prog

$(BASENAME).ihx: $(BASENAME).c
	$(CC) $(INCLUDES) $(BASENAME).c

clean:
	rm -f $(BASENAME).{asm,ihx,lnk,lst,map,mem,rel,rst,sym}

.PHONY:run
run: $(BASENAME).ihx
	$(CYCFX2PROG) prg:$(BASENAME).ihx run delay:10 sbulk:2,Hello_World dbulk:6,-512
	$(CYCFX2PROG) sbulk:2,"This is another test :)" dbulk:6,-512
	$(CYCFX2PROG) sbulk:2,"String one first" sbulk:2,"String two afterwards" dbulk:6,-512 dbulk:6,-512
