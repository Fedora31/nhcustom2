#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "str.h"
#include "csv.h"
#include "pathlist.h"
#include "parser.h"
#include "defield.h"


//Paths gotten at every line
static Pathlist globalpaths;


static Csvi gcsvi; //global indexes
static Csvi lcsvi; //line indexes


//get from the given string the header and the value(s) to search in it
//the given string is modified.
static int gethv(char **, Hvpair *);

//redirect the header/values to the function suited to handle them
static int redirect(Csv *, Csvi *, Hvpair *);


void
parser_init(void)
{
	csvi_alloc(&gcsvi);
}

void
parser_clean(void)
{
	csvi_free(&gcsvi);
}

void
parser_show(Csv *db)
{
	printf("~matches~\n");
	int hatindex = csv_getheaderindex(db, "hat");
	
	for(int i = 0; i < gcsvi.max; i++){
		if(gcsvi.pos[i][0] == -1)
			continue;
		printf("%s\n", db->ptrs[hatindex][gcsvi.pos[i][1]]);
	}
	printf("~end~\n");
}

int
parseline(Csv *db, char *line)
{
	if(strlen(line) == 0 || line[0] == '#')
		return 0;

	char l[CSV_FIELD];
	memcpy(l, line, strlen(line)+1); //+1 to copy '\0'
	char *li = l; //line index

	Hvpair hvpair;
	csvi_alloc(&lcsvi);

	int exception; //holds if the first value of the line is an exception
	int res;
	int i = 0;
	while(1){
		if ((res = gethv(&li, &hvpair)) == -1)
			return -1;


		//the first value of a line will always be added, but it's exception status is saved
		//and will determine if the whole line will be added to or deleted from the gcsvis
		if(i == 0) {
			printf("%s\n", line);

			exception = hvpair.exception;
			hvpair.exception = 0;
		}

		if(redirect(db, &lcsvi, &hvpair) < 0)
			return -1;

		i++;
		if(res == -2) //no more values to get from the string
			break;
	}


	//modify the gcsvi according to the lcsvi and if the first value was an exception
	if(exception){
		csvi_remfrom(&gcsvi, &lcsvi);
	}
	else
		csvi_addfrom(&gcsvi, &lcsvi);
	
	csvi_free(&lcsvi);

	return 0;
}

static int
gethv(char **s, Hvpair *hvpair)
{
	char *tmph = wstrsep(s, ":"); //get the header
	memcpy(hvpair->header, tmph, strlen(tmph)+1);

	if (*s == NULL) //we shouldn't be at the end of the string, no values have been found!
		return -1;

	//get the values (the string up until the beginning of another header:value pair or a '\0'
	char *value = wstrsep(s, ":");

	if(strlen(value) == 0) //likewise, there should be values after the ":" notation
		return -1;

	memcpy(hvpair->value, value, strlen(value)+1);

	//search for special characters.
	if(hvpair->value[0] == '!'){
		hvpair->exception = 1;
		strremc(hvpair->value, 0);
	}else
		hvpair->exception = 0;

	if (*s == NULL) //have we arrived at the end of the string?
		return -2;
	return 0;
}

static int
redirect(Csv *db, Csvi *csvi, Hvpair *hvpair)
{
	if(strcmp(hvpair->header, "class") == 0)
		printf("class\n");
	else if(strcmp(hvpair->header, "date") == 0)
		printf("date\n");
	else if(strcmp(hvpair->header, "equip") == 0)
		printf("equip\n");

	return defield_add(db, csvi, hvpair);
}



void
csvi_alloc(Csvi *csvi)
{
	csvi->count = 0;
	csvi->max = CSVINDEX_DEF_SIZE;
	csvi->pos = malloc(sizeof(csvi->pos) * CSVINDEX_DEF_SIZE);

	for(int i = 0; i < csvi->max; i++){
		csvi->pos[i][0] = -1;
		csvi->pos[i][1] = -1;
	}
}

void
csvi_realloc(Csvi *csvi)
{
	int i = csvi->max;
	csvi->max += CSVINDEX_DEF_SIZE;
	//possible memory leak?
	csvi->pos = realloc(csvi->pos, sizeof(csvi->pos) * csvi->max);

	for(; i < csvi->max; i++){
		csvi->pos[i][0] = -1;
		csvi->pos[i][1] = -1;
	}
}

void
csvi_free(Csvi *csvi)
{
	csvi->count = 0;
	csvi->max = 0;
	free(csvi->pos);
}

//when I think about it, I only have to care about the Y value...
//remove the X?
void
csvi_addpos(Csvi *csvi, int *pos)
{
	//function can be optimized

	//check if the Y value isn't already in the csvi
	for(int i = 0; i < csvi->max; i++)
		if(csvi->pos[i][1] == pos[1]) //nothing to do
			return;

	//printf("adding pos %d/%d...\n", pos[0], pos[1]);
	for(int i = 0; i < csvi->max; i++){
		//printf("%d: %d/%d\n", i, csvi->pos[i][0], csvi->pos[i][1]);
		if(csvi->pos[i][0] != -1)
			continue;
		//printf("Adding value!!\n");	
		csvi->pos[i][0] = pos[0];
		csvi->pos[i][1] = pos[1];
		return;
	}

	//printf("csvi full!\n");
	csvi_realloc(csvi);
	csvi_addpos(csvi, pos);
}

//because there's only one hat per line
void
csvi_remy(Csvi *csvi, int y)
{
	for(int i = 0; i < csvi ->max; i++){
		if(csvi->pos[i][1] == y){
			csvi->pos[i][0] = -1;
			csvi->pos[i][1] = -1;
		}
	}
}

void
csvi_addfrom(Csvi *to, Csvi *from)
{
	for(int i = 0;i < from->max; i++){
		if(from->pos[i][0] == -1)
			continue;
		csvi_addpos(to, from->pos[i]);
	}
}

void
csvi_remfrom(Csvi *to, Csvi *from)
{
	for(int i = 0; i < from->max; i++){
		if(from->pos[i][0] == -1)
			continue;
		csvi_remy(to, from->pos[i][1]);
	}
}

//logical and (if it's not in "to" and "from", it's deleted in "to")
void
csvi_andfrom(Csvi *to, Csvi *from)
{
	for(int i = 0; i < to->max; i++){
		if(to->pos[i][1] == -1)
			continue;
		if(!csvi_containsy(from, to->pos[i][1]))
			csvi_remy(to, to->pos[i][1]);
	}
}

int
csvi_containsy(Csvi *csvi, int y)
{
	for(int i = 0; i < csvi->max; i++){
		if(csvi->pos[i][0] == -1)
			continue;
		if(csvi->pos[i][1] == y)
			return 1;
	}
	return 0;
}

void csvi_print(Csvi *csvi)
{
	printf("CSVI CONTENT:\n");
	for(int i = 0; i < csvi->max; i++){
		if(csvi->pos[i][0] == -1)
			continue;
		printf("entry %d: %d/%d\n", i, csvi->pos[i][0], csvi->pos[i][1]);
	}
	printf("END\n");
}
