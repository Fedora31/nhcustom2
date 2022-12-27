#include <stdio.h>
#include <string.h>

#include "str.h"
#include "io.h"
#include "arg.h"

static char input[ARG_ARGLEN] = {"./input\0"};
static char output[ARG_ARGLEN] = {"./output\0"};
static char conf[ARG_ARGLEN] = {"./config.txt\0"};
static char csv[ARG_ARGLEN] = {"./database.csv\0"};
static char sep = ';';

//don't print and information, appart from errors.
int quiet = 0;
//don't touch any files and print the found paths instead.
int print = 0;
//dont copy any files over.
int norun = 0;

static void formatpath(char *, char *);
static int incrstep(int *, int, int);
static void showhelp(void);


int
arg_process(int argc, char **argv)
{
	int len;

	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			int step = 0;

			len = strlen(argv[i]);
			for(int e = 1; e < len; e++){
				switch(argv[i][e]){
				case 'q':
					quiet = 1;
					break;

				case 'p':
					print = 1;
					break;

				case 'n':
					norun = 1;
					break;

				case 'i':
					if(incrstep(&step, i, argc)<0)
						return -1;
					formatpath(input, argv[i+step]);
					break;

				case 'o':
					if(incrstep(&step, i, argc)<0)
						return -1;
					formatpath(output, argv[i+step]);
					break;

				case 'f':
					if(incrstep(&step, i, argc)<0)
						return -1;
					formatpath(conf, argv[i+step]);
					break;

				case 'd':
					if(incrstep(&step, i, argc)<0)
						return -1;
					formatpath(csv, argv[i+step]);
					break;

				case 's':
					if(incrstep(&step, i, argc)<0)
						return -1;
					if(sscanf(argv[i+step], "%c", &sep) != 1)
						return -1;
					break;

				case 'h':
				case '?':
					showhelp();
					return -2;

				default:
					prnte("fatal: option not recognized: %c\nTry -h for help.\n", argv[i][e]);
					return -1;
				}
			}
			i+=step; //to not get args that were already parsed in the switch
		}
	}
	return 0;
}

char *
arg_getinput(void)
{
	return input;
}

char *
arg_getoutput(void)
{
	return output;
}

char *
arg_getconf(void)
{
	return conf;
}

char *
arg_getcsv(void)
{
	return csv;
}

char
arg_getsep(void)
{
	return sep;
}

//change backslashes into slashes and remove
//any trailing slash at the end
static void
formatpath(char *res, char *arg)
{
	int len = strlen(arg);
	strncpy(res, arg, ARG_ARGLEN-1);
	strswapall(res, "\\", "/");
	if(res[len-1] == '/')
		res[len-1] = '\0';
}

//this takes care to see if there are any other
//arguments after the current position plus the
//incremented step
static int
incrstep(int *step, int pos, int argc)
{
	*step+=1;
	if(pos + *step >= argc)
		return -1;
	return 0;
}

//probably a better way to do it...
static void
showhelp(void)
{
	prnte("Usage: nhcustom2 [OPTION...]\n\
\n\
nhcustom2 is a utility to select and filter filepaths\n\
of cosmetics from the video game \"Team Fortress 2\".\n\
It was created to more easily create customized versions\n\
of the mods known as \"no hats mods\", which remove\n\
certain or all cosmetics from the game.\n\
\n\
nhcustom needs a database, an input folder and a\n\
configuration file to function. The input folder must contain\n\
a decompiled mod, starting at the folder \"models\".\n\
\n\
Options\n\
\n\
-q	Don't print info messages.\n\
-p	print the found paths to stdout,\n\
-n	don't touch to any file (the program has no effect).\n\
-i	specify the input folder. (default: %s)\n\
-o	specify the output folder. (default: %s)\n\
-f	specify the configuration file. (default: %s)\n\
-d	specify the database. (default: %s)\n\
-s	specify the separator used in the database. (default: %c)\n\
-h -?	show this text.\n\
\n\
Configuration syntax\n\
\n\
The syntax is composed of header:pattern values, which can\n\
be stacked by writing a column between them. Before the pattern,\n\
one can use an exception \"!\" or edit \"}\" flag, which changes\n\
the behavior of the matching:\n\
\n\
!	The matches will be removed from the already found matches.\n\
}	The already found matches will be removed if they don't match\n\
	with the current one.\n\
\n\
Note that the first value modifies all the previous matches,\n\
whereas stacked values modify only the matches found on the\n\
current line.\n\
\n\
By default the strings that are accepted as headers are the\n\
following:\n\
\n\
hat\n\
update\n\
equip\n\
class\n\
date\n\
path\n\
\n\
The \"date\" header's pattern must be written like so:\n\
YYYY-MM-DD/YYYY-MM-DD. Appart from the \"date\" header, the\n\
others support regexes.\n\
\n\
Examples\n\
\n\
#find hats that can be worn by mutiple classes but that are not all-class\n\
class:.*:class:!All classes\n\
\n\
#deselect hats that came out between 2008 and march 1st, 2013, except\n\
#if they can be worn by the soldier (along with other classes)\n\
date:!2008-01-01/2013-3-1:class:!Soldier\n\
\n\
#find all the hats containing the string \"aaa\"\n\
hat:aaa\n\
\n\
#find only the 1st style of the Millennial Mercenary\n\
hat:millennial mercenary:path:!style\n\
\n\
#find every path with the word \"scout\" in them\n\
path:.*scout.*\n\
\n\
#find only the first style of the Foppish Physician\n\
hat:foppish physician:path:!necktie\n\
\n\
#find only the third style of the medic cosmetics from Smissmas 2022\n\
update:Smissmas 2022:class:}^Medic$:path:}style3\n\
\n\
see <https://github.com/Fedora31/nhcustom2> for more complete\n\
explanations.\n\
",

arg_getinput(), arg_getoutput(), arg_getconf(), arg_getcsv(), arg_getsep()
);
}
