#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<unistd.h>
#include	"wanddef.h"
/*
**      WANDER -- Non-deterministic fantasy story tool
** Copyright (c) 1978 by Peter S. Langston - New  York,  N.Y.
*/

static	char    *whatwand = "@(#)wand1.c	1.6 2/23/85 -- (c) psl 1978";

static	char    *wand_h =   H_SCCS;

char    ungotlin[BUFSIZE];
int     curstate, actrace, owner;
int     vrbquit, vrbsave, vrbrest, vrbtake, vrbdrop, vrbinve;
int     vrblook, vrbinit, vrbstar, vrbhist;
int     vrbsnoop, vrbgoto, vrbvars, vrbvers;
int     lstdirvrb;
int     objnum1, objnum2, objall;
FILE	*fpungot, *monfp;
long    lbegaddr;		    /* addr of begin of last getlin() line */
long    ungotaddr;		  /* addr of begin of last ungetlin() line */

uid_t	myruid() { return(getuid()); }	// normally in glib
uid_t	myeuid() { return(geteuid()); }	// normally in glib

extern	long    wtell();
extern	FILE	*fopen(), *wopen();

int
main(argc, argv)
char     *argv[];
{
	char *cp;
	int i, beforeloc;
	long t_then, t_now;
	struct tm *tp;

        setup(argc, argv);
	if (myruid() == myeuid())
	    owner = 1;
	else
	    owner = 0;
	time(&t_then);
        for (i = COM_DESC; ; ) {
	    curstate = locstate[var[CUR_LOC]];
	    if (i & COM_DESC)
		prloc();
	    do {
		cp = getcom(i);
	    } while (*cp == '\n' || *cp == '\0');
	    boswell(cp);		// record the history
	    time(&t_now);
	    tp = localtime(&t_now);
	    var[NOW_YEAR] = tp->tm_year;
	    var[NOW_MONTH] = tp->tm_mon + 1;
	    var[NOW_DOM] = tp->tm_mday;
	    var[NOW_DOW] = tp->tm_wday;
	    var[NOW_HOUR] = tp->tm_hour;
	    var[NOW_MIN] = tp->tm_min;
	    var[NOW_SEC] = tp->tm_sec;
	    var[NOW_ET] += t_now - t_then;
	    t_then = t_now;
            if (monitor) {
		if (monloc != var[CUR_LOC] || monstate != curstate)
		    fprintf(monfp, "\n#%d.%d  ",
		     monloc = var[CUR_LOC], monstate = curstate);
                fprintf(monfp, "%s; ", cp);
            }
	    beforeloc = var[CUR_LOC];
            i = carry_out(cp);
	    if (var[CUR_LOC] != beforeloc)
		var[PREV_LOC] = beforeloc;
            if (i & COM_DESC)
		var[NUM_MOVES]++;
	}
}

void
prloc()		// print the location description
{
	char *cp, buf[128];
	int c_loc, c_state;
	long sd_addr, ld_addr;
        struct wrdstr *op;
	static int locview;
	static long lvsd_addr, lvld_addr;

        c_loc = var[CUR_LOC];
        c_state = locstate[c_loc];
        if (place.p_loc != c_loc || place.p_state != c_state)
            get_loc(c_loc, c_state);
	if (var[LOC_VIEW]) {                /* if seeing specific location */
	    if (locview != var[LOC_VIEW]) {                     /* find it */
		locview = var[LOC_VIEW];
		get_loc(locview, locstate[locview]);
		lvsd_addr = place.p_sdesc;
		lvld_addr = place.p_ldesc;
		get_loc(c_loc, c_state);
	    }
	    sd_addr = lvsd_addr;
	    ld_addr = lvld_addr;
	} else {
	    sd_addr = place.p_sdesc;
	    ld_addr = place.p_ldesc;
	}
	if (locseen[c_loc]++ == 0)
	    var[NUM_PLACES]++;
	cp = 0;
	if (var[BREVITY] != 0) {
	    if (locseen[c_loc] == 1
	     || (var[BREVITY] > 0 && locseen[c_loc] > var[BREVITY])) {
		locseen[c_loc] = 1;
		cp = msgpara(wfp, ld_addr);
	    }
	}
	if (cp == 0)
	    cp = msglin(wfp, sd_addr);
	printf("%s\n", cp);
	if (var[OBJ_VIEW] != 0)
	    return;
	for (op = &wrds[1]; op->w_word; op++) {
	    if (op->w_word != listunused
	     && op->w_syn == 0
	     && op->w_loc == c_loc)
		printf("%s.\n",
		 objdesc((char *) 0, (char *) 0, op, " here", buf, sizeof buf));
	}
}

char *
getcom()
{
        char *bp;
        static char buf[128], *ep;

        if (ep == 0) {
            bp = buf;
	    if (getlin(stdin, bp) == 0)
                bp = "quit";
        } else
            bp = ep;
        for (ep = bp; *ep && *ep != ';'; ep++)
            if (*ep >= 'A' && *ep <= 'Z')
                *ep |= 040;
        if (*ep == ';')
            *ep++ = '\0';
        else
            ep = 0;
        while (*bp == ' ')
            bp++;
        return(bp);
}

int
carry_out(com)
char     *com;
{
	char *cp;
	char junk[BUFSIZE], *jp;
	int i, j, retval;
        struct actstr *ap;

	retval = COM_UNREC;
	var[INP_WC] = wdparse(com, &var[INP_W1], &var[INP_N1], 0);
	if (var[INP_W2] == objall) {
	    for (i = 1; i < maxwrds && wrds[i].w_word != 0; i++) {
		if ((wrds[i].w_loc == var[CUR_LOC] && var[INP_W1] != vrbdrop)
		 || (wrds[i].w_loc < 0 && var[INP_W1] != vrbtake)) {
		    if (wrds[i].w_flg & W_DONLY)
			i++;
		    sprintf(junk, "%s %s", wrds[var[INP_W1]].w_word, wrds[i].w_word);
		    printf("%s --- ", deparity(junk));
		    retval |= carry_out(junk);
		}
	    }
	    return(retval);
	}
	if (var[INP_WC] == 0) {
	    printf("I don't understand \"%s\" ...\n", com);
	    return(retval);
	}
	if (actrace > 1)
	    printf("pre-actions\n");
	for (ap = pre_acts; ap->a_wrd[0] != 0; ap++) {
	    retval |= check_act(ap);                           /* pre-acts */
            if (retval & COM_COMPLETE)
                return(retval);
        }
	if (actrace > 1)
	    printf("local actions\n");
	for (ap = place.p_acts; ap->a_wrd[0] != 0; ap++) {
	    retval |= check_act(ap);                         /* local acts */
            if (retval & COM_COMPLETE)
                return(retval);
        }
	if (actrace > 1)
	    printf("post-actions\n");
	for (ap = post_acts; ap->a_wrd[0] != 0; ap++) {
	    retval |= check_act(ap);                          /* post acts */
            if (retval & COM_COMPLETE)
                return(retval);
        }
	if (actrace > 1)				// built-in actions
	    printf("built-in actions\n");

	if (var[INP_W1] == vrbdrop) {
	    j = 0;
	    for (i = 1; i < maxwrds && wrds[i].w_word != 0; i++) {
		if (oneof(i, &var[INP_W2]) && wrds[i].w_loc < 0) {
		    wrds[i].w_loc = var[CUR_LOC];
		    --var[NUM_CARRY];
		    printf("%s dropped.\n",
		     objdesc("", "", &wrds[i], "", junk, sizeof junk));
		    j++;
		}
	    }
	   if (j == 0)
		printf("I'd like to %s, but ...\n", com);
            return(COM_DONE);
        }
	if (var[INP_W1] == vrbgoto && owner) {
	    cp = movchars(com, com, fldels);
	    var[CUR_LOC] = atoi(cp);
	    while (*cp)
		if (*cp++ == dotchar) {
		    locstate[var[CUR_LOC]] = atoi(cp);
		    break;
		}
	    printf("Goto loc %d which is in state %d\n", var[CUR_LOC],
	     locstate[var[CUR_LOC]]);
            return(COM_DESC | COM_DONE);
	}
	if (var[INP_W1] == vrbhist) {
	    i = 1;
	    if (var[INP_WC] == 2)
		i = HISTLEN - var[INP_N1];
	    for (; i < HISTLEN; i++) {
		cp = history[(histi + i) % HISTLEN];
		if (*cp)
		    printf("%s%s", cp, i < (HISTLEN - 1)? "; " : "\n");
	    }
            return(COM_DONE);
        }
	if (var[INP_W1] == vrbinit) {
	    restart(movchars(com, com, fldels));
            return(COM_DESC | COM_DONE);
        }
	if (var[INP_W1] == vrbinve) {
            inventory();
            return(COM_DONE);
        }
	if (var[INP_W1] == vrblook) {
	    locseen[var[CUR_LOC]] = 0;
	    --var[NUM_PLACES];
            return(COM_DONE | COM_DESC);
        }
	if (var[INP_W1] == vrbquit)
	    quit(QUIT_QUIET);
	if (var[INP_W1] == vrbrest) {
	    restore(movchars(com, com, fldels), FRESTORE);
            return(COM_DESC | COM_DONE);
	}
	if (var[INP_W1] == vrbsave) {
	    save(movchars(com, com, fldels));
            return(COM_DESC | COM_DONE);
	}
	if (var[INP_W1] == vrbsnoop && owner) {
	    printf("<==== loc:%d  state:%d ====>", place.p_loc, place.p_state);
	    printf("   %s\n", msglin(wfp, place.p_sdesc));
	    printf("%s", msgpara(wfp, place.p_ldesc));
	    for (ap = place.p_acts; ap->a_wrd[0] != 0; ap++) {
		for (i = 0; i < 5; i++)
		    if (ap->a_wrd[i] != NO_WORD)
			printf(" %s", deparity(wrds[ap->a_wrd[i]].w_word));
		printf(" <%d> ", ap->a_rloc);
		if (ap->a_msgaddr) {
		    wseek(ap->a_msgfp, ap->a_msgaddr, 0);
		    getlin(ap->a_msgfp, junk);
		    for (cp = junk; *cp; cp = jp) {
			jp = movchars(cp, cp, fldels);
			if (cp[0] == 'm' && cp[1] == '=')
			    printf("%s", deparity(cp));
		    }
		}
		printf("\n");
	    }
	    return(COM_DONE);
	}
	if (var[INP_W1] == vrbtake) {
	    j = 0;
	    for (i = 1; i < maxwrds && wrds[i].w_word != 0; i++) {
		if (oneof(i, &var[INP_W2])
		 && (wrds[i].w_loc == var[CUR_LOC] || wrds[i].w_loc < 0)) {
		    takeobj(i);
		    j++;
		}
	    }
	    if (j == 0)
		printf("Can't %s\n", com);
	    return(COM_DONE);
        }
	if (var[INP_W1] == vrbvars && owner) {
	    for (i = 0; i < maxvars; i++)
		if (var[i])
		    printf("var[%2d] = %d\n", i, var[i]);
	    return(COM_DONE);
	}
	if (var[INP_W1] == vrbvers && owner) {
	    printf("Current Wander version:\n");
	    printf("MAXLOCS:%d\tMax # of locations possible.\n", maxlocs);
	    printf("MAXACTS:%d\tMax # of actions per location.\n", maxacts);
	    printf("MAXFIELDS:%d\tMax # of fields per action.\n", maxfields);
	    printf("BUFSIZE:%d\tSize of long descriptions, etc.\n", BUFSIZE);
	    printf("PATHLENGTH:%d\tMax length of file path names.\n", PATHLENGTH);
	    printf("MAXWRDS:%d\tMax # of words Wander will remember.\n",
	     maxwrds);
	    printf("MAXINDEX:%d\tMax # of location/states, total.\n",
	     maxndx);
	    printf("MAXPREACTS:%d\tMax # of pre actions.\n", maxpreacts);
	    printf("MAXPOSTACTS:%d\tMax # of post actions.\n", maxpostacts);
	    return(COM_DONE);
	}

	if (retval & COM_DONE)
	    return(retval);
	if (retval & COM_NDOBJ) {
	    printf("%s what?\n", wrds[var[INP_W1]].w_word);
	    return(retval | COM_COMPLETE);
	}
	printf("You can't do that ");
        if (retval & COM_RECOG)
	    printf("now.\n");
	else
	    printf("here.\n");
        return(COM_DONE);
}

int
check_act(actp)                               /* if the action fits, do it */
struct   actstr   *actp;
{
        struct actstr *ap;
	char *newwrld;
	int i, retval, fld1v, fld2v;
	long addr;
        struct fieldstr *fp;

        retval = COM_UNREC;
			    /* first test to see if this action is a match */
        ap = actp;
	for (i = 0; i < MAXACTWDS && ap->a_wrd[i] != NO_WORD; i++) {
	    if (ap->a_wrd[i] != vrbstar
	     && oneof(ap->a_wrd[i], &var[INP_W1]) == 0) {
		if (i == 0)
		    return(retval);
		else
		    return(retval |= COM_NDOBJ);
	    }
	}
	if (ap->a_wrd[i] != vrbstar)
            retval |= COM_RECOG;
	newwrld = 0;
	for (fp = ap->a_field; fp < &ap->a_field[maxfields]; fp++) {
	    if (fp->f_type == 0)
		break;
	    fld1v = fp->f_fld1;
	    if (fp->f_type & FLD1_VAR)
		fld1v = var[fp->f_fld1];
	    fld2v = fp->f_fld2;
	    if (fp->f_type & FLD2_VAR)
		fld2v = var[fp->f_fld2];
	    switch (fp->f_type & TYPEONLY) {
            case FT_OBJ:
		if (obj_at(fld1v, fld2v) == 0)
                    return(retval);
                break;
            case FT_NOBJ:
		if (obj_at(fld1v, fld2v) == 1)
                    return(retval);
                break;
            case FT_TOOL:
		if (wrds[fld1v].w_loc >= 0
                 || (fld2v != 0 && var[CUR_LOC] != fld2v))
                    return(retval);
                break;
            case FT_NTOOL:
		if (wrds[fld1v].w_loc < 0
                 && (fld2v == 0 || var[CUR_LOC] == fld2v))
                    return(retval);
                break;
            case FT_STATE:
                if (locstate[fld1v] != fld2v)
                    return(retval);
                break;
            case FT_NSTATE:
                if (locstate[fld1v] == fld2v)
                    return(retval);
                break;
            case FT_EVAR:
                if (var[fld1v] != fld2v)
                    return(retval);
                break;
            case FT_NVAR:
                if (var[fld1v] == fld2v)
                    return(retval);
                break;
            case FT_GVAR:
                if (var[fld1v] <= fld2v)
                    return(retval);
                break;
            case FT_LVAR:
                if (var[fld1v] >= fld2v)
                    return(retval);
                break;
            case FT_ODDS:
                if ((rand() >> 3) % 100 >= fld1v)
                    return(retval);
                break;
	    case FT_EBIN:
		if (locseen[fld1v] != fld2v)
                    return(retval);
                break;
	    case FT_NBIN:
                if (locseen[fld1v] == fld2v)
                    return(retval);
                break;
	    case FT_GBIN:
                if (locseen[fld1v] <= fld2v)
                    return(retval);
                break;
	    case FT_LBIN:
                if (locseen[fld1v] >= fld2v)
                    return(retval);
                break;
            }
        }
					/* it's a match; carry out results */
	if (actrace) {
	    printf("Carry out: ");
	    for (i = 0; i < 5; i++)
		if (ap->a_wrd[i] != NO_WORD)
		    printf("%-8.8s ", wrds[ap->a_wrd[i]].w_word);
	    printf("%-3d ", ap->a_rloc);
	}
        for (fp = ap->a_field; fp < &ap->a_field[maxfields]; fp++) {
	    if (fp->f_type == 0)
		break;
	    fld1v = fp->f_fld1;
	    fld2v = fp->f_fld2;
	    if (actrace > 1)
		printf("t:0%o,f1:%d,f2:%d=>", fp->f_type, fld1v, fld2v);
	    if (fp->f_type & FLD1_VAR)
		fld1v = var[fp->f_fld1];
	    if (fp->f_type & FLD2_VAR)
		fld2v = var[fp->f_fld2];
	    if (actrace)
		printf("t:0%o,f1:%d,f2:%d ",
		 fp->f_type & TYPEONLY, fld1v, fld2v);
	    switch (fp->f_type & TYPEONLY) {
            case FR_SSTATE:
                locstate[fld1v] = fld2v;
                goto states;
            case FR_ISTATE:
                locstate[fld1v] += fld2v;
                goto states;
            case FR_DSTATE:
                locstate[fld1v] -= fld2v;
        states:
                if (fld1v == var[CUR_LOC])
                    curstate = locstate[var[CUR_LOC]];
                if (locseen[fld1v])
                    locseen[fld1v] = ldescfreq;
                break;
            case FR_SVAR:
                var[fld1v] = fld2v;
                break;
            case FR_IVAR:
		var[fld1v] = var[fld1v] + fld2v;
                break;
            case FR_DVAR:
                var[fld1v] -= fld2v;
                if (var[fld1v] < 0)
                    var[fld1v] = 0;
                break;
	    case FR_MVAR:
		var[fld1v] *= fld2v;
		break;
	    case FR_QVAR:
		var[fld1v] /= fld2v;
		break;
            case FR_LOBJ:
		if (obj_at(fld1v, fld2v) == 1) {
		    if (wrds[fld1v].w_loc < 0)
			--var[NUM_CARRY];
		    wrds[fld1v].w_loc = 0;
                }
                break;
            case FR_GOBJ:
		if (wrds[fld1v].w_loc < 0)
		    --var[NUM_CARRY];
                if (fld2v == 0)
		    wrds[fld1v].w_loc = var[CUR_LOC];
                else
		    wrds[fld1v].w_loc = fld2v;
                break;
            case FR_LTOOL:
		if (wrds[fld1v].w_loc < 0
		 && (fld2v == 0 || fld2v == var[CUR_LOC])) {
		    wrds[fld1v].w_loc = 0;
		    --var[NUM_CARRY];
		}
                break;
            case FR_GTOOL:
		if (wrds[fld1v].w_loc >= 0
		 && (fld2v == 0 || fld2v == var[CUR_LOC])) {
		    var[NUM_CARRY]++;
		    wrds[fld1v].w_loc = -1;
                }
                break;
	    case FR_CSUB:
		var[INP_W1] = fld1v;
		var[INP_W2] = fld2v;
		var[INP_WC] = fld2v == 0? 1 : 2;
		break;
	    case FR_WORLD:
		addr = ((long) fp->f_fld1 & 0177777) | (fp->f_fld2 << 16);
		newwrld = msglin(ap->a_msgfp, addr);
		break;
            case FR_SBIN:
                locseen[fld1v] = fld2v;
                break;
            case FR_IBIN:
		locseen[fld1v] = locseen[fld1v] + fld2v;
                break;
            case FR_DBIN:
                locseen[fld1v] -= fld2v;
                if (locseen[fld1v] < 0)
                    locseen[fld1v] = 0;
                break;
            }
        }
	if (actrace)
	    printf("\n");
	if (ap->a_msgaddr)
	    printf("%s\n", msglin(ap->a_msgfp, ap->a_msgaddr));
	if (newwrld != 0)
	    restart(newwrld);
        if (ap->a_rloc < 0)
            quit(ap->a_rloc);
        if (ap->a_rloc > 0)
            var[CUR_LOC] = ap->a_rloc;
	if (ap->a_rcont == 1)                        /* ... style continue */
	    return(retval | COM_DONE | COM_DESC);
	else if (ap->a_rcont == 2)                   /* ,,, style continue */
	    return(retval);
	else                                                /* no continue */
	    return(retval | COM_DONE | COM_DESC | COM_COMPLETE);
}

void
get_loc(loc, state)
{
        char *bp;
	char buf[BUFSIZE];
	long locaddr;
        struct actstr *ap;

        if (place.p_loc == loc && place.p_state == state)
            return;
        if (loc >= maxlocs) {
	    fprintf(stderr, "Too many locations, limit is %d\n", maxlocs);
            exit(1);
        }
        place.p_loc = loc;
        place.p_state = state;
	place.p_sdesc = place.p_ldesc = 0;
        ap = &place.p_acts[0];
				/* get current state description & actions */
	if ((locaddr = getndx(loc, state)) < 0) {
	    if (state != BASESTATE && getndx(loc, BASESTATE) >= 0)
		goto base_case;
	    fprintf(stderr, "Non-existent loc/state %d/%d\n", loc, state);
	    exit(1);
	}
        wseek(wfp, locaddr, 0);
        getlin(wfp, buf);
        if (atoi(&buf[1]) != loc) {
	    fprintf(stderr, "Looking for loc #%d.%d at %ld found <%s>\n",
	     loc, state, locaddr, buf);
            exit(1);
        }
	bp = movchars(buf, buf, fldels);
	if (*bp > ' ')
	    place.p_sdesc = locaddr + 1;
	place.p_ldesc = wtell(wfp);
	getpara(wfp, buf);
	if (buf[0] == '\0')
	    place.p_ldesc = 0;
	for (ap = place.p_acts; getlin(wfp, buf) > 0 && buf[0] == fieldelim; )
	    ap = code_act(place.p_acts, maxacts, ap, buf, wfp, lbegaddr);
base_case:                         /* get base state description & actions */
	if (state == BASESTATE || (locaddr = getndx(loc, BASESTATE)) < 0)
		return;
	wseek(wfp, locaddr, 0);
        getlin(wfp, buf);
        if (atoi(&buf[1]) != loc) {
	    fprintf(stderr, "Looking for #%d.0 at %ld found <%s>\n",
	     loc, locaddr, buf);
            exit(1);
        }
	if (place.p_sdesc == 0) {
	    bp = movchars(buf, buf, fldels);
	    place.p_sdesc = locaddr + 1;
        }
	if (place.p_ldesc == 0)
	    place.p_ldesc = wtell(wfp);
	getpara(wfp, buf);
	while (getlin(wfp, buf) > 0 && buf[0] == fieldelim)
	    ap = code_act(place.p_acts, maxacts, ap, buf, wfp, lbegaddr);
}

void
setup(argc, argv)                   /* initial setup when Wander first run */
char     *argv[];
{
	int rflag, n, i, lastrw;

	signal(1, quit);
	signal(2, quit);
	var[CUR_LOC] = var[PREV_LOC] = 1;                      /* defaults */
	var[NUM_CARRY] = 0;
	var[MAX_CARRY] = max_carry;
	var[BREVITY] = ldescfreq;
	rflag = n = 0;
        while (--argc > 0) {
            if (argv[argc][0] == '-') {
                switch (argv[argc][1]) {
		case 'r':
		    restore(&argv[argc][2], FMAINRES);
		    rflag++;
                    break;
                case 't':
		    actrace = atoi(&argv[argc][2]);
                    break;
                default:
                    fprintf(stderr, "?%s? bad switch\n", argv[argc]);
syntax:
                    fprintf(stderr, "Usage: %s [-r[file]] [-t[#]] [world]\n",
                     argv[0]);
                }
            } else {
		if (n++) {
		    fprintf(stderr, "More than 1 world file?\n");
		    goto syntax;
		}
		cpyn(curname, argv[argc], sizeof(curname) - 1);
	    }
        }
	// modify the w_word and w_syn fields of wrds from wandglb.c
	lastrw = 1;
	for (i = 1; wrds[i].w_word; i++) {
	    wrds[i].w_word = store(wrds[i].w_word);	// put the word in storebuf
	    if (wrds[i].w_syn)
		wrds[i].w_syn = lastrw;
	    else
		lastrw = i;
	}
	if (rflag == 0 && get_files(curname, FMAINNEW) == -1)
            exit(2);
        srand(0);
        curstate = locstate[var[CUR_LOC]];
	lstdirvrb = which("nw", wrds);   /* see comment on "nw" in wandglb */
	vrbdrop = which("drop", wrds);
	vrbhist = which("history", wrds);
	vrbinit = which("init", wrds);
	vrbinve = which("inventory", wrds);
	vrblook = which("look", wrds);
	vrbquit = which("quit", wrds);
	vrbrest = which("restore", wrds);
	vrbsave = which("save", wrds);
	vrbtake = which("take", wrds);
	vrbsnoop = which("~snoop", wrds);
	vrbgoto = which("~goto", wrds);
	vrbvars = which("~vars", wrds);
	vrbvers = which("~version", wrds);
	vrbstar = which("*", wrds);
	objnum1 = which("N1", wrds);
	objnum2 = which("N2", wrds);
	objall = which("all", wrds);
}

struct  actstr *
code_act(actbuf, maxacts, actp, buf, ifp, baddr)
struct  actstr *actbuf, *actp;
char    *buf;
FILE    *ifp;
long    baddr;
{
        char *bp;
	char wdbuf[128], *bufp;
	int w[5];
        struct actstr *ap;
        struct fieldstr *fp;
	struct actstr actemp;

#define   LR(a,b)     ((a << 8) | b)

	/* strip off and save word(s) */
	for (bp = buf; *bp == ' ' || *bp == '\t'; bp++);
	bufp = movchars(bp, wdbuf, fldels);
	/* encode the fields into actemp */
	ap = &actemp;
	ap->a_rcont = ap->a_rloc = 0;
	ap->a_msgfp = ifp;
	ap->a_msgaddr = 0;
	for (fp = ap->a_field; fp < &ap->a_field[maxfields]; fp++)
	    fp->f_type = fp->f_fld1 = fp->f_fld2 = F_VOID;
	fp = ap->a_field;
	for (bp = bufp; *bp; ) {
	    bufp = bp;
	    bp = movchars(bp, bp, fldels);          /* grab one field */
	    if ((bufp[0] >= '0' && bufp[0] <= '9')
	     || bufp[0] == '-') {
		ap->a_rloc = atoi(bufp);                  /* result loc */
		continue;
	    }
	    if (bufp[0] == varchar) {               /* variable result loc */
		ap->a_rloc = var[atoi(&bufp[1])];   /* NOTE: evaluated now */
		continue;
	    }
	    if (bufp[0] == 'm' && bufp[1] == '=') {        /*message field */
		ap->a_msgaddr = baddr;
		continue;
	    }
	    if (bufp[0] == '.' && bufp[1] == '.' && bufp[2] == '.') {
		ap->a_rcont = 1;
		continue;
	    }
	    if (bufp[0] == ',' && bufp[1] == ',' && bufp[2] == ',') {
		ap->a_rcont = 2;
		continue;
	    }
	    if (fp >= &ap->a_field[maxfields]) {        /* check for space */
		fprintf(stderr, "Too many fields, <%s> lost\n", bufp);
		continue;
	    }
	    switch (LR(bufp[0], bufp[1])) {
	    case LR('o', '?'):
		atpair(FT_OBJ, &bufp[2], fp);
		break;
	    case LR('o', '~'):
		atpair(FT_NOBJ, &bufp[2], fp);
		break;
	    case LR('t', '?'):
		atpair(FT_TOOL, &bufp[2], fp);
		break;
	    case LR('t', '~'):
		atpair(FT_NTOOL, &bufp[2], fp);
		break;
	    case LR('s', '?'):
		dotpair(FT_STATE, &bufp[2], fp);
		break;
	    case LR('s', '~'):
		dotpair(FT_NSTATE, &bufp[2], fp);
		break;
	    case LR('v', '?'):
		dotpair(FT_EVAR, &bufp[2], fp);
		break;
	    case LR('v', '~'):
		dotpair(FT_NVAR, &bufp[2], fp);
		break;
	    case LR('v', '<'):
		dotpair(FT_LVAR, &bufp[2], fp);
		break;
	    case LR('v', '>'):
		dotpair(FT_GVAR, &bufp[2], fp);
		break;
	    case LR('c', '?'):
		fp->f_type = FT_ODDS;
		if (bufp[2] == varchar)
		    fp->f_type |= FLD1_VAR;
		fp->f_fld1 = atov(&bufp[2]);
		break;
	    case LR('b', '?'):
		dotpair(FT_EBIN, &bufp[2], fp);
		break;
	    case LR('b', '~'):
		dotpair(FT_NBIN, &bufp[2], fp);
		break;
	    case LR('b', '<'):
		dotpair(FT_LBIN, &bufp[2], fp);
		break;
	    case LR('b', '>'):
		dotpair(FT_GBIN, &bufp[2], fp);
		break;
	    case LR('s', '='):
		dotpair(FR_SSTATE, &bufp[2], fp);
		break;
	    case LR('s', '+'):
		dotpair(FR_ISTATE, &bufp[2], fp);
		break;
	    case LR('s', '-'):
		dotpair(FR_DSTATE, &bufp[2], fp);
		break;
	    case LR('v', '='):
		dotpair(FR_SVAR, &bufp[2], fp);
		break;
	    case LR('v', '+'):
		dotpair(FR_IVAR, &bufp[2], fp);
		break;
	    case LR('v', '-'):
		dotpair(FR_DVAR, &bufp[2], fp);
		break;
	    case LR('v', '*'):
		dotpair(FR_MVAR, &bufp[2], fp);
		break;
	    case LR('v', '/'):
		dotpair(FR_QVAR, &bufp[2], fp);
		break;
	    case LR('o', '-'):
		atpair(FR_LOBJ, &bufp[2], fp);
		break;
	    case LR('o', '+'):
		atpair(FR_GOBJ, &bufp[2], fp);
		break;
	    case LR('t', '-'):
		atpair(FR_LTOOL, &bufp[2], fp);
		break;
	    case LR('t', '+'):
		atpair(FR_GTOOL, &bufp[2], fp);
		break;
	    case LR('c', '='):
		fp->f_type = FR_CSUB;
		wdparse(&bufp[2], w, 0, 1);
		fp->f_fld1 = w[0];
		fp->f_fld2 = w[1];
		break;
	    case LR('w', '='):
		fp->f_type = FR_WORLD;
		fp->f_fld1 = (baddr + bufp - buf) & 0177777;
		fp->f_fld2 = (baddr + bufp - buf) >> 16;
		break;
	    case LR('b', '='):
		dotpair(FR_SBIN, &bufp[2], fp);
		break;
	    case LR('b', '+'):
		dotpair(FR_IBIN, &bufp[2], fp);
		break;
	    case LR('b', '-'):
		dotpair(FR_DBIN, &bufp[2], fp);
		break;

	    default:
		if (bufp[0] != '\0')
		    printf("??%s?? unrecognized action field in `%s'.\n",
		     bufp, buf);
		break;
	    }
	    if (fp->f_type != F_VOID)
		fp++;
	}

	/* replicate actemp for each alternate word set */
	for (ap = actp; wdbuf[0] != '\0'; ap++) {
            if (ap >= &actbuf[maxacts]) {
		fprintf(stderr, "More than %d actions ", maxacts);
		if (wtell(wfp) == 0)
		    fprintf(stderr, "[misc] <%s>\n", buf);
		else
		    fprintf(stderr, "[wrld %ld] <%s>\n", wtell(wfp), buf);
                exit(1);
            }
	    bytecopy(&actemp, ap, sizeof actemp);   /* copy actemp into ap */
	    bp = movchars(wdbuf, wdbuf, "|");     /* peel off one word set */
	    wdparse(wdbuf, ap->a_wrd, 0, 1);
	    cpyn(wdbuf, bp, sizeof(wdbuf) - 1); /* leave other word sets */
	}
	ap->a_wrd[0] = 0;
	return(ap);
}

int
get_files(name, flag)              /* do processing of .misc, .mon & .wrld */
char     *name;         /* flag indicates caller & current initializations */
{
	char *bp, buf[BUFSIZE], *cp;
	int i, j, iloc, syn, nextndx, flg, lastrw;
	long addr, lines;
        struct actstr *aps, *ap;
	time_t clock;

	if (monitor > 0 && monfp != (FILE *) NULL) {	// if there's already a monitor file
	    monitor = -1;
	    fprintf(monfp, "\n >> %s to %s\n",
	     flag == FRESTART? "init" : "restore", name);
	    monsav();                       /* close & unlink temp monfile */
	}
	if (flag == FRESTORE && strcmp(name, curname) != 0) {
	    printf("Restoring from a different world, (%s instead of %s)\n",
	     name, curname);
	    flag = FRESTART;                      /* need to do more inits */
	}
	if (flag != FRESTORE) {
	    stpcpy(stpcpy(locfile, name), ".wrld");
	    stpcpy(stpcpy(miscfile, name), ".misc");
	    stpcpy(stpcpy(monfile, name), ".mon");
	    if (mfp != (FILE *) NULL)
		wclose(mfp);
	    if ((mfp = wopen(miscfile, "r")) == (FILE *) NULL) {	// open misc
		stpcpy(stpcpy(stpcpy(locfile, stdpath), name), ".wrld");
		stpcpy(stpcpy(stpcpy(miscfile, stdpath), name), ".misc");
		stpcpy(stpcpy(stpcpy(monfile, stdpath), name), ".mon");
		if ((mfp = wopen(miscfile, "r")) == (FILE *) NULL) {
		    printf("Can't open miscfile ");
		    perror(miscfile);
		    return(-1);
		}
	    }
	    setbuf(mfp, mfbuf);                  /* to avoid stdio sbrk()s */
	    if (wfp != (FILE *) NULL)
		wclose(wfp);
	    if ((wfp = wopen(locfile, "r")) == (FILE *) NULL) {		// open wrld
		printf("Can't open wrldfile ");
		perror(locfile);
		return(-1);
	    }
	    setbuf(wfp, wfbuf);                  /* to avoid stdio sbrk()s */
	    if (monitor < 0 && (monfp = fopen(monfile, "r")) == (FILE *) NULL) {
//fprintf(stderr, "defmfile=%s\n", defmfile); //  apparently WANDPATH() fails on OS X
		cpyn(monfile, defmfile, sizeof(monfile));
		if ((monfp = fopen(monfile, "r")) == (FILE *) NULL)
		    monitor = 0;
	    }
	    if (monitor) {
		fclose(monfp);
		sprintf(tmonfil, "/tmp/w%d.mon", getpid());
	    }
	}
	if (monitor) {
	    monfp = fopen(tmonfil, "w");
	    setbuf(monfp, NULL);     /* avoid buffering which does sbrk()s */
	    clock = time(0);
	    fprintf(monfp, "\nruid:%d  euid:%d  tty%s  world:%s %16.16s",
	     myruid(), myeuid(), ttyname(2), locfile, ctime(&clock));
	}
	if (flag == FRESTORE || flag == FMAINRES)
	    goto get_done;

	// Read in and process the .misc file
	if (actrace)
	    printf(" process .misc\n");
        getpara(mfp, buf);                          // introductory paragraph
	if (flag == FMAINNEW)
            printf("%s", msgfmt(buf));
        getlin(mfp, buf);
        if (buf[0] != fieldelim) {                       // fieldelim is <HT>
	    fprintf(stderr, "Line following intro is not a header in %s [%ld]\n%s",
	     miscfile, wtell(mfp), buf);
            return(-1);
        }
	for (;;) {                    // a getlin() has always just been done
	    while (buf[0] == '\0' && getlin(mfp, buf));   /* blank, ignore */
            if (buf[1] == 'n') {                              /* <HT>notes */
		while (getlin(mfp, buf) && buf[0] != '\0');      /* ignore */
		if (getlin(mfp, buf) == 0)
                    break;
            } else if (buf[1] == 'p') {                        /* <HT>p... */
		if (buf[2] == 'r') {                    /* <HT>pre actions */
		    aps = pre_acts;
		    i = maxpreacts;
		} else {                               /* <HT>post actions */
                    aps = post_acts;
		    i = maxpostacts;
		}
                ap = aps;
		while (getlin(mfp, buf) && buf[0] != '\0')
		    ap = code_act(aps, i, ap, buf, mfp, lbegaddr);
		if (getlin(mfp, buf) == 0)
                    break;
            } else if (buf[1] == 'v' && buf[2] == 'a') {  /* <HT>variables */
		while (getlin(mfp, buf) && buf[0] != '\0') {
		    bp = movchars(buf, buf, fldels);
		    i = atoi(buf);
		    if (i == 0 && buf[0] != '0') {
			if ((i = which(buf, spvars)) < 0)
			    goto oops;
			i = spvars[i].w_loc;
		    }
		    j = atoi(bp);
		    if (i & ~0377) {
	oops:           fprintf(stderr,
			 "'%s %s' is an erroneous variable spec in %s [%ld]\n",
			 buf, bp, miscfile, wtell(mfp));
			return(-1);
		    }
		    var[i] = j;
		}
		if (getlin(mfp, buf) == 0)
                    break;
	    } else if (buf[1] == 'w') {                       /* <HT>words */
		lastrw = 0;                                 // last root word
		while (getlin(mfp, buf) && buf[0] != '\0') {
		    bp = movchars(buf, buf, fldels);
		    syn = atoi(bp);
		    if (syn > 0)  // this is a synonym for the last root word
			syn = lastrw;
		    bp = movchars(bp, bp, fldels);
		    iloc = atoi(bp);
		    bp = movchars(bp, bp, fldels);
		    flg = atoi(bp);
		    i = wrdadd(buf, syn, iloc, flg);
		    lastrw = syn? syn : i;
		    if (iloc < 0)
			var[NUM_CARRY]++;
		}
		if (getlin(mfp, buf) == 0)
		    break;
	    } else {                           // Unrecognized header line...
		fprintf(stderr, "%s is a header in %s [%ld]?\n",
		 buf, miscfile, wtell(mfp));
		return(-1);
	    }
        }

	/* now generate ndx of .wrld file */
	if (actrace)
	    printf(" process .wrld\n");
        for (i = maxndx; --i >= 0; )
            ndx[i].i_loc = -1;
        nextndx = 0;
	wseek(wfp, 0l, 0);
	addr = 0;
	for (lines = 0; fgets(buf, sizeof buf, wfp) != NULL; lines++) {
	    if (buf[0] == '#' && buf[1] >= '0' && buf[1] <= '9') {
		if (nextndx >= maxndx) {
		    fprintf(stderr,
		     "Too many total states (limit is %d) line %ld of .wrld\n",
		     maxndx, lines);
		    exit(1);
		}
		cp = &buf[1];
		i = atoip(&cp);
		ndx[nextndx].i_loc = i;
		locstate[i] = 0;
		if (*cp++ == '.')
		    ndx[nextndx].i_state = atoip(&cp);
		else
		    ndx[nextndx].i_state = BASESTATE;
		ndx[nextndx++].i_addr = addr;
	    }
	    for (cp = buf; *cp; cp++);
	    addr += cp - buf;
	}
	cpyn(curname, name, sizeof(curname) - 1);
get_done:
	wseek(wfp, 0l, 0);
        return(0);
}
