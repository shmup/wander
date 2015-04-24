.so mac
.th "WANDER .MISC SYNTAX" 4/80
.sh "SYNTAX OF THE .MISC FILE FOR WANDER"

The
.it .misc
file contains all the location-independent information to describe
a particular "world".
.s1
The file consists of a series of sections.  The sections are:
.nf

	Introductory message

.mc |
	Word definitions
.mc

	Pre-actions

	Post-actions

	Variables

	Notes

.fi
Any lines in the file that begin with a colon, ":" will be ignored
as comments by
.it Wander.
.sh "INTRODUCTORY MESSAGE"
The introductory message is printed out when wander is first run.
It is not printed out
when wander files are switched during a run, (possible with
.mc |
the "init" verb and the "w=" result field).
.s3
.mc
.mc |
The introductory message may be any number of lines in length,
(but is limited to a maximum of 1024 characters),
and may contain blank lines, but must not contain a line beginning with a tab
.mc
or space unless the entire message is enclosed in quotes.
If blank space is required at the beginning of a line in an unquoted
introductory message use an escaped space, "\*(EC ", to begin the line.
If you wish to include quote characters in the message they must also be
escaped.
.br
.mc |
.sh "WORD DEFINITIONS"
Word definitions usually fall into two conceptual categories, "verbs" and
"objects" and will be treated that way here although they may be intermingled
freely if the resulting confusion is unimportant.  Note that since there
may be any number of "words" sections it is common to have a section with
the header "words (verbs)" and one with "words (objects)".
.sh "WORD (VERB) DEFINITIONS"
.mc
This section is used for defining verbs that have several forms which should
be equivalent (synonyms).  It is not necessary to define verbs that don't
need synonyms, although it does no harm to do so.
.s3
.mc
The verb synonym section is headed by a line containing a tab or sqaces
followed by the word "words" which is optionally followed by "verbs" or
"(verbs)".
.s3
.mc
Verbs are entered one per line, followed by spaces or a tab and a number.
This number is zero, (or omitted), if the verb is the primary form,
(i.e. the form
that the program will use when it prints the verb out for any reason).
Synonyms will be folowed by ascending integers.
.s3
Confusing?  For example,
if "hit", "strike", "break", "battle" and "attack" are to be synonyms,
with "attack" being the preferred form, the following would define them.
.nf

attack	0
hit	1
strike	2
break	3
battle  4

.fi
Note that "strike" must be followed by a "2" in the above, else it would
not be a synonym for "attack".
.br
.mc |
.sh "WORD (OBJECT) DEFINITIONS"
This section is used for defining objects that either
have synonyms, or are to be placed in specific locations at the beginning of
the wander, (or being carried by the person playing).
.s3
The object definition section is headed by a line containing spaces or a tab
and the word "words" optionally followed by something like "objects"
or "(objects)".
.s3
Objects are entered one per line, followed by spaces or tabs and
up to three numbers.
.br
The first number is zero if the object is the primary form
or an ascending integer if the object is a synonym.
.br
The second number is either the number of the location in which the object
is to appear, zero, or "-1" indicating that the player is already carrying
it.
.br
The third number is a flag indicating "special handling" when describing
the object.  It is the sum of any combination of the following numbers:
.nf
 1  The obj is singular even though it ends with an 's'
 2  The obj is plural even though it doesn't end with an 's'
 4  The obj already contains an article; so don't supply one
 8  The obj is a complete description as is; so don't use
    constructions like "there is a *** here" for it.
16  This form of the obj is to be used ONLY when describing
    it in a location, not when being carried, dropped, etc.
.fi
Note that 1 and 2 need only rarely be used, e.g. if you have a 'princess'
she will be described as "some princess",
as in "there are some princess here",
 unless you specify a 1, but seldom
do such examples seem to arise.
Note also that 8 usually requires 16, (i.e. 24),  whereas 16 doesn't always
require 8.
If you use the 16 code be sure that it is the zeroth synonym and that there
is a first synonym following it that can be used for the carrying, dropping,
etc. cases.
.s3
Any omitted number is assumed to be zero.
.s3
For example, if something called "credit card" is to appear in location
26 and is to have "card" as a synonym, the following would be appropriate:
.nf

credit\*(EC card   0  26
card   1

.fi
If the object "wallet" is to be carried by the player at the very beginning
then it would be defined as:
.nf

wallet   0   -1

.fi
If the object "sword" is going to appear through some action during the
wander and will change subtly when it is being carried then it could be
defined as:
.nf

sword   0  0  16
glowing\ sword  1

.fi
Note that the omitted numbers default to zeroes.
.mc
.sh PRE-ACTIONS
Pre-actions are global actions that are checked before any local actions.
.s3
The order of search for matches to user commands is:
.nf
    1) pre-actions (from .misc)
    2) location.state actions (from .wrld)
    3) location base state actions (from .wrld)
    4) post-actions (from .misc)
    5) built-in actions (quit, look, shell, etc)
.fi
.s3
The pre-actions section is headed by a line containing spaces or tabs
followed by the word "pre", (actually only the "pr" is checked).
.s3
Each action takes one line which starts with spaces or tabs
and has the same format as actions in the
.it .wrld
file, (see ".wrld" documentation for details)
.s3
A typical use might be, in a situation where the credit card defined above
must be "insert"ed into machines to be used and you'd like not to have to
check in each location whether the user has the card, to include the
following:
.nf

.mc |
     insert\*(EC card  o~card  m="You don't have a credit card."
.mc

.fi
In the case that someone says "insert card" without having one, the above
message will be output and no local action will be invoked.  If, on the
other hand, the card is there the "o~card" condition will not be met and
searching will continue in the local actions.
.sh POST-ACTIONS
Post-actions are used to provide default actions in the case that no
local action is provided for a particular command.
Their format is
the same as pre-actions except that their header is a line beginning with
spaces or tabs and the word "post".
.s3
A typical example is to provide a way of
determining your credit balance at any time.
.nf

balance  o~card  m="You need a credit card to do that!"
balance  o?card  m="Your account now stands at %6% credits."
.fi
(This assumes that variable 6 contains the number of credits.)
.sh VARIABLES
This section contains initializations of variables and is also a handy
place to keep information about the use of variables.
.s3
The variables section is headed by a line containing tabs
or spaces and the word "variables".
The format for the following lines is:

variable number <SEP> initial value <SEP> comments.

For example:
.nf

0           0       number of treasures found
1           0       number of vampires killed
2           50      dollars in bank account
3           0       temp variable used for dice game
CUR_LOC     3       starting location
MAX_CARRY   4       maximum number of things that can be carried at once

.fi
Notice the use of the mnemonic names "CUR_LOC" and "MAX_CARRY"; these
are special, built-in variables and are described further in "wander.doc".

.sh NOTES
The notes section is ignored by
.it Wander
and is provided as a convenient spot to record such things as
reminders of expansion plans, etc.
.s3
The notes section is headed by a line containing tabs
or spaces and the word "notes".
.s3
Notes can also be included anywhere in the file in lines that begin with ":".
.sh BUGS
.sh "SEE ALSO"
wrld.doc, wander.doc
.CC
