extern int quiet;
extern int print;
extern int norun;

#define ARG_ARGLEN 128

int arg_process(int, char **);
char *arg_getinput(void);
char *arg_getoutput(void);
char *arg_getconf(void);
char *arg_getcsv(void);
char arg_getsep(void);
