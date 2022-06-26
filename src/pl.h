/*
 * requires: "csv.h"
 */

#define PL_DEF_SIZE 4 //to increase lol

//path list
typedef struct Pl{
	char (*path)[CSV_FIELD];
	int count;
	int max;
}Pl;


void pl_alloc(Pl *);
void pl_realloc(Pl *);
void pl_free(Pl *);
void pl_add(Pl *, char *);
void pl_fadd(Pl *, char *, char *);
void pl_addfrom(Pl *, Pl *);
void pl_rem(Pl *, char *);
void pl_frem(Pl *, char *, char *);
void pl_remfrom(Pl *, Pl *);
