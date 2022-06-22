#include <stdio.h>
#include <string.h>
#include "csv.h"
#include "parser.h"


#define LINE_LENGTH 128


static void rmnl(char *);


int
main(int argc, char **args)
{
	printf("-- nhcustom2 --\n");

	Csv *db = csv_load("database.csv");
	FILE *conf = fopen("config.txt", "rb");
	if(conf == NULL) {
		fprintf(stderr, "Error: could not find the config file \"config.txt\"\n");
		csv_unload(db);
		return 1;
	}
	char line[LINE_LENGTH] = {0};
	int err = 0;

	if(fgets(line, LINE_LENGTH, conf) == NULL){
		fprintf(stderr, "Error: empty config file\n");
		csv_unload(db);
		fclose(conf);
		return 1;
	}
	rmnl(line);

	if(strcmp(line, "keep") == 0)
		printf("<keep mode>\n");
	else if (strcmp(line, "remove") == 0)
		printf("<remove mode>\n");
	else {
		fprintf(stderr, "Error: unkown mode. Either \"keep\" or \"remove\" must be on the first line.\n");
		csv_unload(db);
		fclose(conf);
		return 1;
	}


	while(fgets(line, LINE_LENGTH, conf) != NULL){
		rmnl(line);
		if(parseline(db, line) < 0) {
			fprintf(stderr, "Error parsing line: \"%s\"\n", line);
			err = 1;
		}
	}

	if(err){
		fprintf(stderr, "Errors occured, aborting.\n");
		csv_unload(db);
		fclose(conf);
		return 1;
	}

	csv_unload(db);
	fclose(conf);
	return 0;
}

//"remove" newline (and carriage return)
static void
rmnl(char *s)
{
	for(int i = 0; i < LINE_LENGTH; i++)
		if(s[i] == '\n' || s[i] == '\r') {
			s[i] = '\0';
			return;
		}
}
