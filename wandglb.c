#include    "wanddef.h"
/*
**      WANDGLB -- Non-deterministic fantasy story tool
**          Global storage allocations
** Compile: cc -c -O -q wandglb.c
** Copyright (c) 1978 by Peter S. Langston - New  York,  N.Y.
*/

char    *whatglb =  "@(#)wandglb.c	2.11  last mod 5/29/82 -- (c) psl 1978";
char    *glb_h   =  H_SCCS;

/* the following defines are used only in wandglb.c and may be changed */
		      /* numbers in [] are #bytes data space used for each */
#define MAXWRDS     768    /* [6] max words incl ones mentioned in actions */
#define MAXINDEX    768                 /* [8] max states total (all locs) */
#define MAXPREACTS  32     /* [10+2*MAXACTWDS+6*MAXFIELDS] max pre actions */
#define MAXPOSTACTS 100   /* [42+2*MAXACTWDS+6*MAXFIELDS] max post actions */


struct  ndxstr    ndx[MAXINDEX];

struct  paramstr    param;

struct  placestr    place;

struct  actstr  pre_acts[MAXPREACTS];
struct  actstr  post_acts[MAXPOSTACTS];

// Note that the syn field must be turned into the index of the root word
struct  wrdstr wrds[MAXWRDS] = {
	listunused,	0,  0,  MAXWRDS, /* hopefully nothing matches this */
	"drop",		0,  0,  0,
	"inventory",	0,  0,  0,
	"quit",		0,  0,  0,
	"save",		0,  0,  0,
	"take",		0,  0,  0,
	"pick",		1,  0,  0,
	"restore",	0,  0,  0,
	"look",		0,  0,  0,
	"initialize",	0,  0,  0,
	"history",	0,  0,  0,
	"north",	0,  0,  0,
	"n",		1,  0,  0,
	"south",	0,  0,  0,
	"s",		1,  0,  0,
	"east",		0,  0,  0,
	"e",		1,  0,  0,
	"west",		0,  0,  0,
	"w",		1,  0,  0,
	"up",		0,  0,  0,
	"u",		1,  0,  0,
	"down",		0,  0,  0,
	"d",		1,  0,  0,
	"northeast",	0,  0,  0,
	"ne",		1,  0,  0,
	"southeast",	0,  0,  0,
	"se",		1,  0,  0,
	"southwest",	0,  0,  0,
	"sw",		1,  0,  0,
	"northwest",	0,  0,  0,
	"nw",		1,  0,  0,      /* must be the last direction verb */
	"~snoop",	0,  0,  0,     /* this only works if you are owner */
	"~goto",	0,  0,  0,     /* this only works if you are owner */
	"~vars",	0,  0,  0,     /* this only works if you are owner */
	"~version",	0,  0,  0,     /* this only works if you are owner */
	"*",		0,  0,  0,
	"N1",		0,  0,  0,
	"N2",		0,  0,  0,
	"all",		0,  0,  0,      /* used in "take all" & "drop all" */
	0,		0,  0,  0,  /* "all" must be the last defined here */
};

struct  wrdstr spvars[] = {            /* special construct & their meanings */
	"CUR_LOC",	0,  0,  CUR_LOC,
	"PREV_LOC",	0,  0,  PREV_LOC,
	"INP_W1",	0,  0,  INP_W1,
	"INP_W2",	0,  0,  INP_W2,
	"INP_W3",	0,  0,  INP_W3,
	"INP_W4",	0,  0,  INP_W4,
	"INP_W5",	0,  0,  INP_W5,
	"INP_WC",	0,  0,  INP_WC,
	"NUM_CARRY",	0,  0,  NUM_CARRY,
	"MAX_CARRY",	0,  0,  MAX_CARRY,
	"NOW_YEAR",	0,  0,  NOW_YEAR,
	"NOW_MONTH",	0,  0,  NOW_MONTH,
	"NOW_DOM",	0,  0,  NOW_DOM,
	"NOW_DOW",	0,  0,  NOW_DOW,
	"NOW_HOUR",	0,  0,  NOW_HOUR,
	"NOW_MIN",	0,  0,  NOW_MIN,
	"NOW_SEC",	0,  0,  NOW_SEC,
	"NOW_ET",	0,  0,  NOW_ET,
	"BREVITY",	0,  0,  BREVITY,
	"LOC_VIEW",	0,  0,  LOC_VIEW,
	"OBJ_VIEW",	0,  0,  OBJ_VIEW,
	"INP_N1",	0,  0,  INP_N1,
	"INP_N2",	0,  0,  INP_N2,
	"NUM_MOVES",	0,  0,  NUM_MOVES,
	"NUM_PLACES",	0,  0,  NUM_PLACES,
	0,		0,  0,  0,
};

char *thereis[] = {
	" ", "There is ",   "There is ", "There are ",
};

char *aansome[] = {
	" ", "a ", "an ",  "some ",
};

char fldels[] = { FIELDELIM, LINEDELIM, 0, };             /* delimits fields */
char vardel[] = { VARCHAR, 0, };                     /* terminates variables */
char wrdels[] = {                                       /* to separate words */
	' ', ' ' | 0200, ',', '.', ';', '!', '?', 0,
};

char    listunused[] = "\b\b\b\b";        /* used to mark empty list entries */

char    locfile[PATHLENGTH];
char    miscfile[PATHLENGTH];
char    tmonfil[PATHLENGTH];
char    monfile[PATHLENGTH];

char    *stdpath    = WANDPATH(/);		 /* where std. worlds live */
char    curname[PATHLENGTH] = "a3";                       /* default world */
char    *defmfile   = WANDPATH(wand.mon);		/* def monfil name */

char    mfbuf[BUFSIZ];                            /* so stdio won't sbrk() */
char    wfbuf[BUFSIZ];                                            /* ditto */

char	history[HISTLEN][BUFSIZE];            /* the last HISTLEN commands */
int	histi;				             /* index into history */



int     maxwrds     = MAXWRDS;
int     maxlocs     = MAXLOCS;
int     maxndx    = MAXINDEX;
int     maxacts     = MAXACTS;
int     maxpreacts  = MAXPREACTS;
int     maxpostacts = MAXPOSTACTS;
int     maxfields   = MAXFIELDS;
int     maxvars     = MAXVARS;
int     ldescfreq   = 5;                 /* how often long desc is printed */

char    fieldelim   = FIELDELIM;
char    linedelim   = LINEDELIM;
char    eschar      = ESCHAR;
char    varchar     = VARCHAR;
char    dotchar     = DOTCHAR;
char    atchar      = ATCHAR;
char    comchar     = COMCHAR;

int     monitor     = -1;                     /* -1 => monitor, 0 => don't */
int     monloc, monstate;

int     max_carry   = 8;           /* default max objects to carry at once */

char    inwrd[MAXACTWDS][32];                       /* current input words */
char    locseen[MAXLOCS], locstate[MAXLOCS];
int     var[MAXVARS];
FILE	*mfp, *wfp;
