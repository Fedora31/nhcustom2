/*
 * requires: "csv.h"
 */

#define MAX_VALUES 24 //max values (ex: class:Scout|Soldier|*Heavy|!Medic...)


typedef struct Hvpair{
	char header[CSV_FIELD];
	char value[MAX_VALUES][CSV_FIELD];
	int count;
}Hvpair;


int parseline(Csv *, char *);
