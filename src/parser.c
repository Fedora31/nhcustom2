#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "str.h"
#include "csv.h"
#include "parser.h"

#define MAX_VALUES 24 //max values (ex: class:Scout|Soldier|*Heavy|!Medic...)

typedef struct Value{
	char *num[MAX_VALUES];
}Value;


static int gethv(char **, char *, Value *);

int
parseline(Csv *db, char *line)
{
	if(strlen(line) == 0)
		return 0;
	if(line[0] == '#')
		return 0;


	char l[CSV_FIELD];
	memcpy(l, line, strlen(line));

	char *li = l; //line index

	char h[CSV_FIELD];
	Value v;

	int res;
	while(1){
		if ((res = gethv(&li, h, &v)) == -1)
			return -1;
		else if(res == -2)
			break;

	}

	return 0;


}

static int
gethv(char **s, char *header, Value *value)
{
	header = wstrsep(s, ":"); //get the header
	if (*s == NULL) //we shouldn't be at the end of the string, no values have been found!
		return -1;
	printf("header: %s\n", header);

	int i = 0;
	char *values = wstrsep(s, ":"); //get the values
	while(values != NULL && i < MAX_VALUES){
		value->num[i] = wstrsep(&values, "|");
		printf("value %d: %s\n", i, value->num[i]);
		i++;
	}

	if (*s == NULL) //have we arrived at the end of the line?
		return -2;
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