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


static int isdir(char *);


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
