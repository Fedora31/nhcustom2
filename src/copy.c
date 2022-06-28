#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "str.h"
#include "csv.h"
#include "pl.h"
#include "copy.h"

#define INPUT_DIR "./input/"
#define OUTPUT_DIR "./output/"

static void copydel(Pl *pl, int);
static void copy(Pl *pl, int);
static int isdir(char *);

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

int
getallfiles(Pl *pl, char *path)
{
	//static variable to check if the function doesn't go haywire
	//currently the function recurses around 1900 times
	static int recurse = 10000;
	DIR *dir;
	struct dirent *ent;

	recurse--;
	if(recurse < 0){
		fprintf(stderr, "Error: getallfiles() recursed an abnormally high number of times!!\n");
		return -1;
	}

	if((dir = opendir(path)) == NULL){
		fprintf(stderr, "Error: getallfiles(): could not open directory %s\n", path);
		return -1;
	}

	while((ent = readdir(dir)) != NULL){
		if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0){
			continue;
		}

			//add new file/folder to the path
			strcat(path, "/");
			strcat(path, ent->d_name);

			//if file, add it to the pathlist and move on
			if(isdir(path) == 0){
				char tmp[1024];
				strcpy(tmp, path);
				strswap(tmp, "./input/", "");
				pl_add(pl, tmp);
				//printf("found %s\n", tmp);

				//go back one level
				path[strrchr(path, '/')-path] = '\0';

				continue;
			}

			//else, recurse!
			if(getallfiles(pl, path) > 0)
				return -1;

	}
	//go back one level
	path[strrchr(path, '/')-path] = '\0';

	closedir(dir);
	free(ent);

	if(recurse < 0)
		return -1;
	return 0;
}

static int
isdir(char * path)
{
	struct stat s;
	if(stat(path, &s) != 0)
		return 0;
	return S_ISDIR(s.st_mode);
}
