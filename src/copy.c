#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <stack.h>
#include "str.h"
#include "copy.h"

static int isdir(char *);


//add all the filepaths under path into the stack res
//Warning: path is modified, and need to be large enough
//to contain the largest path in the tree.
int
getallfiles(Stack *res, char *path)
{
	static int recurse = RECURSE_LIMIT;
	DIR *dir;
	struct dirent *ent;

	recurse--;
	if(recurse < 0){
		fprintf(stderr, "err: getallfiles() recursed an abnormally high number of times!\n");
		return -1;
	}
	if((dir = opendir(path)) == NULL){
		fprintf(stderr, "err: getallfiles(): could not open directory %s\n", path);
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
			stack_add(res, path);
			//printf("found %s\n", tmp);

			//go back one level
			path[strrchr(path, '/')-path] = '\0';
			continue;
		}
		//else, recurse!
		if(getallfiles(res, path) < 0)
			return -1;
	}

	//go back one level
	path[strrchr(path, '/')-path] = '\0';

	closedir(dir);

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
fcopy(char *from, char *to)
{
	FILE *of, *nf;
	if((of = fopen(from, "rb")) == NULL)
		return -1;
	if((nf = fopen(to, "wb")) == NULL)
		return -2;
	char buf[4096];
	int n;

	while((n = fread(buf, sizeof(char), sizeof(buf), of)) > 0){
		if(fwrite(buf, sizeof(char), n, nf) < n){
			fprintf(stderr, "Error: an error occured while writing file %s\n", to);
			fclose(nf);
			fclose(of);
			return -1;
		}
	}

	fclose(nf);
	fclose(of);
	return 0;
}

int
makedirs(char *path, int rights)
{
	char tmp[1024] = {0};

	char **list = strsplit(path, "/"); //bad function, to change

	for(int i = 0; list[i] != NULL; i++){
		strcat(tmp, list[i]);

		if(!direxist(tmp)){
			//printf("should make %s\n", tmp);
			if(makedir(tmp, rights) < 0){
				fprintf(stderr, "Error: could not create directory \"%s\"\n", tmp);
				return -1;
			}
		}

		strcat(tmp, "/");
	}
	free(list);
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
