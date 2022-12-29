#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "arg.h"
#include "str.h"

#include <stack.h>
#include "strstack.h"
#include "hat.h"
#include "parser.h"
#include "date.h"
#include "copy.h"
#include "io.h"

#define PARSER_LINELEN 512

//global pathlist, containing paths gotten during the
//execution of the configuration file
static Stack gstack;

//remove flag (if not 1, means keep flag)
static int removeflag = 1;

//get from the given string the header and the value(s) to search in it
//the given string is modified.
static int gethv(char **, Hvpair *);

//redirect the header/values to the function suited to handle them
static int redirect(Stack *, Hvpair *);
static int copy(char *);
static int modifystack(Stack *, Stack *, int, int);


int
parser_init(int flag)
{
	prnt("initializing the parser...\n");
	removeflag = flag;
	stack_init(&gstack, 64, 512, HAT_PATHLEN);

	if(!direxist(arg_getinput())){
		prnte("err: input directory \"%s\" doesn't exist\n", arg_getinput());
		return -1;
	}
	return 0;
}

void
parser_clean(void)
{
	stack_free(&gstack);
}

int
parser_exec(void)
{
	prnt("copying found files...\n");

	char *path;
	if(removeflag) //remove flag, copy the paths we found
		for(int i = 0; (path = stack_getnextused(&gstack, &i)) != NULL;){
			if(print)
				printf("%s\n", path);
			if(!norun)
				copy(path);
		}

	else{ //keep flag, copy the paths we haven't found
		Stack tmp;
		char str[HAT_PATHLEN] = {0};
		strcpy(str, arg_getinput());

		stack_init(&tmp, 1024, 1024, HAT_PATHLEN);
		prnt("scanning the input folder...\n");

		if(getallfiles(&tmp, str)<0)
			return -1;

		for(int i = 0; (path = stack_getnextused(&tmp, &i)) != NULL;)
			if(strstack_contain(&gstack, path)<0){
				if(print)
					printf("%s\n", path);
				if(!norun)
					copy(path);
			}

		stack_free(&tmp);
	}

	return 0;
}

int
parseline(char *line)
{
	if(strlen(line) == 0 || line[0] == '#')
		return 0;

	char l[PARSER_LINELEN] = {0};
	char *li = l; //line index
	Hvpair hvpair;
	Stack lstack; //local stack for the line

	strncpy(l, line, PARSER_LINELEN-1);

	stack_init(&lstack, 64, 128, HAT_PATHLEN);

	int exception; //holds if the first value of the line is an exception
	int filter; //same for the filter
	int res;
	int i = 0;
	while(1){
		if ((res = gethv(&li, &hvpair)) == -1)
			return -1;


		//the matches of the first value of the line will always be added to the stack, but it's exception
		//status is saved and will determine if the stack will be added to or deleted from the gstack
		if(i == 0) {
			prnt("%s\n", line);
			exception = hvpair.exception;
			filter = hvpair.filter;
			hvpair.exception = 0;
			hvpair.filter = 0;
		}

		//fill the lstack with the found matches
		if(redirect(&lstack, &hvpair) < 0)
			return -1;

		i++;
		if(res == -2) //no more values to get from the string
			break;
	}


	//modify the gstack according to the lstack and the modifiers
	modifystack(&gstack, &lstack, exception, filter);

	stack_free(&lstack);
	return 0;
}

//store the header:value pair and the exception status in the hvpair struct
//return -1 if an error occured, -2 if all the given string has been read
static int
gethv(char **s, Hvpair *hvpair)
{
	hvpair->header[0] = '\0';
	hvpair->value[0] = '\0';
	hvpair->exception = 0;
	hvpair->filter = 0;
	int e = 0, i = 0;
	int stop = 0;

	//get the header, simple enough
	char *tmph = wstrsep(s, ":");
	strcpy(hvpair->header, tmph);

	//if true, it means the header wasn't correctly written (missing ':')
	if(*s == NULL)
		return -1;

	//get the length of the **new** string, needed to know if we've arrived
	//at the end of the line or not
	int len = strlen(*s);

	//Get the value. This allows special characters to be escaped by a '\' if
	//we want to write them literally. This means that literal '\'s must also
	//be escaped.
	for(; (*s)[i] != '\0' && !stop; i++){
		switch((*s)[i]){
		case '\\':
			i++;
			break;
		case ':':
			stop = 1;
			continue;
		case '!':
			if(i == 0){
				hvpair->exception = 1;
				continue; //we don't want the flags printed
			}
			//act as a normal char in any other position
			break;
		case '}':
			if(i == 0){
				hvpair->filter = 1;
				continue;
			}
			break;
		}
		hvpair->value[e++] = (*s)[i];
	}
	hvpair->value[e] = '\0';

	//new string starts at 1 past the end colon
	*s = &(*s)[i];

	//if true, the value of the header:value pair was probably missing
	if(hvpair->value[0] == '\0')
		return -1;

	if(i >= len)
		return -2;

	return 0;
}

static int
redirect(Stack *res, Hvpair *hvpair)
{
	int ok;
	Stack tmp;
	stack_init(&tmp, 64, 512, HAT_PATHLEN);

	if(strcmp(hvpair->header, "date") == 0)
		ok = date_search(&tmp, hvpair);
	else if(strcmp(hvpair->header, "path") == 0)
		ok = hat_pathsearch(&tmp, hvpair->value);
	else
		ok = hat_defsearch(&tmp, hvpair->header, hvpair->value);

	if(ok<0)
		return -1;

	modifystack(res, &tmp, hvpair->exception, hvpair->filter);

	stack_free(&tmp);
	return 0;
}

//copy the file to the new location (move)
static int
copy(char *ofile)
{
	//ofile: <input>/models/...
	//nfile: <output>/models/...
	//dirs: <output>/models/player/... up until the last folder in the path

	char nfile[HAT_PATHLEN], dirs[HAT_PATHLEN];
	strcpy(nfile, ofile);
	strswap(nfile, arg_getinput(), arg_getoutput());
	strcpy(dirs, nfile);
	strrchr(dirs, '/')[0] = '\0';

	if(makedirs(dirs, 0755) < 0)
		return -1;
	if(fcopy(ofile, nfile) < 0)
		return -1;
	return 0;
}

//modify the stack according to the other stack and the modifiers
static int
modifystack(Stack *this, Stack *with, int exception, int filter)
{
	if(exception){
		strstack_remfrom(this, with);
	}else if(filter){
		strstack_filter(this, with);
	}else{
		strstack_addto(this, with);
	}
	return 0;
}
