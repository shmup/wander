#include    <stdio.h>
#include    "wanddef.h"
/*
**      WANDSYS -- System dependent routines for Wander
**	    Interfaces to stdio routines to keep track of file position
**	    on V6 systems without tell().
** Copyright (c) by Peter S. Langston - New  York,  N.Y.
*/

/*#define	NOFTELL		/* uncomment this for systems without tell() */

#ifdef NOFTELL
static	char    *whatwand = "@(#)wandsys.c	1.1  WITHOUT FTELL() 2/22/84 -- (c) psl";
#else
static	char    *whatwand = "@(#)wandsys.c	1.1  with ftell() 2/22/84 -- (c) psl";
#endif
static	char    *wand_h =   H_SCCS;

extern	FILE    *fpungot;

#ifdef NOFTELL
#define	NUMWTELL	2
struct	wtellstr {
	FILE	*wt_fp;				/* associated file pointer */
	long	wt_addr;				/* wtell() address */
} wt[NUMWTELL + 1];
#endif

FILE	*
wopen(file, rwflg)			 	     /* interface to fopen */
char	*file, *rwflg;
{
#ifdef	NOFTELL
	register int i;
	FILE *fp;

	if ((fp = fopen(file, rwflg)) != (FILE *) NULL) {
	    i = wtfind((FILE *) NULL);
	    wt[i].wt_fp = fp;
	    wt[i].wt_addr = 0L;
	}
	return(fp);
#else
	return(fopen(file, rwflg));
#endif
}

wseek(fp, addr, mode)				   /* interface to fseek() */
FILE	*fp;
long	addr;
{
#ifdef NOFTELL
	register int i;

	i = wtfind(fp);
	if (mode == 0)
	    wt[i].wt_addr = addr;
	else if (mode == 1)
	    wt[i].wt_addr += addr;
	else if (mode == 2)
	    printf("Illegal wseek(%d, %D, %d)\n", fp, addr, mode);
#endif
	if (fpungot == fp)
	    fpungot = (FILE *) NULL;
	return(fseek(fp, addr, mode));
}

wgetc(fp)					    /* interface to getc() */
FILE	*fp;
{
#ifdef	NOFTELL
	register int i;

	if (fp != stdin) {
	    i = wtfind(fp);
	    wt[i].wt_addr++;
	}
#endif
	return(getc(fp));
}

long
wtell(fp)
FILE	*fp;
{
#ifdef	NOFTELL
	register int i;

	if (fp == stdin)
	    return(0L);
	i = wtfind(fp);
	return(wt[i].wt_addr);
#else
	extern long ftell();

	return(ftell(fp));
#endif
}

wclose(fp)					  /* interface to fclose() */
FILE	*fp;
{
#ifdef	NOFTELL
	register int i;

	i = wtfind(fp);
#endif
	if (fpungot == fp)
	    fpungot = (FILE *) NULL;
	return(fclose(fp));
}

#ifdef	NOFTELL
wtfind(fp)		 	 /* find record for specified file pointer */
FILE	*fp;
{
	register int i;

	for (i = 0; i < NUMWTELL; i++)
	    if (fp == wt[i].wt_fp)
		return(i);
	printf("wtfind(%d): Couldn't find a record\n", fp);
	return(NUMWTELL);
}
#endif
