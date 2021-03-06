#define CSV_MAX_LINE_LENGTH 2048 //max length of a line in the csv file
#define CSV_MAX_HEADERS 24 //max number of headers
#define CSV_FIELD 1024 //max length of a single field (for;example;this) (some cosmetics have reaaaally long path fields)

typedef struct Csv{
	char *headers[CSV_MAX_HEADERS];
	int hcount;
	int lcount; //number of lines (excluding the header line!)

	char ***ptrs;
	char *buf;
}Csv;

typedef struct Csvres{
	char *res;		//result of the query
	char **resline;	//array of the pointers in the same line as the match
	struct Csvres *next; //maybe another result
}Csvres;

Csv * csv_load(char *);
int csv_searchpos(Csv *, char *, char *, int,  int *);
int csv_searchline(Csv *, char *, char *, int);
int csv_getheaderindex(Csv *, char *);
char * csv_ptr(Csv *, int *);
void csv_unload(Csv *);
