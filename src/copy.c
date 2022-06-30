#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "str.h"
#include "csv.h"
#include "pl.h"
#include "copy.h"


static int isdir(char *);


int
getallfiles(Pl *pl, char *path)
{
	//static variable to check if the function doesn't go haywire
	//currently the function recurses around 1900 times
	static int recurse = RECURSE_LIMIT;
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
				strswap(tmp, INPUT_DIR, "");
				strswap(tmp, "/", "");
				pl_add(pl, tmp);
				//printf("found %s\n", tmp);

				//go back one level
				path[strrchr(path, '/')-path] = '\0';

				continue;
			}

			//else, recurse!
			if(getallfiles(pl, path) < 0)
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

int
rmtree(char *path)
{
	static int recurse = RECURSE_LIMIT;
	DIR *dir;
	struct dirent *ent;

	recurse--;

	if(recurse < 0){
		fprintf(stderr, "Error: rmtree() recursed an abnormally high number of times!!\n");
		return -1;
	}

	if((dir = opendir(path)) == NULL){
		fprintf(stderr, "Error: rmtree(): could not open directory %s\n", path);
		return -1;
	}

	while((ent = readdir(dir)) != NULL){
		if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;

		//add new file/folder to the path
		strcat(path, "/");
		strcat(path, ent->d_name);

		if(isdir(path) == 0){

			//printf("removing %s\n", path);

			if(remove(path) < 0){
				fprintf(stderr, "Error: could not remove file \"%s\"\n", path);
				free(ent);
				closedir(dir);
				return -1;
			}

			//go back one level
			path[strrchr(path, '/')-path] = '\0';

			continue;
		}

		if(rmtree(path) < 0)
			return -1;

	}

	free(ent);
	closedir(dir);
	//apparently I must use rmdir to please Windows
	if(rmdir(path) < 0){
		int a = errno;
		fprintf(stderr, "Error: could not remove directory \"%s\" (%d %s)\n", path, a, strerror(a));
		return -1;
	}

	//go back one level
	path[strrchr(path, '/')-path] = '\0';


	//to test with this check removed
	if(recurse < 0)
		return -1;
	return 0;
}

int
direxist(char *path)
{
	//not a very robust function...
	struct stat s;
	int r = stat(path, &s);

	if(r == -1 && errno == ENOENT)
		return 0;
	return 1;
}

static int
isdir(char *path)
{
	struct stat s;
	if(stat(path, &s) != 0)
		return 0;
	return S_ISDIR(s.st_mode);
}
