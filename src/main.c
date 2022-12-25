#include <stdio.h>
#include <string.h>
#include "arg.h"
#include "csv.h"
#include "parser.h"

//#include <csv.h>
#include <stack.h>
#include "hat.h"


//should make it "line length agnostic"
#define LINE_LENGTH 254

//don't print and information, appart from errors.
int quiet = 0;
//don't touch any files and print the found paths instead.
int print = 0;

static void rmnl(char *);


int
main(int argc, char **args)
{
	//parse the given arguments
	for(int i = 1; i < argc; i++){
		if(args[i][0] == '-'){
			int len = strlen(args[i]);
			for(int a = 1; a < len; a++){
				switch(args[i][a]){
				case 'q':
					quiet = 1;
					break;
				case 'p':
					print = 1;
					break;
				default:
					fprintf(stderr, "Error: unkown option \"%c\"\n", args[i][a]);
					return 1;
				}
			}
		}
	}

	if(hat_init()<0){
		fprintf(stderr, "hat_init() failed\n");
		return 1;
	}

	if(!quiet)
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
		if(!quiet)
			printf("<keep>\n");
		remove = 0;
	}else if (strcmp(line, "remove") == 0){
		if(!quiet)
			printf("<remove>\n");
		remove = 1;
	}else{
		if(!quiet)
			fprintf(stderr, "Error: unkown mode. Either \"keep\" or \"remove\" must be on the first line.\n");
		fclose(conf);
		return 1;
	}

	if(!quiet)
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

	if(print)
		parser_show();
	else
		parser_exec();

	csv_unload(db);
	fclose(conf);
	parser_clean();

	if(!quiet)
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
