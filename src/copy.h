/*
 * requires: "pl.h", stack.h
 */


#define INPUT_DIR "./input"
#define OUTPUT_DIR "./output"
#define RECURSE_LIMIT 10000


int getallfiles(Pl *, char *);
int rmtree(char *);
int fcopy(char *, char *);
int makedirs(char *, int);
int makedir(char *, int);
int direxist(char *);
int getallfiles2(Stack *, char *);
