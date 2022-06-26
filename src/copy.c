#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"
#include "pl.h"
#include "copy.h"

#define INPUT_DIR "./input/"
#define OUTPUT_DIR "./output/"

static void copydel(Pl *pl, int);
static void copy(Pl *pl, int);

void
copypaths(Pl *pl, int remove)
{
	//for copying and deleting files, I chose to let the OS take care
	//of this business. I'm not experienced and confident enough
	//to roll out my own code for this matter.

	int win;

	if(getenv("OS") != NULL){
		printf("Windows\n");
		win = 1;
	}else{
		printf("Linux idk\n");
		win = 0;
	}


	if(remove)
		copy(pl, win);
	else
		copydel(pl, win);

}

//to use with the keep flag - copy all files
//from input to output, then delete the
//unwanted files
static void
copydel(Pl *pl, int win)
{
	/*for(int i = 0; i < pl->max; i++){

	}*/
}

//to use with the remove flag - copy only
//the files we want to the output folder
static void
copy(Pl *pl, int win)
{
	char cmd[1024] = {0}; //if it goes past this we have a problem
	char pdir[256] = {0};
	char *c;

	for(int i = 0; i < pl->max; i++){
		if(pl->path[i][0] == 0)
			continue;

		int len = strlen(pl->path[i]);
		memcpy(pdir, pl->path[i], len+1);

		//make pdir stop at the parent directory
		//if(pdir[len-1] == '*'){
			c = strrchr(pdir, '/');
			pdir[c-pdir+1] = '\0'; //+1 to leave the '/'
		//}

		printf("PDIR: %s\n", pdir);

		if(win){
			printf("to implement\n");
		}else{
			sprintf(cmd, "mkdir -p %s%s ; cp -r %s%s %s%s",
				OUTPUT_DIR, pdir, INPUT_DIR, pl->path[i], OUTPUT_DIR, pdir);
			//printf("CMD: %s\n", cmd);
			system(cmd);
		}
	}
}
