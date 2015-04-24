.so mac
.th WANDER 4/80
.sh NAME
wander \*- Fantasy story tool
.sh SYNOPSIS
.mc |
.bd wander
[ world ] [
.bd -r[savefile]
] [
.bd -t\c
#
]
.mc
.sh DESCRIPTION
.it Wander
is a tool for writing non-deterministic fantasy "stories".
The product is a story whose unfolding is affected by decisions
made by the "reader".
.sp
The optional world argument is described below under "WANDER FILES".
.sp
The
.bd -r
flag allows continuation from a previously "saved" wander.
If the form "-rsavefile" is used the saved wander is restored from
"savefile".  "-r" by itself refers to the default save file for the
particular wander world, (e.g. "a3.save" for the world "a3").
.sp
The
.bd -t\c
# flag turns on tracing of action matching; `#' represents a number
whose value controls the amount of diagnostic output produced.
.sh COMMANDS
Wander includes several built-in commands.  Aside from recognizing the
standard compass directions (and their abbreviations),
.mc |
and "up" and "down",
.mc
the following commands are recognized:
.nf

inventory   list objects being carried
take        pick up specified object
drop        drop specified object
quit        stop playing
save        quit & save environment for later continuation
.mc |
restore     restore saved environment
.mc
look        print the long description of the current location
.mc |
init        read new .wrld & .misc files, (switch worlds)
.mc
 The following debugging verbs only work if you own the files
~snoop      give a condensed list of possible actions
~goto m     move, magically, to location number "m"
~goto m.n   move to location "m" and put it in state "n"
~vars       print a list of all non-zero variables
~version    print miscellaneous parameter information

.fi
.mc |
Whenever the word "all" is encountered as the second recognized
word of user input on a line it will be macro expanded.  This expansion
replaces the word "all" with each object in the current location including
objects being carried.  E.g. "drop all" may expand to "drop keys; drop net;
drop leaflet".
.mc
.sh "WANDER FILES"
The
.it world
argument is used to specify which fantasy "world" you wish to
explore.
Each world is described by a
.mc |
minimum of two files.
.mc
One file, (with the extension
.it .wrld),
contains all location-specific information, (long and short descriptions,
local action keywords, conditions and results); another file,
(with the extension
.it .misc),
contains all the global information, (initial message,
.mc |
word synonyms, initial object locations and characteristics,
.mc
global action keywords, conditions and results).
.s3
These two files have identical first parts of their names, e.g. if you
wish your world to be called "oz", you would name the files
.it oz.wrld
and
.it oz.misc
respectively and you would execute "wander oz".
If no
.it world
is given when executing
.it Wander,
the default
.it world
.mc |
"a3" is used, (i.e. the files are "a3.wrld" and "a3.misc").
.mc
.sp
A third, optional file related to a particular world is the
.it .mon
file.  If such a file exists, (e.g.
.it oz.mon),
a record of each user's commands will be kept there.
If no such file exists, but a file named "/sys/games/.../wand/wand.mon",
(this name can be changed in "wandglb.c"),
does exist then the record of user's commands will be kept there.
If neither file exists or if the symbol "MONITOR" is defined as "0"
in wanddef.h no record will be kept.
This record is often useful in two ways;
it allows the author to see how other users respond to his/her world
and it allows the author to type notes and suggestions as commands
while running
.it Wander
and later use these notes while modifying the
.it .wrld
and
.it .misc
files.
.sp
When a user "saves" his/her environment it is saved in a file
.mc |
whose name is the world name followed by ".save", ("a3.save"
for the default world, "a3"),
in the current working directory.
.mc
.sh "FILE PROTECTION"
Typically, the author of a
.it Wander
world will want the
.it .misc
and
.it .wrld
files to be unreadable by others except through
.it Wander
itself.  A simple way to do this is to generate, for each world, a small
C program that runs setuid to the owner of the world and execs
.it Wander.
For example, if "smith" has files /u/smith/oz.misc and /u/smith/oz.wrld,
the following program would suffice:
.nf

main()
{
     execl("/usr/games/wander", "oz", "/u/smith/oz", 0);
}

.fi
This program would be compiled; a.out moved to "oz" in some convenient
location and "chmod 4755 egypt" would be done.
At this point running "oz" would make the user effectively "smith"
and thereby allow the oz files, (which would have mode 0600), to be read.
.s3
It was decided that using encryption on the files was too weak a defense
against a dedicated world-cracker and too much overhead to be worth the
effort.
.sh VARIABLES
.it Wander
provides 128 variables (numbered 0 through 127) which can contain numeric
values of -32768 through 32767.
The variables numbered 0 through 99 are general purpose and may be used
freely; variables 100 through 127 are set aside for pre-defined uses,
(see below).
These variables are referenced with two syntaxes.
Some constructions require the specification of a variable number;
for instance, "v=6.3" is used to set variable 6 to the value 3.
Note that here the "6" is automatically a variable number, while
the "3" is a simple number.
"\*(VC6\*(VC" would be used to specify substitution of the value
contained in variable 6.
Thus, if variable 3 contains 5 and variable 6 contains 2,
\*(VC3\*(VC is equal to 5, and \*(VC6\*(VC is equal to 2.
Moreover:
.nf

"v=7.\*(VC3\*(VC" will set variable 7 to 5
"v+\*(VC6\*(VC.4" will add 4 to variable 2
"v?\*(VC6\*(VC.\*(VC3\*(VC" will test whether variable 2 is equal to 5.
"m=The answer is \*(VC3\*(VC." will print out as "The answer is 5."

.fi
.sp
The special variables and their mnemonic names are:
.nf
CUR_LOC     100     current location
PREV_LOC    101     previous location
INP_W1      102     hash of first recognized word in inp comm
INP_W2      103     hash of second recog word from inp comm
INP_W3      104     hash of third recog word from inp comm
INP_W4      105     hash of fourth recog word from inp comm
INP_W5      106     hash of fifth recog word from inp comm
INP_WC      107     number of words in input comm
NUM_CARRY   108     # of things being carried
MAX_CARRY   109     # of thing poss. to carry at once
NOW_YEAR    110     year of decade (0:99)
NOW_MONTH   111     month of year (1:12)
NOW_DOM     112     day of month (1:31)
NOW_DOW     113     day of week (0:6)
NOW_HOUR    114     hour of day (0:23)
NOW_MIN     115     minute of hour (0:59)
NOW_SEC     116     second of minute (0:59)
NOW_ET      117     elapsed time in Wander (seconds)
.mc |
BREVITY     118     brevity of place descriptions
LOC_VIEW    119     location description override
OBJ_VIEW    120     object description override
INP_N1      121     numeric value of first number from inp comm
INP_N2      122     numeric value of first number from inp comm
NUM_MOVES   123     number of "moves"
NUM_PLACES  124     number of "places" visited
	    125     reserved
	    126     reserved
	    127     reserved
Note that the actual variable numbers used by these
may vary in later releases but the
mnemonic names should not -- so use the mnemonics.

.mc
.fi
A common use of these is the following action:
.nf

back  v=CUR_LOC.%PREV_LOC%  m="Hmm, I think we came this way..."

.fi
Note that "\*(VCINP_W1\*(VC", "\*(VCINP_W2\*(VC", etc. are replaced by the
first, second, etc. recognized input words when used in a text message.
Also note that, if you are carrying 5 things, "v?NUM_CARRY.5" will be true
while "v?\*(VCNUM_CARRY\*(VC.5" will only be true if variable 5 is equal to 5.
.mc |
The two variables INP_N1 and INP_N2 are set to the values of the first
and second "numbers" input in a command by the user.
The corresponding word entries are set to the symbols "N1" and "N2"
so that the following constructions work:
.nf

"take N1 apples"  v+4.\*(VCINP_N1\*(VC  m="You now have \*(VC4\*(VC apples."
"add N1 and N2"   v=22.\*(VCINP_N1\*(VC v+22.\*(VCINP_N2\*(VC m="Sum is \*(VC22\*(VC"

.mc
.fi
The ".wrld" documentation describes the uses and syntax of variables in
greater detail.
.sh LOCATIONS
.it Wander
is usually set up for 256 to 512 numbered locations,
(rooms, chambers, whatever),
but this limit can be changed by a parameter in the "wanddef.h" file.
Again, the ".wrld" documentation describes these further.
.sh "SYNTACTIC CONVENTIONS"
In reading both the .misc and .wrld files
.it Wander
uses the following conventions.
In order to allow reasonable formatting of lines the following conventions
are implemented:
.mc |
.nf

\*(EC<LF> is completely ignored,
\*(ECn is replaced by <LF>,
\*(ECb is replaced by a <BS>,
\*(ECt is replaced by <HT>,
\*(ECr is replaced by <CR>,
\*(EC" is replaced by " (not considered a "quote"),
\*(EC<SP> is replaced by <SP>

.fi
In reading the file, each unescaped tab is
replaced by a single space, and quotes are stripped off.
The resulting <SP> and <LF>
codes created by the escape sequence using `\*(EC'
are turned into non-delimiting <SP> or <LF> codes.
In addition, all <LF>, <HT> and <SP> codes that appear in a quoted
sequence of characters are treated as non-delimiters,
(`"press red button"' and `press\*(EC red\*(EC button'
generate the same result),
.mc
Otherwise, <SP> is
used as the field delimiter, and <LF> is used as the line delimiter.
Throughout the doc files the field delimiter may be described as
"spaces or tabs" indicating that one or more of these characters
may be used to delimit fields.  In some places "<SEP>" is used, meaning
a separator, either spaces or tabs.
.sh CAVEAT
Peter found lots of time to write and play
.it Wander;
however, when it came time to write documentation he found he was pressed for
time (so what's new?).  As a result, this documentation is of the bare-bones
variety and probably loaded with errors.  Sorry.
.sh BUGS
Ho ho ho.
.sh "SEE ALSO"
wrld.doc, misc.doc
.CC
