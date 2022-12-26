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
	if(!quiet)
		printf("initializing the parser...\n");
	removeflag = flag;
	stack_init(&gstack, 64, 512, HAT_PATHLEN);

	char opath[1024] = OUTPUT_DIR;

	if(!direxist(INPUT_DIR)){
		fprintf(stderr, "Error: input directory \"%s\" doesn't exist\n", INPUT_DIR);
		return -1;
	}

	//only touch the files if the print flag isn't set
	if(!print){
		if(direxist(opath))
			if(rmtree(opath) < 0)
				return -1;
		makedir(OUTPUT_DIR, 0755);
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
	if(!quiet)
		printf("copying found files...\n");

	char *path;
	if(removeflag)
		for(int i = 0; (path = stack_getnextused(&gstack, &i)) != NULL;)
			copy(path);
	else{ //keep flag, copy the paths we haven't found
		Stack tmp;
		char str[HAT_PATHLEN] = {INPUT_DIR};

		stack_init(&tmp, 1024, 1024, HAT_PATHLEN);
		printf("scanning the input folder...\n");

		if(getallfiles(&tmp, str)<0)
			return -1;

		for(int i = 0; (path = stack_getnextused(&tmp, &i)) != NULL;)
			if(strstack_contain(&gstack, path)<0)
				copy(path);

		stack_free(&tmp);
	}

	return 0;
}

int
parser_show(void)
{
	//incomplete, to merge with parser_exec()

	char *p;
	for(int i = 0; (p = stack_getnextused(&gstack, &i)) != NULL;)
		printf("%s\n", p);


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
			if(!quiet)
				printf("%s\n", line);
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
	//get the header
	char *tmph = wstrsep(s, ":");
	memcpy(hvpair->header, tmph, strlen(tmph)+1);

	//we shouldn't be at the end of the string, no values have been found!
	if (*s == NULL)
		return -1;

	//get the value (the string up until the beginning of another header:value pair
	//(delimited by another colon) or a '\0'
	char *value = wstrsep(s, ":");

	//likewise, there should be values after the ":" notation in header:value
	if(strlen(value) == 0)
		return -1;

	memcpy(hvpair->value, value, strlen(value)+1);

	hvpair->exception = 0;
	hvpair->filter = 0;

	//tell if the value is an exception or a filter
	if(hvpair->value[0] == '!' || hvpair->value[0] == '}'){

		if(hvpair->value[0] == '!')
			hvpair->exception = 1;
		if(hvpair->value[0] == '}')
			hvpair->filter = 1;

		strremc(hvpair->value, 0);
	}

	//have we arrived at the end of the string?
	if (*s == NULL)
		return -2;
	return 0;
}

static int
redirect(Stack *res, Hvpair *hvpair)
{
	Stack tmp;
	stack_init(&tmp, 64, 512, HAT_PATHLEN);

	if(strcmp(hvpair->header, "date") == 0)
		date_search(&tmp, hvpair);
	else if(strcmp(hvpair->header, "path") == 0)
		hat_pathsearch(&tmp, hvpair->value);
	else
		hat_defsearch(&tmp, hvpair->header, hvpair->value);

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
	strswap(nfile, INPUT_DIR, OUTPUT_DIR);
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
