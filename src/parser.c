#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "arg.h"
#include "str.h"
#include "csv.h"
#include "pl.h"

#include <stack.h>
#include "strstack.h"
#include "hat.h"
#include "parser.h"
#include "path.h"
#include "defield.h"
#include "date.h"
#include "copy.h"

#define PARSER_LINELEN 512


//all the paths in the input dir, used at the very end
static Pl apl;

//global pathlist, containing paths gotten during the
//execution of the configuration file
static Pl gpl;
static Stack gstack;

//line pathlist, containing paths gotten each line, emptied after each line
static Pl lpl;

//database used
static Csv *db;

//remove flag (if not 1, means keep flag)
static int removeflag = 1;

//get from the given string the header and the value(s) to search in it
//the given string is modified.
static int gethv(char **, Hvpair *);

//redirect the header/values to the function suited to handle them
//static int redirect(Csv *, Pl*, Hvpair *);
static int redirect2(Stack *, Hvpair *);

static int copy(char *);

int
parser_init(Csv *database, int flag)
{
	if(!quiet)
		printf("initializing the parser...\n");
	removeflag = flag;
	db = database;
	pl_alloc(&gpl);
	pl_alloc(&apl);
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

	//don't need to do that with the remove flag
	if(!removeflag){
		if(!quiet)
			printf("scanning the input folder...\n");
		char path[1024] = INPUT_DIR;
		if (getallfiles(&apl, path) < 0)
			return -1;
	}
	return 0;
}

void
parser_clean(void)
{
	pl_free(&gpl);
	pl_free(&apl);
}

int
parser_exec(void)
{
	if(!quiet)
		printf("copying found files...\n");

	/*
	//if remove flag, copy all the paths we found
	if(removeflag){
	for(int i = 0; i < gpl.max; i++){
		if(gpl.path[i][0] == 0)
			continue;
		copy(gpl.path[i]);
	}

	//else (keep), copy the paths we haven't found
	}else{
		for(int i = 0; i < apl.max; i++){
			if(apl.path[i][0] == 0)
				continue;
			if(!pl_contain(&gpl, apl.path[i])){
				copy(apl.path[i]);
			}
		}
	}
	*/

	char *path;
	if(removeflag)
		for(int i = 0; (path = stack_getnextused(&gstack, &i)) != NULL;)
			copy(path);
	else{ //keep flag, copy the paths we haven't found
		Stack tmp;
		char str[HAT_PATHLEN] = {INPUT_DIR};

		stack_init(&tmp, 1024, 1024, HAT_PATHLEN);
		printf("scanning the input folder...\n");
		getallfiles2(&tmp, str);

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
	//almost same code as parser_exec(). merge?
	if(removeflag){
	for(int i = 0; i < gpl.max; i++){
		if(gpl.path[i][0] == 0)
			continue;
		printf("%s\n", gpl.path[i]);
	}
	}else{
		for(int i = 0; i < apl.max; i++){
			if(apl.path[i][0] == 0)
				continue;
			if(!pl_contain(&gpl, apl.path[i])){
				printf("%s\n", apl.path[i]);
			}
		}
	}


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


	pl_alloc(&lpl);
	stack_init(&lstack, 64, 128, HAT_PATHLEN);

	int exception; //holds if the first value of the line is an exception
	int filter; //same for the filter
	int res;
	int i = 0;
	while(1){
		if ((res = gethv(&li, &hvpair)) == -1)
			return -1;


		//the matches of the first value of the line will always be added to the lpl, but it's exception
		//status is saved and will determine if the lpl will be added to or deleted from the gpl
		if(i == 0) {
			if(!quiet)
				printf("%s\n", line);
			exception = hvpair.exception;
			filter = hvpair.filter;
			hvpair.exception = 0;
			hvpair.filter = 0;
		}

		//fill the lpl with the found matches
		/*if(redirect(db, &lpl, &hvpair) < 0)
			return -1;*/
		if(redirect2(&lstack, &hvpair) < 0)
			return -1;

		i++;
		if(res == -2) //no more values to get from the string
			break;
	}


	//modify the gpl according to the lpl and if the first value was an exception
	if(exception){
		pl_remfrom(&gpl, &lpl);
		strstack_remfrom(&gstack, &lstack);
	}else if(filter){
		strstack_filter(&gstack, &lstack);
	}else{
		pl_addfrom(&gpl, &lpl);
		strstack_addto(&gstack, &lstack);
	}

	printf("exception =%d filter =%d\n", exception, filter);

	pl_free(&lpl);
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


	/*//tell if the value is an exception or a filter
	if(hvpair->value[0] == '!'){
		hvpair->exception = 1;
		hvpair->filter = 0;
		strremc(hvpair->value, 0);
	}else if(hvpair->value[0] == '}'){
		hvpair->filter = 1;
		hvpair->exception = 0;
		strremc(hvpair->value, 0);
	}else*/


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

/*
static int
redirect(Csv *db, Pl* pl, Hvpair *hvpair)
{
	if(strcmp(hvpair->header, "date") == 0)
		return date_add(db, pl, hvpair);
	if(strcmp(hvpair->header, "path") == 0)
		return path_add(db, pl, hvpair);
	return defield_add(db, pl, hvpair);
}
*/

static int
redirect2(Stack *res, Hvpair *hvpair)
{
	Stack tmp;
	stack_init(&tmp, 64, 512, HAT_PATHLEN);

	if(strcmp(hvpair->header, "date") == 0)
		date_search(&tmp, hvpair);
	else if(strcmp(hvpair->header, "path") == 0)
		path_search(&tmp, hvpair);
	else
		defield_search(&tmp, hvpair);

	if(hvpair->exception)
		strstack_remfrom(res, &tmp);
	else if(hvpair->filter){
		strstack_filter(res, &tmp);
	}else
		strstack_addto(res, &tmp);

	stack_free(&tmp);
	return 0;
}

//copy the file to the new location
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
