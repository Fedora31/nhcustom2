#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <stack.h>
#include "arg.h"
#include "parser.h"
#include "hat.h"
#include "io.h"


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

	prnt("-- nhcustom2 --\n");

	//get the content of the config file
	int size;
	char *buf;
	fseek(conf, 0, SEEK_END);
	size = ftell(conf);
	rewind(conf);

	if(size == 0){
		prnte("fatal: empty config file\n");
		fclose(conf);
		return 1;
	}

	if((buf = malloc(size+1)) == NULL){
		prnte("fatal: main() malloc() failed\n");
		fclose(conf);
		return 1;
	}

	int fres;
	if((fres = fread(buf, 1, size, conf)) < size){
		prnte("fatal: fread() read %d bytes instead of %d\n", fres, size);
		fclose(conf);
		free(buf);
		return 1;
	}
	fclose(conf);
	buf[size] = '\0';


	if(hat_init()<0){
		prnte("fatal: the database couldn't be loaded\n");
		return 1;
	}

	if(parser_init(1) < 0){
		prnte("fatal: couldn't initialize the parser\n");
		free(buf);
		return 1;
	}


	//parse all the lines in the buffer
	int err = 0;
	int i = 0;
	char *ln = buf;
	for(; i < size; i++){

		if(buf[i] == '\r')
			buf[i] = '\0';

		//the second check is to process the last line if not terminated by a '\n'
		if(buf[i] == '\n' || i+1 >= size){

			if(buf[i] == '\n')
				buf[i] = '\0';

			if(parseline(ln) < 0){
				prnte("err: couldn't parse line: \"%s\"\n", ln);
				err = 1;
			}

			//update ln to point to the next line
			//this check probably won't be ever useful
			if(i+1 < size)
				ln = &buf[i+1];
		}
	}

	if(err){
		prnte("fatal: errors occured, aborting\n");
		parser_clean();
		return 1;
	}

	parser_exec();
	parser_clean();
	//TODO: free the hat list

	prnt("done.\n");
	return 0;
}
