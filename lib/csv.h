typedef struct Csv{
	int columns;
	int lines;
	char *buf;
	char ***cells;
}Csv;

enum CSV_ERRORS{
	CSV_FILE_NOT_LOADED = -2,
	CSV_MATCH_NOT_FOUND = -3
};

Csv * csvload(char *, char);
void csvdel(Csv *);
char * csvs(Csv *, char *, char *, char *);
int csvy(Csv *, int);
int csvx(Csv *, int);
char * csvsxy(Csv *, int, int);
