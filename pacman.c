/*
/* PACMAN  - written by Dave Nixon, AGS Computers Inc., July, 1981.
/* Converted for curses Feb 1982 by Mark Horton.
/*
/* Terminal handling for video games taken from aliens
/*      the original version  of aliens is from 
/*      Fall 1979                      Cambridge               Jude Miller
/*
/* Score keeping modified and general terminal handling (termcap routines
/* from UCB's ex) added by Rob Coben, BTL, June, 1980.
/*
/* If MSG is defined, the program uses the inter-process message facility
/* of UNIX/TS Augmented to optimize reading the tty while updating the
/* screen. Otherwise, the child process (reading the tty) writes
/* a temporary communications file, which the parent (updating the screen)
/* is constantly seeking to the beginning of and re-reading. (UGH!)
/* If your system has a non-blocking read (read-without-wait), it should
/* be implemented, and the child process could be dispensed with entirely.
*/
#include <stdio.h>
#include "pacdefs.h"
 
/*
 * global variables
 */

extern char
	message[];

extern char
	initbrd[BRDY][BRDX],
	display[BRDY][BRDX];

extern unsigned
	pscore;

extern struct pac
	monst[];

extern char monst_names[];
#ifdef A_BLINK
# define pflash A_BLINK
#else
# define pflash _STANDOUT
#endif

int	pacsymb = PACMAN,
	rounds,		/* time keeping mechanism */
	killflg,
	delay,
	potion,
	goldcnt,		/* no. of gold pieces remaining */
	game,
	monst_often,
	monsthere,
	boardcount = 1,
	wmonst,
	potintvl = POTINTVL,
	potioncnt,
	treascnt = 0;
extern
	char *full_names[];

struct pac
	pac;

struct pac
	pacstart =
{
	PSTARTX,
	PSTARTY,
	DNULL,
	SLOW,
	FALSE
};

struct pac
	*pacptr = &pac;

#define DEFCHPERTURN 60

main(argc, argv)
char **argv;
{
	register int tmp;		/* temp variables */
	register int pac_cnt;
	register int monstcnt;	/* monster number */
	int tries;
	long denom;
	struct pac *mptr;
	char gcnt[10];
	char msgbuf[50];
	int chperturn = DEFCHPERTURN;

	game = 0;
	if (argc > 1) {
		game = argv[1][0] - '0';
		if (game < 1 || game > 3)
			game = 0;
		if (argc > 2)
			chperturn = atoi(argv[2]);
	}
	init();		/* global init */
	for (pac_cnt = MAXPAC; pac_cnt > 0; pac_cnt--)
	{
redraw:
		erase();
		potioncnt = 0;
		treascnt = 0;
		potion = FALSE;
		SPLOT(0, 45, "SCORE: ");
		(void) sprintf(msgbuf, "GAME: %s",	game==1 ? "EASY" :
							game==2 ? "MEDIUM" :
								  "HARD");
		SPLOT(0, 65, msgbuf);
		SPLOT(21, 45, "gold left = ");
		(void) sprintf(gcnt, "%6d", goldcnt);
		SPLOT(21, 57, gcnt);

		/*
		 * We update the monsters every monst_often turns, to keep
		 * the CRT caught up with the computer.  The fudge factor
		 * was calculated from the assumption that each full refresh
		 * outputs chperturn characters.  The default is pessimistic
		 * based on ANSI and HP terminals w/verbose cursor addressing.
		 */
		denom = ((long) delay) * baudrate();
		monst_often = (chperturn * 10000L + denom - 1) / denom;
		if (monst_often < 1)
			monst_often = 1;

		if (potion == TRUE)
		{
			SPLOT(3, 45, "COUNTDOWN: ");
			(void) sprintf(message, "%2d", potioncnt);
			SPLOT(3, 60, message);
		};
		pacsymb = PACMAN;
		killflg = FALSE;
		(void) sprintf(message,
			"delay = %3d, refresh = %3d", delay, monst_often);
		SPLOT(22, 45, message);
		/*
		 * PLOT maze
		 */
		for (tmp = 0; tmp < BRDY; tmp++)
		{
			SPLOT(tmp, 0, &(display[tmp][0]));
		};
		/* initialize a pacman */
		pac = pacstart;
		PLOT(pacptr->ypos, pacptr->xpos, pacsymb | pflash);
		/* display remaining pacmen */
		for (tmp = 0; tmp < pac_cnt - 1; tmp++)
		{
			PLOT(LINES >=24 ? 23 : 22, (MAXPAC * tmp), PACMAN);
		};
		/*
		 * Init. monsters
	 	 */
		for (mptr = &monst[0], monstcnt = 0; monstcnt < MAXMONSTER; mptr++, monstcnt++)
		{
			mptr->xpos = MSTARTX + (2 * monstcnt);
			mptr->ypos = MSTARTY;
			mptr->speed = SLOW;
			mptr->dirn = DNULL;
			mptr->danger = TRUE;
			mptr->stat = START;
			PLOT(mptr->ypos, mptr->xpos, monst_names[monstcnt]);
			mptr->xdpos = mptr->xpos;
			mptr->ydpos = mptr->ypos;
		};
		rounds = 0;	/* timing mechanism */
		update();
		refresh();
		tries = 0;
		while ((pacptr->dirn == NULL) && (tries++ < 300))
		{
			napms(100);
			poll(1);
		}

		/* main game loop */
		do
		{
			if (rounds++ % MSTARTINTVL == 0)
			{
				startmonst();
			};
			pacman();
			if (killflg == TURKEY)
				break;
			for (monstcnt = 0; monstcnt < (MAXMONSTER / 2); monstcnt++)
			{
				monster(monstcnt);	/* next monster */
			};
			if (killflg == TURKEY)
				break;
			if (pacptr->speed == FAST)
			{
				pacman();
				if (killflg == TURKEY)
					break;
			};
			for (monstcnt = (MAXMONSTER / 2); monstcnt < MAXMONSTER; monstcnt++)
			{
				monster(monstcnt);	/* next monster */
			};
			if (killflg == TURKEY)
				break;
			if (potion == TRUE)
			{
				(void) sprintf(message, "%2d", potioncnt);
				SPLOT(3, 60, message);
				if (potioncnt == 10 || potioncnt < 5)
					beep();
				if (--potioncnt <= 0)
				{
					SPLOT(3,45,"                        ");
					SPLOT(4,45,"                        ");
					SPLOT(5,45,"                        ");
					potion = FALSE;
					pacptr->speed = SLOW;
					pacptr->danger = FALSE;
					for (monstcnt = 0; monstcnt < MAXMONSTER; monstcnt++)
					{
						monst[monstcnt].danger = TRUE;
					}
				}
			}
			if (treascnt && --treascnt == 0) {
				display[TRYPOS][TRXPOS] = VACANT;
				PLOT(TRYPOS, TRXPOS, VACANT);
			}
			if (rounds % monst_often == 0)
				update();	/* score display etc */
			refresh();
			if (goldcnt <= 0)
			{
				potintvl -= 5;
				if (potintvl <= 0)
					potintvl = 5;
				reinit();
				goto redraw;
			};
		} while (killflg != TURKEY);
		sprintf(msgbuf, "Oops!  %s got you!\n", full_names[wmonst]);
		SPLOT(5, 45, msgbuf);
		flushinp();
		refresh();
		sleep(2);
	}
	SPLOT(8, 45, "THE MONSTERS ALWAYS TRIUMPH");
	SPLOT(9, 45, "IN THE END!");
	update();
	over();
}

pacman()
{
	register int sqtype;
	register int mcnt;
	register int tmpx, tmpy;
	int deltat;
	struct pac *mptr;

	refresh();

	/* pause; wait for the player to hit a key */
	napms(delay);

	/* get instructions from player, but don't wait */
	poll(0);

	/* remember current pacman position */
	tmpx = pacptr->xpos;
	tmpy = pacptr->ypos;

	/* "eat" any gold */
	/* update display array to reflect what is on terminal */
	display[tmpy][tmpx] = VACANT;

	/* what next? */
	switch (pacptr->dirn)
	{
	case DUP:
		pacsymb = (rounds%2) ? CUP : PUP;
		switch (sqtype = display[tmpy + UPINT][tmpx])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->ypos += UPINT;
			break;

		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DDOWN:
		pacsymb = (rounds%2) ? CDOWN : PDOWN;
		switch (sqtype = display[tmpy + DOWNINT][tmpx])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->ypos += DOWNINT;
			break;

		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DLEFT:
		if(tmpx == 0)
		{
			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos = XWRAP;
			sqtype = VACANT;
			break;
		};
		pacsymb = (rounds%2) ? CLEFT : PLEFT;
		switch (sqtype = display[tmpy][tmpx + LEFTINT])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos += LEFTINT;
			break;
		
		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DRIGHT:
		if(tmpx == XWRAP)
		{
			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos = 0;
			sqtype = VACANT;
			break;
		};
		pacsymb = (rounds%2) ? CRIGHT : PRIGHT;
		switch (sqtype = display[tmpy][tmpx + RIGHTINT])
		{
		case GOLD:
		case VACANT:
		case CHOICE:
		case POTION:
		case TREASURE:

			/* erase where the pacman went */
			PLOT(tmpy, tmpx, VACANT);
			pacptr->xpos += RIGHTINT;
			break;

		default:
			pacptr->dirn = DNULL;
			pacsymb = PACMAN;
			break;
		};
		break;
	case DNULL:
		pacsymb = PACMAN;
		break;
	}

	/* did the pacman get any points or eat a potion? */
	switch (sqtype)
	{
	case CHOICE:
	case GOLD:
		pscore += GOLDVAL;
		goldcnt--;
		break;

	case TREASURE:
		switch (boardcount) {
			case 0:
			case 1:          pscore +=  100; break;
			case 2:          pscore +=  200; break;
			case 3: case  4: pscore +=  500; break;
			case 5: case  6: pscore +=  700; break;
			case 7: case  8: pscore += 1000; break;
			default:
			case 9: case 10: pscore += 2000; break;
		}
		break;

	case POTION:
		SPLOT(3, 45, "COUNTDOWN: ");
		potion = TRUE;
		potioncnt = potintvl;
		monsthere = 0;
		pacptr->speed = FAST;
		pacptr->danger = TRUE;

		/* slow down monsters and make them harmless */
		mptr = &monst[0];
		for (mcnt = 0; mcnt < MAXMONSTER; mcnt++)
		{
			mptr->speed = SLOW;
			mptr->danger = FALSE;
			mptr++;
		}
		break;
	}

	/* did the pacman run into a monster? */
	killflg = FALSE;
	for (mptr = &monst[0], mcnt = 0; mcnt < MAXMONSTER; mptr++, mcnt++)
	{
		if ((mptr->xpos==pacptr->xpos) && (mptr->ypos==pacptr->ypos))
			killflg = dokill(mcnt);
	};
	if (killflg != TURKEY)
	{
		PLOT(pacptr->ypos, pacptr->xpos, pacsymb | pflash);
	};
}
