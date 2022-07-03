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

	FILE *conf = fopen("config.txt", "rb");
	if(conf == NULL) {
		fprintf(stderr, "Error: could not find the config file \"config.txt\"\n");
		return 1;
	}


	char line[LINE_LENGTH] = {0};
	int err = 0;

	if(fgets(line, LINE_LENGTH, conf) == NULL){
		fprintf(stderr, "Error: empty config file\n");
		fclose(conf);
		return 1;
	}
	rmnl(line);

	int remove = 1;
	if(strcmp(line, "keep") == 0){
		printf("<keep>\n");
		remove = 0;
	}else if (strcmp(line, "remove") == 0){
		printf("<remove>\n");
		remove = 1;
	}else{
		fprintf(stderr, "Error: unkown mode. Either \"keep\" or \"remove\" must be on the first line.\n");
		fclose(conf);
		return 1;
	}

	printf("loading the database...\n");
	Csv *db;
	if((db = csv_load("database.csv")) == NULL){
		fprintf(stderr, "Error: could not load the database\n");
		fclose(conf);
		return 1;
	}

	if(parser_init(db, remove) < 0){
		fprintf(stderr, "Error: could not initialize the parser\n");
		csv_unload(db);
		fclose(conf);
		return 1;
	}

	//send each line to the parser
	while(fgets(line, LINE_LENGTH, conf) != NULL){
		rmnl(line);
		if(parseline(line) < 0) {
			fprintf(stderr, "Error parsing line: \"%s\"\n", line);
			err = 1;
		}
	}

	if(err){
		fprintf(stderr, "Errors occured, aborting.\n");
		csv_unload(db);
		fclose(conf);
		parser_clean();
		return 1;
	}

	parser_exec();

	csv_unload(db);
	fclose(conf);
	parser_clean();

	printf("done.\n");
	return 0;
}

//"remove" newline (and carriage return)
//move to str.c?
static void
rmnl(char *s)
{
	for(int i = 0; i < LINE_LENGTH; i++)
		if(s[i] == '\n' || s[i] == '\r') {
			s[i] = '\0';
			return;
		}
}
