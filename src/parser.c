#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "str.h"
#include "csv.h"
#include "parser.h"



//get from the given string the header and the value(s) to search in it
//the given string is modified.
static int gethv(char **, Hvpair *);
//redirect the header/values to the function suited to handle them
static int redirect(Hvpair *);

int
parseline(Csv *db, char *line)
{
	if(strlen(line) == 0)
		return 0;
	if(line[0] == '#')
		return 0;


	char l[CSV_FIELD];
	memcpy(l, line, strlen(line)+1); //+1 to copy '\0'

	char *li = l; //line index

	Hvpair hvpair;

	int res;
	while(1){
		if ((res = gethv(&li, &hvpair)) == -1)
			return -1;

		printf("header: %s\n", hvpair.header);
		for(int i = 0; i < hvpair.count; i++)
			printf("value %d: %s\n", i, hvpair.value[i]);
		printf("count: %d\n", hvpair.count);

		redirect(&hvpair);

		if(res == -2) //no more values to get from the string
			break;
	}
	return 0;
}

static int
gethv(char **s, Hvpair *hvpair)
{
	char *tmph = wstrsep(s, ":"); //get the header
	memcpy(hvpair->header, tmph, strlen(tmph)+1);

	if (*s == NULL) //we shouldn't be at the end of the string, no values have been found!
		return -1;

	int i = 0;
	//get the values (the string up until the beginning of another header:value pair or a '\0'
	char *values = wstrsep(s, ":");

	if(strlen(values) == 0) //likewise, there should be values after the ":" notation
		return -1;

	char *tmpv;
	while(values != NULL && i < MAX_VALUES){
		tmpv = wstrsep(&values, "|");
		memcpy(hvpair->value[i], tmpv, strlen(tmpv)+1);
		i++;
	}
	hvpair->count = i;

	if (*s == NULL) //have we arrived at the end of the string?
		return -2;
	return 0;
}

static int
redirect(Hvpair *hvpair)
{
	return 0;
}











/*
	int pos[2];

	if (csv_searchpos(db, "hat", "Electronic Sports League", pos) >= 0) {
		printf("match: \"%s\" at pos %d/%d\n", csv_ptr(db, pos), pos[0], pos[1]);
		printf("path(s): \"%s\"\n", db->ptrs[csv_getheaderindex(db, "path")][pos[1]]);
	}else
		printf("no match found\n");

	return -1;
*/