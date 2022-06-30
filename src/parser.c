#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
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

static int copy(char *);

int
parser_init(Csv *database, int flag)
{
	printf("initializing the parser...\n");
	removeflag = flag;
	db = database;
	pl_alloc(&gpl);
	pl_alloc(&apl);

	char opath[1024] = OUTPUT_DIR;

	if(!direxist(INPUT_DIR)){
		fprintf(stderr, "Error: input directory \"%s\" doesn't exist\n", INPUT_DIR);
		return -1;
	}

	if(direxist(opath))
		if(rmtree(opath) < 0)
			return -1;
	makedir(OUTPUT_DIR, 0755);

	//don't need to do that with the remove flag
	if(!removeflag){
		printf("scanning the input folder...");
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
	//char tmp[1024] = {0};

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
	return 0;
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
			printf("%s\n", line);
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

static int copy(char *path)
{
	//dirs: output/models/player/... up until the last folder in the path
	//tmp1: input/models/...
	//tmp2: output/models/...

	char tmp1[1024], tmp2[1024], dirs[1024] = {0};
	sprintf(tmp1, "%s/%s", INPUT_DIR, path);
	sprintf(tmp2, "%s/%s", OUTPUT_DIR, path);
	sprintf(dirs, "%s/", OUTPUT_DIR);
	strncat(dirs, path, strrchr(path, '/') - path);


	if(makedirs(dirs, 0755) < 0)
		return -1;
	if(fcopy(tmp1, tmp2) < 0)
		return -1;
	return 0;
}
