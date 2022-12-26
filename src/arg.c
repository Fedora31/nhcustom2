#include <stdio.h>
#include <string.h>

#include "arg.h"


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
				default:
					fprintf(stderr, "fatal: option not recognized: %c\n", argv[i][e]);
					return -1;
				}
			}
			i+=step; //to not get args that were already parsed in the switch
		}
	}
	return 0;
}
