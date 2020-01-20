

lab7.bin:	lab7final.s
		nasm lab7final.s -o lab7.bin -l lab7.lst

lab7final.s:	clib.s yaks.s yakc.s isr.s myinth.s lab7app.s
		cat clib.s lab7app.s yaks.s yakc.s isr.s myinth.s > lab7final.s

yakc.s:	yakc.c
		cpp yakc.c yakc.i
		c86 -g yakc.i yakc.s

myinth.s:	myinth.c
		cpp myinth.c myinth.i
		c86 -g myinth.i myinth.s

lab7app.s:	lab7app.c
		cpp lab7app.c lab7app.i
		c86 -g lab7app.i lab7app.s

clean:
		rm lab7.bin lab7.lst lab7final.s myinth.s myinth.i \
		yakc.i yakc.s lab7app.i lab7app.s
