#include <stdio.h>
#include <string.h>
#include <time.h>

#include <stack.h>
#include "arg.h"
#include "parser.h"
#include "hat.h"
#include "io.h"

//should make it "line length agnostic"
#define LINE_LENGTH 254

static void rmnl(char *);


int
main(int argc, char **argv)
{
	int res;
	//parse the given arguments
	if((res = arg_process(argc, argv))<0){
		if(res == -1)
			prnte("fatal: incorrect arguments\n");
		return 1;
	}

	FILE *conf = fopen(arg_getconf(), "rb");
	if(conf == NULL) {
		prnte("fatal: could not find the config file \"%s\"\n", arg_getconf());
		return 1;
	}

	char line[LINE_LENGTH] = {0};
	int err = 0;

	if(fgets(line, LINE_LENGTH, conf) == NULL){
		prnte("fatal: empty config file\n");
		fclose(conf);
		return 1;
	}

	prnt("-- nhcustom2 --\n");
	prnt("loading the database...\n");

	if(hat_init()<0){
		prnte("fatal: the database couldn't be loaded\n");
		return 1;
	}

	rmnl(line);
	int remove = 1;
	if(strcmp(line, "keep") == 0){
		prnt("mode: keep listed files\n");
		remove = 0;
	}else if (strcmp(line, "remove") == 0){
		prnt("mode: remove listed files\n");
		remove = 1;
	}else{
		prnte("fatal: unkown mode. Either \"keep\" or \"remove\" must be on the first line.\n");
		fclose(conf);
		return 1;
	}

	if(parser_init(remove) < 0){
		prnte("fatal: could not initialize the parser\n");
		fclose(conf);
		return 1;
	}

	//send each line to the parser
	while(fgets(line, LINE_LENGTH, conf) != NULL){
		rmnl(line);
		if(parseline(line) < 0) {
			prnte("err: couldn't parse line: \"%s\"\n", line);
			err = 1;
		}
	}

	if(err){
		prnte("fatal: errors occured, aborting\n");
		fclose(conf);
		parser_clean();
		return 1;
	}

	parser_exec();

	fclose(conf);
	parser_clean();

	prnt("done.\n");
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
