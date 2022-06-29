#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "str.h"
#include "csv.h"
#include "parser.h"
#include "pl.h"
#include "defield.h"
#include "date.h"
#include "path.h"
#include "copy.h"


//all the paths in the input dir, used at the very end
static Pl apl; 

//global pathlist, containing paths gotten during the
//execution of the configuration file
static Pl gpl; 

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
static int redirect(Csv *, Pl*, Hvpair *);


int
parser_init(Csv *database, int flag)
{
	removeflag = flag;

	db = database;
	pl_alloc(&gpl);
	pl_alloc(&apl);


	char path[1024] = INPUT_DIR;
	if (getallfiles(&apl, path) < 0)
		return -1;
	return 0;
}

void
parser_clean(void)
{
	pl_free(&gpl);
	pl_free(&apl);
}

void
parser_show(void)
{
	//if remove flag, print all the paths we found
	if(removeflag){
	for(int i = 0; i < gpl.max; i++){
		if(gpl.path[i][0] == 0)
			continue;
		printf("%s\n", gpl.path[i]);
	}

	//else (keep), print the paths we haven't found
	}else{
	for(int i = 0; i < apl.max; i++){
		if(apl.path[i][0] == 0)
			continue;

		if(!pl_contain(&gpl, apl.path[i]))
			printf("%s\n", apl.path[i]);
	}
	}

}

int
parseline(char *line)
{
	if(strlen(line) == 0 || line[0] == '#')
		return 0;

	char l[CSV_FIELD];
	memcpy(l, line, strlen(line)+1); //+1 to copy '\0'
	char *li = l; //line index

	Hvpair hvpair;
	pl_alloc(&lpl);

	int exception; //holds if the first value of the line is an exception
	int res;
	int i = 0;
	while(1){
		if ((res = gethv(&li, &hvpair)) == -1)
			return -1;


		//the matches of the first value of the line will always be added to the lpl, but it's exception
		//status is saved and will determine if the lpl will be added to or deleted from the gpl
		if(i == 0) {
			exception = hvpair.exception;
			hvpair.exception = 0;
		}

		//fill the lpl with the found matches
		if(redirect(db, &lpl, &hvpair) < 0)
			return -1;

		i++;
		if(res == -2) //no more values to get from the string
			break;
	}


	//modify the gpl according to the lpl and if the first value was an exception
	if(exception)
		pl_remfrom(&gpl, &lpl);
	else
		pl_addfrom(&gpl, &lpl);


	pl_free(&lpl);
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


	//tell if the value is an exception
	if(hvpair->value[0] == '!'){
		hvpair->exception = 1;
		strremc(hvpair->value, 0);
	}else
		hvpair->exception = 0;

	//have we arrived at the end of the string?
	if (*s == NULL)
		return -2;
	return 0;
}

static int
redirect(Csv *db, Pl* pl, Hvpair *hvpair)
{
	if(strcmp(hvpair->header, "date") == 0)
		return date_add(db, pl, hvpair);
	if(strcmp(hvpair->header, "path") == 0)
		return path_add(db, pl, hvpair);
	return defield_add(db, pl, hvpair);
}
