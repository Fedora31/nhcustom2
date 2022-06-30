/*
 * requires: "pl.h"
 */


#define INPUT_DIR "./input"
#define OUTPUT_DIR "./output"
#define RECURSE_LIMIT 10000


int getallfiles(Pl *, char *);
int rmtree(char *);
int makedir(char *, int);
int direxist(char *);
