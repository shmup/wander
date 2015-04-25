#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"wanddef.h"
/*
**      WANDER -- Non-deterministic fantasy story tool
** Copyright (c) 1978 by Peter S. Langston - New  York,  N.Y.
*/

static	char	*whatwand = "@(#)wand2.c	1.4 2/23/85 -- (c) psl 1978";

static	char	*wand_h =   H_SCCS;

extern	char	ungotlin[BUFSIZE];
extern	int	curstate, actrace, owner;
extern	int	vrbquit, vrbsave, vrbrest, vrbtake, vrbdrop, vrbgoto, vrbinve;
extern	int	vrblook, vrbinit, vrbstar, vrbsnoop, vrbvars, vrbvers;
extern	int	lstdirvrb;
extern	int	objnum1, objnum2, objall;
extern	FILE	*fpungot, *monfp;
extern	long    lbegaddr;           /* addr of begin of last getlin() line */
extern	long    ungotaddr;        /* addr of begin of last ungetlin() line */

extern	char	*ctime();
extern	long    wtell();
extern	FILE	*fopen(), *wopen();

void
restart(name)                       /* (init) restart from specified files */
char	*name;
{
	int i, numwrds;

	/* forget words not being carried and not internal */
	numwrds = wrds[0].w_loc;
	for (i = objall + 1; wrds[i].w_word != 0 && i < numwrds; i++)
	    if (wrds[i].w_word != listunused && wrds[i].w_loc >= 0)
		wrds[i].w_word = listunused;
	/* open new .misc & .wrld and create ndx */
	if (get_files(name, FRESTART) == -1
	 && get_files(curname, FRESTART) == -1)
	    exit(2);
	for (i = maxlocs; --i >= 0; )
	    locseen[i] = 0;
	place.p_loc = -1;
	return;
}

void
takeobj(obj)
{
	int i, j;
	struct wrdstr *wp;
	char buf[128];

	wp = &wrds[obj];
	if (wp->w_loc == var[CUR_LOC]) {
	    i = 0;
	    for (j = 1; wrds[j].w_word; j++)
		if (wrds[j].w_loc < 0)
		    i++;
	    var[NUM_CARRY] = i;
	    if (i >= var[MAX_CARRY]) {
		printf("You can't carry anything more;");
		printf(" perhaps you should drop something.\n");
		return;
	    }
	    wp->w_loc = -1;
	    var[NUM_CARRY]++;
	    printf("Done\n");
	} else if (wrds[obj].w_loc < 0)
	    printf("%s!\n",
	     objdesc("You're already carrying ", "the ", wp, "", buf, sizeof buf));
	else
	    printf("%s.\n", objdesc("I don't see ", "any ", wp, " here", buf, sizeof buf));
	return;
}

char	*
objdesc(pre, art, wp, post)  /* assemble an object description in buf */
char	*pre, *art, *post;
struct  wrdstr  *wp;
{
	char *cp, buf[1024];

	if (wp->w_flg & W_ASIS) {		// don't augment the description
	    cp = wp->w_word;
	} else {				// do augment the description
	    if (pre == (char *) 0) {           /* implies location description */
		cp = cpyn(buf, thereis[class(wp)], sizeof(buf) - 1);
	    } else {                             /* not a location description */
		cp = strncpy(buf, pre, sizeof(buf) - 1);
		if (wp->w_flg & W_DONLY)
		    wp++;
	    }
	    if ((wp->w_flg & (W_NOART | W_ASIS)) == 0) {  /* if article needed */
		if (art)				      /* specified article */
		    cp = cpyn(cp, art, buf + sizeof(buf) - cp - 1);
		else					/* default article */
		    cp = cpyn(cp, aansome[class(wp)], buf + sizeof(buf) - cp - 1);
	    }
	    cp = cpyn(cp, wp->w_word, buf + sizeof(buf) - cp - 1);
	    cpyn(cp, post, buf + sizeof(buf) - cp - 1);
	    cp = buf;
	}
	return(deparity(cp));
}

char	*
deparity(char *fp)
{
	char *dp;
	int i;
	static char dbuf[1024];

	dp = dbuf;
	for (i = sizeof dbuf; (--i > 0) && (*dp = *fp++); *dp++ &= 0177);
	*dp = '\0';
	return(dbuf);
}

void
bytecopy(from, to, length)
char	*from, *to;
{
	char *fp, *tp;
	int i;

	fp = from;
	tp = to;
	i = length;
	while (--i >= 0)
	    *tp++ = *fp++;
	return;
}

char	*			// split off the next field
movchars(from, to, delims)
char	*from, *to, *delims;
{
        char *dp, c;

        while (c = *from++) {
            for (dp = delims; *dp; dp++) {
                if (*dp == c) {
                    *to = '\0';
                    return(from);
                }
	    }
            *to++ = c;
        }
        --from;
        *to = '\0';
        return(from);
}

int
obj_at(obj, loc)
{
        int i;

	i = wrds[obj].w_loc;
	if (i < 0)
	    i = var[CUR_LOC];
        if ((loc != 0 && loc == i)
         || (loc == 0 && i == var[CUR_LOC]))
	    return(1);
        return(0);
}

int
oneof(wrd, w)       /* return 1 if "wrd" is = one of w[0] ... w[MAXACTWDS] */
int	w[];
{
	int i;

	for (i = 0; i < MAXACTWDS && w[i]; i++)
	    if (wrd == w[i])
		return(1);
	return(0);
}

int
class(wp)        /* 1 => singular, consonant first, 2 => sing, vowel first */
struct  wrdstr  *wp;                                        /* 3 => plural */
{
	char c, *cp;

	if (wp->w_flg & W_PLUR)
	    return(3);                                  /* plural, class 3 */
	cp = wp->w_word;
	c = *cp;
	if ((wp->w_flg & W_SING) == 0) {
	    while (*cp++);
	    if (cp[-2] == 's')
		return(3);                              /* plural, class 3 */
	}
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
	    return(2);                         /* sing with vowel, class 2 */
	return(1);                         /* sing with consonant, class 1 */
}

void
dotpair(type, string, fp)
char	*string;
struct   fieldstr     *fp;
{
        char *cp;

        fp->f_type = type;
	for (cp = string; *cp; cp++)
	    if (*cp == dotchar) {
		*cp++ = '\0';
                break;
	    }
	if (*string == varchar)
	    fp->f_type |= FLD1_VAR;
        fp->f_fld1 = atov(string);
	if (*cp == varchar)
	    fp->f_type |= FLD2_VAR;
        fp->f_fld2 = atov(cp);
	return;
}

void
atpair(type, string, fp)
char	*string;
struct   fieldstr     *fp;
{
        char *cp;
	int i;

        fp->f_type = type;
        for (cp = string; *cp != '\0'; cp++)
	    if (*cp == atchar && cp[1] != atchar) {
                *cp++ = '\0';
                break;
            }
	i = -1;
	if (string[0] == varchar) {
	    movchars(&string[1], &string[1], vardel);
	    if ((i = which(&string[1], spvars)) >= 0)
		i = spvars[i].w_loc;     /* loc in spvars holds real var # */
	    fp->f_fld1 = i;
	    fp->f_type |= FLD1_VAR;
	}
	if (i < 0)
	    fp->f_fld1 = wrdadd(string, 0, 0, 0);
	if (*cp == varchar)
	    fp->f_type |= FLD2_VAR;
	fp->f_fld2 = atov(cp);
	return;
}

int
atov(string)                    /* return coded form of number or variable */
char	*string;
{
	int i;

	if (*string == '\0')
	    return(0);
	if (*string == varchar)
	    movchars(&string[1], string, vardel);
	if ((*string >= '0' && *string <= '9') || *string == '-')
	    return(atoi(string));                         /* simple number */
	if ((i = which(string, spvars)) >= 0)
	    return(spvars[i].w_loc);                     /* special number */
	return(wrdadd(string, 0, 0, 0));                   /* hashed value */
}

char	*storebuf   = 0;
char	*storebp    = 0;
int	storenleft  = 0;

char	*
store(string)		// copy string into storebuf and return a pointer to it
char	*string;
{
        char *cp;
	extern char *sbrk();

	if (storebp == 0) {
	    if ((storebp = storebuf = sbrk(storenleft = 4096)) == (char *) -1) {
		perror("Initial sbrk(4096) failed");
		exit(3);
	    }
	}
	storenleft -= length(string);
	if (storenleft < 0) {
	    if (sbrk(4096) == (char *) -1) {
		perror("sbrk(4096) failed");
		exit(3);
	    }
	    storenleft += 4096;
        }
	storebp = cpy(cp = storebp, string) + 1;
        return(cp);
}

int
length(string)
char	*string;
{
        char *cp;

        for (cp = string; *cp++ != '\0'; );
        return(cp - string);
}

int				       /* put wrd vals in w[0], w[1],  ... */
wdparse(string, w, nums, flag)            /* if flag != 0 add to wrds list */
char	*string;                 /* if flag == 0 (user inp) put #s in nums */
int	w[], nums[];                              /* and return # of words */
{
        char *bp;
	char wdbuf[128];
	int numactwds, i, numnums;

	if (flag == 0)
	    nums[0] = nums[1] = numnums = 0;
	cpyn(bp = wdbuf, string, sizeof(wdbuf) - 1);
	for (numactwds = 0; numactwds < MAXACTWDS; ) {
	    bp = movchars(bp, wdbuf, wrdels);
	    if (wdbuf[0] == '\0')
		break;
	    if (flag) {                                    /* script input */
		w[numactwds] = wrdadd(wdbuf, 0, 0, 0);
		cpy(inwrd[numactwds++], wdbuf);
	    } else {                                         /* user input */
		i = which(wdbuf, wrds);
		if (i > 0) {                              /* hashed symbol */
		    w[numactwds] = i;
		    cpy(inwrd[numactwds++], wdbuf);
		} else if ((*wdbuf == '-' ||
		 (*wdbuf >= '0' && *wdbuf <= '9'))
		 && numnums < MAXINPNUMS) {                      /* number */
		    w[numactwds] = numnums == 0? objnum1 : objnum2;
		    cpy(inwrd[numactwds++], wdbuf);
		    nums[numnums++] = atoi(wdbuf);
		}
	    }
	}
	for (i = numactwds; i < MAXACTWDS; ) {
	    w[i] = NO_WORD;
	    inwrd[i++][0] = '\0';
	}
	return(numactwds);
}

char	*
msglin(fp, addr)
FILE    *fp;
long    addr;
{
	char *cp;
	char string[BUFSIZE];
	int isdesc;

	wseek(fp, addr, 0);
	getlin(fp, string);
	isdesc = string[0] > ' ';
	for (cp = string; *cp; ) {
	    cp = movchars(cp, string, fldels);
	    if (isdesc)                               /* short description */
		return(msgfmt(cp));
	    if (string[0] == 'm' && string[1] == '=')           /* message */
		return(msgfmt(&string[2]));
	}
/****/printf("Failed to find message at loc %ld <%s>\n", addr, string);
	return("<lost message?>");
}

void
quit(int n)
{
	if (monitor)
	    monsav();
        if (n == QUIT_SCORE)
            printf("You wandered to %d place%s in %d moves.\n",
	     var[NUM_PLACES], splur(var[NUM_PLACES]), var[NUM_MOVES]);
        exit(0);
}

void
save(file)
char	*file;
{
	char *cp, newname[PATHLENGTH];
	int fh, sbufsize;

	if (*file == '\0') {
	    for (cp = file = curname; *cp; )
		if (*cp++ == '/')
		    file = cp;
	    sprintf(newname, "%s.save", file);
	    file = newname;
	}
	if ((fh = open(file, 1)) < 0 && (fh = creat(file, 0600)) < 0) {
	    printf("Can't open \"%s\", sorry\n", file);
            return;
        }
	printf("Saving the current environment under the name \"%s\" ...", file);
	param.p_pathlength = PATHLENGTH;
	param.p_histlen = HISTLEN;
	param.p_histi = histi;
	param.p_maxlocs = maxlocs;
	param.p_maxwrds = maxwrds;
	param.p_maxvars = maxvars;
	param.p_maxndx = maxndx;
	param.p_maxpre = maxpreacts;
	param.p_maxpost = maxpostacts;
	param.p_storebuf = storebuf;
	param.p_sbufsiz = storebp - storebuf;
	param.p_time = time(0);
	param.p_msize = fsize(mfp);
	param.p_wsize = fsize(wfp);
	write(fh, &param, sizeof (struct paramstr));
	write(fh, curname, PATHLENGTH);
	write(fh, locseen, sizeof(locseen[0]) * maxlocs);
	write(fh, locstate, sizeof(locstate[0]) * maxlocs);
	write(fh, var, sizeof(var[0]) * maxvars);
	write(fh, wrds, sizeof(struct wrdstr) * maxwrds);
	write(fh, ndx, sizeof(struct ndxstr) * maxndx);
	write(fh, pre_acts, sizeof(struct actstr) * maxpreacts);
	write(fh, post_acts, sizeof(struct actstr) * maxpostacts);
	write(fh, history, HISTLEN * PATHLENGTH);
	write(fh, storebuf, param.p_sbufsiz);
	close(fh);
	printf("\n");
	return;
}

void
restore(char *file, int flag)
{
	char *cp, newname[PATHLENGTH];
	int then, diff, i, fh;
	extern char *sbrk();

	if (*file == '\0') {
	    for (cp = file = curname; *cp; )
		if (*cp++ == '/')
		    file = cp;
	    sprintf(newname, "%s.save", file);
	    file = newname;
	}
	printf("Restoring from the file \"%s\" ", file);
	if ((fh = open(file, 0)) < 0) {
	    printf("Can't open \"%s\", sorry\n", file);
            exit(1);
        }
	i = read(fh, &param, sizeof (struct paramstr));
	printf("saved %s", ctime(&param.p_time));
	i = read(fh, newname, PATHLENGTH);
	printf(".");
	if (get_files(newname, flag) == -1)
            exit(2);
	printf(".");
	if (storebuf == 0) {                        /* no sbrk() space yet */
	    if ((storebuf = storebp = sbrk(4096)) == (char *) -1) {
		printf("Initial sbrk(4096) failed\n");
		exit(1);
	    }
	    storenleft = 4096;
	}
	if (param.p_pathlength != PATHLENGTH
	 || param.p_histlen != HISTLEN
	 || param.p_maxlocs != maxlocs
	 || param.p_maxwrds != maxwrds
	 || param.p_maxvars != maxvars
	 || param.p_maxpre != maxpreacts
	 || param.p_maxpost != maxpostacts
	 || param.p_maxndx != maxndx) {
	    printf("\n`%s' saved from another version of Wander.\n", file);
	    exit(1);
	}
	if (param.p_msize != fsize(mfp)) {
	    printf("\n`%s' saved with a different %s.misc file.\n", file, newname);
	    exit(1);
	}
	if (param.p_wsize != fsize(wfp)) {
	    printf("\n`%s' saved with a different %s.wrld file.\n", file, newname);
	    exit(1);
	}
	histi = param.p_histi;
	read(fh, locseen, sizeof(locseen[0]) * maxlocs);
	read(fh, locstate, sizeof(locstate[0]) * maxlocs);
	read(fh, var, sizeof(var[0]) * maxvars);
	read(fh, wrds, sizeof(struct wrdstr) * maxwrds);
	read(fh, ndx, sizeof(struct ndxstr) * maxndx);
	read(fh, pre_acts, sizeof(struct actstr) * maxpreacts);
	read(fh, post_acts, sizeof(struct actstr) * maxpostacts);
	read(fh, history, HISTLEN * PATHLENGTH);
	printf(".");
	// to read the saved storebuf we can dump everything we have stored
	storenleft += storebp - storebuf;	// reclaim the space
	storebp = storebuf;			// now it's empty
	storenleft -= param.p_sbufsiz;		// will there be any left?
	if (storenleft < 0) {			// we'll need more
	    diff = -storenleft;			// need at least this much
	    i = (diff + 07777) & ~07777;	// round up to a mult of 4k
	    if (sbrk(i) == (char *) -1) {
		printf("sbrk(%d) failed.\n", i);
		exit(1);
	    }
	    storenleft += i;			// suold be positive now
	}
	read(fh, storebuf, param.p_sbufsiz);
	storebp = storebuf + param.p_sbufsiz;
	close(fh);
	// now we need to readjust all the w_word pointers to the new storebuf offset
	diff = storebuf - param.p_storebuf;
	for (i = 1; wrds[i].w_word && i <= maxwrds; i++)
	    wrds[i].w_word += diff;
	printf("\n");
	boswell("restore");
        place.p_loc = -1;
        inventory();
	return;
}

void
monsav()                         /* copy out temporary monfile & unlink it */
{
	int tmfh, pmfh, i;
	char buf[512];

	fclose(monfp);
	if ((tmfh = open(tmonfil, 0)) == -1) {
	    fprintf(stderr, "Can't open %s\n", tmonfil);
	    return;
	}
	unlink(tmonfil);
	if ((pmfh = open(monfile, 2)) == -1) {
	    fprintf(stderr, "Can't open %s\n", monfile);
	    close(tmfh);
	    return;
	}
	lseek(pmfh, (long) 0, 2);
	while ((i = read(tmfh, buf, 512)) > 0)
	    write(pmfh, buf, i);
	close(tmfh);
	close(pmfh);
	return;
}

long
getndx(loc, state)
{
        struct ndxstr *ip;

        for (ip = ndx; ip->i_loc >= 0; ip++)
            if (ip->i_loc == loc && ip->i_state == state)
                return(ip->i_addr);
        return(-1l);
}

char	*
msgpara(fp, addr)
FILE    *fp;
long    addr;
{
	char string[BUFSIZE];

	wseek(fp, addr, 0);
	getpara(fp, string);
	return(msgfmt(string));
}

char	*
msgfmt(string)
char	*string;
{
/****/int xp;
	char *sp, *bp, c;
	char junk[512];
        int i;
	static char buf[BUFSIZE];

        sp = string;
        bp = buf;
        while (c = *sp++) {
	    if (c == varchar) {
		if (*sp == varchar)
		    sp++;
		else {
		    if (*sp < '0' || *sp > '9') {   /* special var? */
			movchars(sp, junk, vardel);
			if ((i = which(junk, spvars)) >= 0) {
			    i = spvars[i].w_loc;           /* special var */
			    if (i >= INP_W1 && i <= INP_W1 + MAXACTWDS)
				bp = cpyn(bp, inwrd[i - INP_W1], buf + sizeof(buf) - bp - 1);
			    else {			/* numeric special */
				sprintf(bp, "%d", var[i]);
				bp = cpyn(bp, bp, buf + sizeof(buf) - bp - 1);
			    }
			}
		    } else {                            /* normal variable */
			i = atoi(sp);
			sprintf(bp, "%d", var[i]);
			bp = cpyn(bp, bp, buf + sizeof(buf) - bp - 1);
		    }
		    while (*sp && *sp++ != varchar);
                    continue;
                }
            }
	    *bp++ = c & 0177;
        }
        *bp = '\0';
        return(buf);
}

void
inventory()
{
        int i, obj;
	char buf[128];

        i = 0;
	for (obj = 1; wrds[obj].w_word; obj++) {
	    if (wrds[obj].w_loc < 0) {
		if (i++ == 0)
		    printf("You are carrying ");
		else
		    printf("             and ");
		printf("%s\n", objdesc("", (char *) 0, &wrds[obj], "", buf, sizeof buf));
            }
	}
        if (i == 0)
            printf("You're empty-handed\n");
	return;
}

int
wrdadd(word, syn, iloc, flg)             /* return index of the root word, */
char	*word;                                   /* adding it if necessary */
{
	char *wrd;
	int i, limit;

	i = which(word, wrds);
        if (i >= 0)			// we already have it
            return(i);
	limit = wrds[0].w_loc - 2;	// add it
	for (i = 1; (wrd = wrds[i].w_word) != 0 && i < limit; i++) {
	    if (wrd == listunused)
		break;
	}
	if (i >= limit && wrd != 0 && wrd != listunused)
	    fprintf(stderr, "wrdadd: I had to forget '%s' (%d wds already)\n",
	     wrds[i].w_word, limit);
	if (wrd == 0)                       /* push end of list down */
	    wrds[i+1].w_word = 0;
	wrds[i].w_word = store(word);
	wrds[i].w_syn = syn;        // index of root word iff a synonym
	wrds[i].w_loc = iloc;
	wrds[i].w_flg = flg;
        return(i);
}

int
which(word, wrds)  /* find root (description) text for existing word */
char	*word;
struct  wrdstr  *wrds;
{
        int i;

	i = wfnd(word, wrds);
        if (i >= 0 && wrds[i].w_syn)
	    i = wrds[i].w_syn;
        return(i);
}

int
wfnd(char *word, struct wrdstr *wrds)
{
	char *wrd;
	int i, limit;

	limit = wrds[0].w_loc - 2;
	for (i = 1; (wrd = wrds[i].w_word) != 0 && i < limit; i++) {
	    if (strcmp(wrd, word) == 0)
		return(i);
	}
	return(-1);
}

void
ungetlin(ifp, cbp)
FILE *ifp;
char *cbp;
{
        fpungot = ifp;
	ungotaddr = lbegaddr;
	cpyn(ungotlin, cbp, sizeof(ungotlin));
	return;
}

// read in a "line" from ifp, observing quotes and escapes and removing the final \n
int
getlin(ifp, cbp)
FILE *ifp;
char *cbp;
{
        char *cp;
        int c;
	char *cbe, lastc;
        int quote, escape;

        if (fpungot == ifp) {       /* if a line was put back */
            fpungot = (FILE *) NULL;
	    lbegaddr = ungotaddr;
	    return(cpy(cbp, ungotlin) - cbp);
        }
	lbegaddr = wtell(ifp);
	cbe = &cbp[BUFSIZE];
        quote = escape = 0;
	lastc = '\n';
	for (cp = cbp; (c = wgetc(ifp)) != EOF; ) {
	    if (lastc == '\n' && c == comchar) {	// skip over comment
		while (wgetc(ifp) != '\n');
		continue;
	    }
	    lastc = c;
	    if (escape) {
		if (c == linedelim)
		    c = 0;
		else if (c == fieldelim || c == varchar)
		    c |= 0200;
		else if (c == 'n')
		    c = '\n' | 0200;
		else if (c == 'r')
		    c = '\r';
		else if (c == 'b')
		    c = '\b';
		else if (c == 't')
		    c = '\t';
		if (c)
		    *cp++ = c;
		escape = 0;
	    } else {
		if (quote && (c == '\n' || c == ' ' || c == '\t'))
		    c |= 0200;
		if (c == linedelim)
		    break;
		if (c == '"') {
		    quote ^= 1;
		    continue;
		}
		if (c == eschar) {
		    escape = 1;
		    continue;
		}
		if (c == '\t')
		    c = ' ';
		if (c == ' ' && cp > cbp && cp[-1] == ' ')
		    continue;
		*cp++ = c;
	    }
	    if (cp >= cbe) {
		fprintf(stderr, "line <%s> too long, [%ld] in %s\n",
		 cbp, wtell(ifp), ifp == wfp? locfile : miscfile);
		exit(1);
	    }
	}
	if (c == EOF)
	    *cp = '\0';
	else
	    *cp++ = '\0';
        return(cp - cbp);
}

int
getpara(ifp, cbp)
FILE *ifp;
char *cbp;
{
        char *bp;
	char buf[BUFSIZE];

	bp = cbp;
	while (getlin(ifp, buf) > 0 && buf[0] != fieldelim && buf[0] != '#') {
	    bp = cpy(bp, buf);
            *bp++ = '\n';
        }
        *bp++ = '\0';
        ungetlin(ifp, buf);
        return(bp - cbp);
}

int
atoip(ptrptr)
char **ptrptr;
{
	int num, base;
	char *cp;
	int neg;

	cp = *ptrptr;
	num = 0;
	base = 10;
	neg = 0;
loop:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '-') {
		neg++;
		cp++;
		goto loop;
	}
	if (*cp == '0') {
		base = 8;
		cp++;
	}
	while (*cp >= '0' && *cp <= '9')
		num = num * base + *cp++ - '0';
	*ptrptr = cp;
	return(neg ? -num : num);
}

char	*
splur(n)
{
	return(n==1? "" : "s");
}

// This should really be stpcpy() from libc
char	*
cpy(char *tp, char *fp)
{
	while (*tp++ = *fp++);
	return(--tp);
}

// This should really be stpncpy() from libc
char	*
cpyn(char *tp, char *fp, int n)
{
	while ((*tp++ = *fp++) && (--n > 0));
	*--tp = '\0';	// just in case n==0 (not like stpncpy)
	return(tp);
}

off_t
fsize(FILE *fp)
{
	struct stat sbuf;

	fstat(fileno(fp), &sbuf);
	return(sbuf.st_size);
}

void
boswell(char *command)
{
	cpyn(history[histi], command, BUFSIZ);
	histi = (histi + 1) % HISTLEN;
}
