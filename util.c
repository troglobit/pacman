#include "pacdefs.h"
#include <signal.h>
#include <pwd.h>

extern char
	*mktemp();

extern int
	delay,
	errno,
	wmonst,
	boardcount,
	rounds,
	monsthere,
	potintvl,
	treascnt,
	goldcnt;

extern long
	time();

extern struct pac
	*pacptr;

extern struct pac
	monst[];

extern char monst_names[];
extern char *full_names[];

/*
 * initbrd is used to re-initialize the display
 * array once a new game is started.
 */
char	initbrd[BRDY][BRDX] =
{
"#######################################",
"# . . . * . . . . ### . . . . * . . . #",
"# O ### . ##### . ### . ##### . ### O #",
"# * . . * . * . * . . * . * . * . . * #",
"# . ### . # . ########### . # . ### . #",
"# . . . * # . . . ### . . . # * . . . #",
"####### . ##### . ### . ##### . #######",
"      # . # . . * . . * . . # . #      ",
"      # . # . ### - - ### . # . #      ",
"####### . # . #         # . # . #######",
"        * . * #         # * . *        ",
"####### . # . #         # . # . #######",
"      # . # . ########### . # . #      ",
"      # . # * . . . . . . * # . #      ",
"####### . # . ########### . # . #######",
"# . . . * . * . . ### . . * . * . . . #",
"# O ### . ##### . ### . ##### . ### O #",
"# . . # * . * . * . . * . * . * # . . #",
"### . # . # . ########### . # . # . ###",
"# . * . . # . . . ### . . . # . . * . #",
"# . ########### . ### . ########### . #",
"# . . . . . . . * . . * . . . . . . . #",
"#######################################",
};

/*
 * brd is kept for historical reasons.
 * It should only be used in the routine "which"
 * to determine the next move for a monster or
 * in the routine "monster" to determine if it
 * was a valid move. Admittedly this is redundant
 * and could be replaced by initbrd, but it is kept
 * so that someday additional intelligence or
 * optimization could be added to the choice of
 * the monster's next move. Hence, note the symbol
 * CHOICE at most points that a move decision
 * logically HAS to be made.
 */
char	brd[BRDY][BRDX] =
{
"#######################################",
"# . . . * . . . . ### . . . . * . . . #",
"# O ### . ##### . ### . ##### . ### O #",
"# * . . * . * . * . . * . * . * . . * #",
"# . ### . # . ########### . # . ### . #",
"# . . . * # . . . ### . . . # * . . . #",
"####### . ##### . ### . ##### . #######",
"      # . # . . * . . * . . # . #      ",
"      # . # . ### - - ### . # . #      ",
"####### . # . #         # . # . #######",
"        * . * #         # * . *        ",
"####### . # . #         # . # . #######",
"      # . # . ########### . # . #      ",
"      # . # * . . . . . . * # . #      ",
"####### . # . ########### . # . #######",
"# . . . * . * . . ### . . * . * . . . #",
"# O ### . ##### . ### . ##### . ### O #",
"# . . # * . * . * . . * . * . * # . . #",
"### . # . # . ########### . # . # . ###",
"# . * . . # . . . ### . . . # . . * . #",
"# . ########### . ### . ########### . #",
"# . . . . . . . * . . * . . . . . . . #",
"#######################################",
};

/*
 * display reflects the screen on the player's
 * terminal at any point in time.
 */
char	display[BRDY][BRDX] =
{
"#######################################",
"# . . . . . . . . ### . . . . . . . . #",
"# O ### . ##### . ### . ##### . ### O #",
"# . . . . . . . . . . . . . . . . . . #",
"# . ### . # . ########### . # . ### . #",
"# . . . . # . . . ### . . . # . . . . #",
"####### . ##### . ### . ##### . #######",
"      # . # . . . . . . . . # . #      ",
"      # . # . ### - - ### . # . #      ",
"####### . # . #         # . # . #######",
"        . . . #         # . . .        ",
"####### . # . #         # . # . #######",
"      # . # . ########### . # . #      ",
"      # . # . . . . . . . . # . #      ",
"####### . # . ########### . # . #######",
"# . . . . . . . . ### . . . . . . . . #",
"# O ### . ##### . ### . ##### . ### O #",
"# . . # . . . . . . . . . . . . # . . #",
"### . # . # . ########### . # . # . ###",
"# . . . . # . . . ### . . . # . . . . #",
"# . ########### . ### . ########### . #",
"# . . . . . . . . . . . . . . . . . . #",
"#######################################",
};

char	combuf[BUFSIZ],
	message[81],	/* temporary message buffer */
	inbuf[2];

int	ppid,
	cpid,
	game,
	killcnt = 0,
	vs_rows,
	vs_cols;

unsigned
	pscore;

long	timein;

struct uscore
{
	unsigned score;	/* same type as pscore */
	int uid;	/* uid of player */
};

struct scorebrd
{
	struct uscore entry[MSSAVE];
} scoresave[MGTYPE] = 
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

update()
{
	char	str[10];

	sprintf(str, "%6d", pscore);
	SPLOT(0, 52, str);
	sprintf(str, "%6d", goldcnt);
	SPLOT(21, 57, str);
}

reinit()
{
	register int locx, locy;
	register char tmp;

	if (boardcount % 2 == 0)
		movie();
	for (locy = 0; locy < BRDY; locy++)
	{
		for (locx = 0; locx < BRDX; locx++)
		{
			tmp = initbrd[locy][locx];
			brd[locy][locx] = tmp;
			if ((display[locy][locx] = tmp) == CHOICE)
			{
				display[locy][locx] = GOLD;
			};
		};
	};
	goldcnt = GOLDCNT;
	delay = delay * 3 / 4;	/* hot it up */
	boardcount++;
}

errgen(string)
char	*string;
{
	SPLOT(23,45,string);
}

dokill(mnum)
	int mnum;
{
	register struct pac *mptr;
	char msgbuf[50];

	beep();
	if (monst[mnum].danger == FALSE)
	{
		if (++killcnt == MAXMONSTER)
		{
			if (display[TRYPOS][TRXPOS] == GOLD)
			{
				goldcnt--;
			};
			display[TRYPOS][TRXPOS] = TREASURE;
			PLOT(TRYPOS, TRXPOS, TREASURE);
			killcnt = 0;
			treascnt = potintvl;
		}
		SPLOT(5, 45, "MONSTERS KILLED: ");
		(void) sprintf(message, "%1d", killcnt);
		SPLOT(5, 62, message);
		mptr = (&monst[mnum]);
		mptr->ypos = MSTARTY;
		mptr->xpos = MSTARTX + (2 * mnum);
		mptr->danger = TRUE;
		mptr->stat = START;
		PLOT(mptr->ypos, mptr->xpos, monst_names[mnum]);
		monsthere++;
		rounds = 1;	/* force it to be a while before he comes out */
		switch (monsthere) {
		case 1: pscore +=     KILLSCORE; break;
		case 2: pscore += 2 * KILLSCORE; break;
		case 3: pscore += 4 * KILLSCORE; break;
		case 4: pscore += 8 * KILLSCORE; break;
		}
		sprintf(msgbuf, "You got %s!\n", full_names[mnum]);
		SPLOT(4, 45, msgbuf);
		return(GOTONE);
	};
	wmonst = mnum;
	return(TURKEY);
}

/*
/* clr -- issues an escape sequence to clear the display
*/

clr()
{
	clear();
}

printw(fmt, p1, p2, p3, p4)
char *fmt;
int p1, p2, p3, p4;
{
	static char buf[100];
	sprintf(buf, fmt, p1, p2, p3, p4);
	addstr(buf);
}

/*
 *	display initial instructions
 */

instruct()
{
	clr();
	POS(0, 0);
	printw("Attention: you are in a maze, being chased by monsters!\n\n");
	printw("There is food scattered uniformly in the maze, marked by \".\".\n");
	printw("One magic potion is available at each spot marked \"O\". Each potion will\n");
	printw("enable you to eat monsters for a limited duration. It will also\n");
	printw("scare them away. When you eat a monster it is regenerated, but this takes\n");
	printw("time. You can also regenerate yourself %d times. Eating all the monsters\n", MAXPAC);
	printw("results in further treasure appearing magically somewhere in the dungeon,\n");
	printw("marked by \"$\". There is a magic tunnel connecting the center left and\n");
	printw("center right parts of the dungeon. The monsters know about it!\n\n");
	printw("        Type:   h or s  to move left\n");
	printw("                l or f  to move right\n");
	printw("                k or e  to move up\n");
	printw("                j or c  to move down\n");
	printw("                <space> to halt \n");
	printw("                q       to quit\n\n");
	printw("        Type:   1       easy game\n");
	printw("                2       intelligent monsters\n");
	printw("                3       very intelligent monsters\n");
	refresh();
}

/*
 * over -- game over processing
 */

over()
{
	register int i;
	register int line, col;
	int scorefile = 0;
	struct passwd *getpwuid(), *p;

	refresh();
	signal(SIGINT, SIG_IGN);
	/* clr(); */
	/* high score to date processing */
	if (game != 0)
	{
		col = 45;
		line = 10;
		POS(line++, col);
		(void) printw(" ___________________________ ");
		POS(line++, col);
		(void) printw("| G A M E   O V E R         |");
		POS(line++, col);
		(void) printw("|                           |");
		POS(line++, col);
		(void) printw("| Game type: %6.6s         |",game==1?"easy":game==2?"medium":"smart");
		if ((scorefile = open(MAXSCORE, 2)) != -1)
		{
			read(scorefile, (char *)scoresave, sizeof(scoresave));
			for (i = MSSAVE - 1; i >= 0; i--) {
				if (scoresave[game - 1].entry[i].score < pscore)
				{
					if (i < MSSAVE - 1)
					{
						scoresave[game - 1].entry[i + 1].score =
							scoresave[game - 1].entry[i].score;
						scoresave[game - 1].entry[i + 1].uid =
							scoresave[game - 1].entry[i].uid;
					};
					scoresave[game - 1].entry[i].score = pscore;
					scoresave[game - 1].entry[i].uid = getuid();
				};
			};
			lseek(scorefile, 0l, 0);
			write(scorefile, (char *)scoresave, sizeof(scoresave));
			close(scorefile);
			POS(line++, col);
			(void) printw("| High Scores to date:      |");
			for (i = 0; i < MSSAVE; i++)
			{
				setpwent();
				p = getpwuid(scoresave[game - 1].entry[i].uid);
				POS(line++, col);
				(void) printw("| Player : %-8s  %5u  |", p->pw_name,
					scoresave[game - 1].entry[i].score);
			};
		}
		else
		{
			/* clr(); */
			POS(line++, col);
			(void) printw("|                           |");
			POS(line++, col);
			(void) printw("| Please create a 'paclog'  |");
			POS(line++, col);
			(void) printw("| file. See 'MAXSCORE' in   |");
			POS(line++, col);
			(void) printw("| 'pacdefs.h'.              |");
		};
		POS(line, col);
		(void) printw("|___________________________|");
	};
	refresh();
	leave();
}

/*
 * leave -- flush buffers,kill the Child, reset tty, and delete tempfile
 */

leave()
{
	POS(23, 0);
	refresh();
	endwin();
	exit(0);
}

/*
 * init -- does global initialization and spawns a child process to read
 *      the input terminal.
 */

init()
{
	register int tries = 0;
	static int lastchar = DELETE;
	extern short ospeed;		/* baud rate for crt (for tputs()) */
	int over();
#ifdef USG
	struct termio t;
#endif

	errno = 0;
	(void) time(&timein);	/* get start time */
	srand((unsigned)timein);	/* start rand randomly */
	signal(SIGINT, over);
	signal(SIGQUIT, over);

	/* Curses init - could probably eliminate much of stuff below */
	initscr();
	noecho();
	crmode();
	nonl();
	leaveok(stdscr, TRUE);
	vs_rows = LINES;
	vs_cols = COLS;

#ifdef USG
	ioctl(0, TCGETA, &t);
	t.c_cc[VTIME] = 0;
	t.c_cc[VMIN] = 0;
	ioctl(0, TCSETA, &t);
#endif

	if (delay == 0)
		delay = 500;	/* number of ticks per turn */

	/*
	 * New game starts here
	 */
	if (game == 0)
		instruct();
	while ((game == 0) && (tries++ < 300))
	{
		napms(100);
		poll(1);
	};
	if (tries >= 300)
	{
		/* I give up. Let's call it quits. */
		leave();
	};
	goldcnt = GOLDCNT;
	pscore = 0;
	clr();
}

/*
 * poll -- read characters sent by input subprocess and set global flags
 */

poll(sltime)
{
	int stop;
	register int charcnt;
	int junk;

	stop = 0;
readin:

	refresh();
	/* Check for input we've seen but not processed */
	if (combuf[1] == FULL)
		charcnt = 1;
	else
		charcnt = 0;
#ifdef FIONREAD
	/* Check for typeahead on 4BSD systems. */
	if (charcnt <= 0) {
		ioctl(0, FIONREAD, &junk);
		if (junk)
			charcnt = read(0, combuf, 1);
	}
#else
	/* Have to try a read if nothing else worked. */
	if (charcnt <= 0)
		charcnt = read(0, combuf, 1);
#endif

	switch (charcnt)
	{

	case 0:
		combuf[1] = EMPTY;
		break;

	case -1:
		errgen("READ ERROR IN POLL");
		abort();

	default:
		combuf[0] = combuf[charcnt-1];
		combuf[1] = FULL;
		break;
	};

	if (combuf[1] == EMPTY) 
	{
		if (stop)
		{
			goto readin;
		};
		return;
	};
	combuf[1] = EMPTY;

	switch(combuf[0] & 0177)
	{
	case LEFT:
		pacptr->dirn = DLEFT;
		break;

	case RIGHT:
		pacptr->dirn = DRIGHT;
		break;

	case NORTH:
	case NNORTH:
		pacptr->dirn = DUP;
		break;

	case DOWN:
	case NDOWN:
		pacptr->dirn = DDOWN;
		break;

	case HALT:
		pacptr->dirn = DNULL;
		break;

	case REDRAW:
		clearok(curscr, TRUE);
		break;

	case ABORT:
	case DELETE:
	case QUIT:
		over();
		break;

	case CNTLS:
		stop = 1;
		goto readin;

	case GAME1:
		game = 1;
		break;

	case GAME2:
		game = 2;
		break;

	case GAME3:
		game = 3;
		break;

	default:
		goto readin;
	}
}

getrand(range)
	int range;
{
	register unsigned int q;

	q = rand();
	return(q % range);
}

/*
 * This function is convenient for debugging pacman.  It isn't used elsewhere.
 * It's like printf and prints in a window on the right hand side of the screen.
 */
msgf(fmt, arg1, arg2, arg3, arg4)
char *fmt;
int arg1, arg2, arg3, arg4;
{
	char msgbuf[100];
	static char msgline = 13;

	sprintf(msgbuf, fmt, arg1, arg2, arg3, arg4);
	SPLOT(msgline, 45, msgbuf);
	if (msgline++ > 20)
		msgline = 13;
}

/*
 * napms.  Sleep for ms milliseconds.  We don't expect a particularly good
 * resolution - 60ths of a second is normal, 10ths might even be good enough,
 * but the rest of the program thinks in ms because the unit of resolution
 * varies from system to system.  (In some countries, it's 50ths, for example.)
 *
 * If you're thinking of replacing this with a call to sleep, or by outputting
 * some pad characters, forget it.  You won't get a decent game.  You absolutely
 * HAVE to have a fraction-of-a-second sleep.  Sleeping for a full second will
 * make the game seem really slow.  Outputting pad characters will cause the
 * keyboard to be ahead of the display by whatever buffering the tty driver
 * does, typically about one second.  This causes you to have to anticipate
 * your moves and also makes response dependent on system load - in general
 * the game will be crummy.
 *
 * Here are some reasonable ways to get a good nap.
 *
 * (1) Use the select (dselect?) system call in Berkeley 4.2BSD.
 *
 * (2) Use the 1/10th second resolution wait in the UNIX 3.0 tty driver.
 *     (This is untested - rumor has it that this feature does not work
 *     as advertised, and there might also be problems with the user hitting
 *     a key too soon.)
 *
 * (3) Install the ft (fast timer) device in your kernel.
 *     This is a psuedo-device to which an ioctl will wait n ticks
 *     and then send you an alarm.
 *
 * (4) Install the nap system call in your kernel.
 *     This system call does a timeout for the requested number of ticks.
 */
#ifdef SELECT
napms(ms)
int ms;
{
	select(0, 0, 0, ms);
}
#endif

#if FTIOCSET || NAPSYSCALL
/*
 * Pause for ms milliseconds.  Convert to ticks and wait that long.
 * the constant 6 is HZ/10, change it to 5 for 50 HZ systems.
 * Call nap, which is either defined below or a system call.
 */
napms(ms)
int ms;
{
	int ticks = ms * 6 / 100;
	if (ticks <= 0)
		ticks = 1;
	nap(ticks);	/* call either the code below or nap system call */
}
#endif

#ifdef FTIOCSET
/*
 * This uses the ft device.
 * The following code is adapted from the sleep code in libc.
 * It uses the "fast timer" device posted to USENET in Feb 1982.
 * nap is like sleep but the units are ticks (e.g. 1/60ths of
 * seconds in the USA).
 */
#include <setjmp.h>
static jmp_buf jmp;
static int ftfd;

nap(n)
unsigned n;
{
	int napx();
	unsigned altime;
	int (*alsig)() = SIG_DFL;
	char *ftname;
	struct requestbuf {
		short time;
		short signo;
	} rb;

	if (ftfd <= 0) {
		ftname = "/dev/ft0";
		while (ftfd <= 0) {
			ftfd = open(ftname, 0);
			if (ftfd <= 0)
				ftname[7] ++;
		}
	}
	if (n==0)
		return;
	altime = alarm(1000);	/* time to maneuver */
	if (setjmp(jmp)) {
		signal(SIGALRM, alsig);
		alarm(altime);
		return;
	}
	if (altime) {
		if (altime > n)
			altime -= n;
		else {
			n = altime;
			altime = 1;
		}
	}
	alsig = signal(SIGALRM, napx);
	rb.time = n;
	rb.signo = SIGALRM;
	ioctl(ftfd, FTIOCSET, &rb);
	for(;;)
		pause();
	/*NOTREACHED*/
}

static
napx()
{
	longjmp(jmp, 1);
}
#endif

#ifdef USG
#define IDLETTY "/dev/idletty"
/*
 * Do it with the timer in the tty driver.  Resolution is only 1/10th
 * of a second.  Problem is, if the user types something while we're
 * sleeping, we wake up immediately, and have no way to tell how long
 * we should sleep again.  So we're sneaky and use a tty which we are
 * pretty sure nobody is using.
 *
 * This requires some care.  If you choose a tty that is a dialup or
 * which otherwise can show carrier, it will hang and you won't get
 * any response from the keyboard.  You can use /dev/tty if you have
 * no such tty, but response will feel funny as described above.
 */
napms(ms)
int ms;
{
	struct termio t, ot;
	static int ttyfd;
	int n, tenths;
	char c;

	if (ttyfd == 0)
		ttyfd = open(IDLETTY, 2);
	if (ttyfd < 0) {
		fprintf(stderr, "Need read/write permission on %s\r\n",
				IDLETTY);
		leave();
	}
	tenths = (ms+50) / 100;
	ioctl(ttyfd, TCGETA, &t);
	ot = t;
	t.c_cflag &= ~ICANON;
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = tenths;
	ioctl(ttyfd, TCSETA, &t);
	n = read(ttyfd, &c, 1);
	if (n > 0) {
		combuf[0] = c;
		combuf[1] = FULL;
	}
	ioctl(ttyfd, TCSETA, &ot);
}
#endif

#ifndef A_BLINK
/* Simulations for the old curses */
flushinp()
{
#ifdef USG
	ioctl(0, TCFLSH, 0);
#else
	ioctl(0, TIOCFLUSH, 0);
#endif
}

beep()
{
	putchar('\7');
	fflush(stdout);
}

baudrate()
{
	int baud;

#ifdef USG
	baud = _tty.c_cflag & CBAUD;
#else
	baud = _tty.sg_ospeed;
#endif
	switch (baud) {
	case B110: return 110;
	case B300: return 300;
	case B1200: return 1200;
	case B2400: return 2400;
	case B4800: return 4800;
	case B9600: return 9600;
	case EXTA: return 19200;
	}
	return 9600;	/* Guess */
}
#endif
