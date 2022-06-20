#define CSV_MAX_LINE_LENGTH 1024
#define CSV_MAX_HEADERS 24

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
char * csv_search(Csv *, char *, char *);