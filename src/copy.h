/*
 * requires: "pl.h", stack.h
 */


#define INPUT_DIR "./input"
#define OUTPUT_DIR "./output"
#define RECURSE_LIMIT 10000


int rmtree(char *);
int fcopy(char *, char *);
int makedirs(char *, int);
int makedir(char *, int);
int direxist(char *);
int getallfiles(Stack *, char *);
