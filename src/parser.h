/*
 * requires: "csv.h"
 */

#define MAX_VALUES 24 //max values (ex: class:Scout|Soldier|*Heavy|!Medic...)
#define CSVINDEX_DEF_SIZE 48//to increase, lol

typedef struct Hvpair{
	char header[CSV_FIELD];
	char value[CSV_FIELD];
	int exception; //wether the value should be removed or added

}Hvpair;

typedef struct Csvi{
	int (*pos)[2]; //csv positions
	int count;	//number of indexes (do I ever use it?)
	int max; //max number of indexes
}Csvi;

void parser_init(void);
void parser_show(Csv *);
void parser_exec(Csv *);
void parser_clean(void);
int parseline(Csv *, char *);

void csvi_alloc(Csvi *);
void csvi_realloc(Csvi *);
void csvi_free(Csvi *);
void csvi_addpos(Csvi *, int *);
void csvi_remy(Csvi *, int);
void csvi_addfrom(Csvi *, Csvi *);
void csvi_remfrom(Csvi *, Csvi *);
void csvi_andfrom(Csvi *, Csvi *);
int csvi_containsy(Csvi *, int);
void csvi_print(Csvi *);
