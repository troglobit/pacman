CC =	cc
DFLAGS =
CFLAGS =	-W -Wall -Wextra -g -O2
LDFLAGS =
CFILES =	pacman.c monster.c util.c movie.c
OFILES =	pacman.o monster.o util.o movie.o

pacman:	$(OFILES)
	$(CC) $(LDFLAGS) -o pacman $(OFILES) -lcurses

pacman.o:	pacman.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) pacman.c

monster.o:	monster.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) monster.c

util.o:	util.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) util.c

movie.o:	movie.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) movie.c


install:	pacman
	cp pacman /usr/games/pacman

strip:	pacman
	strip pacman

shrink:
	-rm -f *.o

clean:	shrink
	-rm -f pacman errs core a.out

lint:	$(CFILES)
	lint -pc $(CFILES)

list:	$(CFILES) pacdefs.h makefile
	oprl -x makefile $(CFILES) pacdefs.h
	oprl -x -C $(CFILES)
	touch list
