#include <stdio.h>
#include <string.h>

#include "str.h"
#include "arg.h"

static char input[ARG_FOLDERLEN] = {"./input\0"};
static char output[ARG_FOLDERLEN] = {"./output\0"};

static void formatfolder(char *, char *);


int
arg_process(int argc, char **argv)
{
	int len;

	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			int step = 0;

			len = strlen(argv[i]);
			for(int e = 1; e < len; e++){
				switch(argv[i][e]){
				case 'q':
					quiet = 1;
					break;
				case 'p':
					print = 1;
					break;
				case 'i':
					step++;
					if(i+step >= argc)
						return -1;
					formatfolder(input, argv[i+step]);
					break;
				case 'o':
					step++;
					if(i+step >= argc)
						return -1;
					formatfolder(output, argv[i+step]);
					break;
				default:
					fprintf(stderr, "fatal: option not recognized: %c\n", argv[i][e]);
					return -1;
				}
			}
			i+=step; //to not get args that were already parsed in the switch
		}
	}
	printf("input = %s\noutput = %s\n", input, output);

	return 0;
}

char *
arg_getinput(void)
{
	return input;
}

char *
arg_getoutput(void)
{
	return output;
}

static void formatfolder(char *res, char *arg)
{
	int len = strlen(arg);
	strncpy(res, arg, ARG_FOLDERLEN-1);
	strswapall(res, "\\", "/");
	if(res[len-1] == '/')
		res[len-1] = '\0';
}