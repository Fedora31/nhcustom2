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

				default:
					prnte("fatal: option not recognized: %c\n", argv[i][e]);
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
