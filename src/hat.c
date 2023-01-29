#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <regex.h>
#include <dirent.h>
#include <time.h>

#include <csv.h>
#include <stack.h>
#include "strstack.h"
#include "str.h"
#include "parser.h" //only for date.h
#include "date.h"
#include "arg.h"
#include "io.h"
#include "hat.h"

static Hat *getdefhat(int);
static int formatpaths(Stack *, char *, char *);
static int getfiles(Stack *);
static int getdate(time_t *, char *);
static void splitfield(Stack *, char *, char *, int);

static Stack list;


//fill the list stack with Hat structs
int
hat_init(void)
{
	Csv *csv;
	int y = 1; //to ignore the first line
	stack_init(&list, 512, 512, sizeof(Hat));

	prnt("loading the database...\n");
	if((csv = csvload(arg_getcsv(), arg_getsep())) == NULL){
		prnte("err: couldn't load the database\n");
		return -1;
	}

	//loop over the entire file
	for(; csvy(csv, y); y++){

		Hat hat;
		stack_init(&hat.ptys, 4, 2, sizeof(Pty));
		stack_init(&hat.paths, 1, 4, HAT_PATHLEN);

		int x = 0;
		char *key, *val;

		//add all the properties of the hat in the struct
		for(; (key = csvsxy(csv, x, 0)) != NULL; x++){
			val = csvsxy(csv, x, y);
			Pty pty;

			strncpy(pty.key, key, HAT_KEYLEN);
			pty.key[HAT_KEYLEN-1] = '\0';
			strncpy(pty.val, val, HAT_VALLEN);
			pty.val[HAT_VALLEN-1] = '\0';

			if(stack_add(&hat.ptys, &pty)<0){
				prnte("err: couldn't add pty (key=%s, val=%s) to ptys\n", pty.key, pty.val);
				return -1;
			}
		}

		if(stack_add(&list, &hat)<0){
			prnte("err: couldn't add database entry (index %d) to internal list\n", y);
			return -1;
		}
	}

	Hat *hat;
	Hat *defhat;
	int namei;
	int pathi;
	int classi;
	int datei;

	if((namei = hat_getptyi("hat"))<0){
		prnte("err: name (or \"hat\") property not found\n");
		return -1;
	}

	if((classi = hat_getptyi("class"))<0){
		prnte("err: class property not found\n");
		return -1;
	}

	if((pathi = hat_getptyi("path"))<0){
		prnte("err: path property not found\n");
		return -1;
	}

	if((datei = hat_getptyi("date"))<0){
		prnte("err: date property not found\n");
		return -1;
	}

	if((defhat = getdefhat(namei)) == NULL){
		prnte("err: couldn't find the default hat (defhat)\n");
		return -1;
	}

	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;){

		Pty *path = stack_get(&hat->ptys, pathi);
		Pty *class = stack_get(&hat->ptys, classi);
		Pty *date = stack_get(&hat->ptys, datei);

		if(strcmp(class->val, "All classes") == 0)
			class = stack_get(&defhat->ptys, classi);

		formatpaths(&hat->paths, class->val, path->val);
		getfiles(&hat->paths);
		getdate(&hat->date, date->val);

	}
	return 0;
}

void
hat_free(void)
{
	Hat *hat;
	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;){
		stack_free(&hat->ptys);
		stack_free(&hat->paths);
	}
	stack_free(&list);
}

int
hat_getptyi(char *key)
{
	Hat *hat = stack_get(&list, 0);
	Pty *pty;

	for(int i = 0; (pty = stack_getnextused(&hat->ptys, &i)) != NULL;){
		if(strcmp(pty->key, key) == 0)
			return i-1; //-1 bc stack_getnextused() increments it
	}
	return -1;
}

int
hat_defsearch(Stack *paths, char *key, char *pattern)
{
	int id;
	Pty *pty;
	Hat *hat;
	regex_t reg;
	regmatch_t match[1];
	int res;

	if((id = hat_getptyi(key))<0)
		return -1;

	if((res = regcomp(&reg, pattern, REG_ICASE | REG_EXTENDED))){ // don't care about case
		char err[44];
		regerror(res, &reg, err, 44);
		prnte("err: regex error %d: %s\n", res, err);
		return -2;
	}

	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;){
		pty = stack_get(&hat->ptys, id);
		if(!regexec(&reg, pty->val, sizeof(match) / sizeof(match[0]), match, 0)){
			strstack_addto(paths, &hat->paths);
		}
	}

	regfree(&reg);
	return 0;
}

int
hat_pathsearch(Stack *paths, char *pattern)
{
	Hat *hat;
	char *path;
	regex_t reg;
	regmatch_t match[1];
	int res;

	if((res = regcomp(&reg, pattern, REG_EXTENDED))){ //we do care about case here
		char err[44];
		regerror(res, &reg, err, 44);
		prnte("err: regex error %d: %s\n", res, err);
		return -1;
	}

	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;){
		for(int e = 0; (path = stack_getnextused(&hat->paths, &e)) != NULL;){
			if(!regexec(&reg, path, sizeof(match) / sizeof(match[0]), match, 0))
				stack_add(paths, path);
		}
	}

	return 0;
}

int
hat_datesearch(Stack *paths, time_t from, time_t to)
{
	Hat *hat;
	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;)
		if(hat->date >= from && hat->date <= to)
			strstack_addto(paths, &hat->paths);
	return 0;
}

Hat *
hat_get(int id)
{
	return stack_getused(&list, id);
}

int
getdate(time_t *res, char *str)
{
	time_t tmp = date_getepoch(str);
	if(tmp<0){
		*res = 0;
		return -1;
	}
	*res = tmp;
	return 0;
}

static Hat *
getdefhat(int namei)
{
	Hat *hat;
	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;){
		Pty *name = stack_get(&hat->ptys, namei);
		if(strcmp(name->val, "defhat") == 0)
			return hat;
	}
	return NULL;
}

//duplicate any path with [CLASS] in it by the number of classes the hat
//is for and replace it with the class, and do a regex with the paths to
//find all the hat's files
static int
formatpaths(Stack *npaths, char *cpty, char *ppty)
{
	Stack classes, paths;
	char c[HAT_VALLEN] = {0}, p[HAT_PATHLEN] = {0};

	stack_init(&classes, 9, 9, HAT_VALLEN);
	stack_init(&paths, 1, 3, HAT_PATHLEN);

	strncpy(c, cpty, HAT_VALLEN);
	strncpy(p, ppty, HAT_VALLEN);

	//separate the classes and paths and add them to their stacks
	splitfield(&classes, c, "|", HAT_VALLEN);
	splitfield(&paths, p, "|", HAT_PATHLEN);

	//now, we have all the classes and all the unformatted paths in stacks

	char *path;
	char *class;
	for(int i = 0; (path = stack_getnextused(&paths, &i)) != NULL;){
		if(strstr(path, "[CLASS]") == NULL)
			continue;

		Stack tmppaths;
		stack_init(&tmppaths, 2, 7, HAT_PATHLEN);

		//duplicate the path and replace [CLASS] by each class
		for(int e = 0; (class = stack_getnextused(&classes, &e)) != NULL;){

			//gotta lower the class name
			char lclass[HAT_VALLEN] = {0};
			for(int a = 0; class[a] != '\0'; a++)
				lclass[a] = tolower(class[a]);

			char tmp[HAT_PATHLEN] = {0};
			strcpy(tmp, path);
			strswapall(tmp, "[CLASS]", lclass);
			stack_add(&tmppaths, tmp);
		}
		//we don't need the unformatted path anymore
		stack_rem(&paths, i-1);

		for(int e = 0; (path = stack_getnextused(&tmppaths, &e)) != NULL;)
			stack_add(&paths, path);

		stack_free(&tmppaths);
	}

	//now, we have formatted paths ready to be regexed against folders and files

	for(int i = 0; (path = stack_getnextused(&paths, &i)) != NULL;)
		stack_add(npaths, path);

	stack_free(&classes);
	stack_free(&paths);
	return 0;
}

//replace the paths from the stack (which are used as regexes) with the
//filepaths of the files they point to.
static int
getfiles(Stack *paths)
{
	Stack matches;
	char *path;
	char pattern[HAT_PATHLEN] = {".*"};

	stack_init(&matches, 2, 3, HAT_PATHLEN);

	for(int i = 0; (path = stack_getnextused(paths, &i)) != NULL;){

		int len = strlen(path);
		char fpath[HAT_PATHLEN] = {0};
		strcpy(fpath, arg_getinput());
		strcat(fpath, "/");
		strcat(fpath, path);

		if(path[len-1] == '*'){
			//the path target files, separate the path into the parent directory and the pattern
			char *delim = strrchr(fpath, '/');
			fpath[delim - fpath] = '\0';
			strcpy(pattern, delim+1); //+1 to exclude the '/'
		}

		//prepare and compile the regex
		regex_t reg;
		regmatch_t match[1];
		int res;
		if((res = regcomp(&reg, pattern, REG_EXTENDED))){
			char err[44];
			regerror(res, &reg, err, 44);
			prnte("err: regex error %d: %s\n", res, err);

			stack_free(&matches);
			return -1;
		}

		struct dirent *file;
		DIR *dir;

		if((dir = opendir(fpath)) == NULL){
			prnte("err: couldn't open directory %s\n", fpath);

			stack_free(&matches);
			return -1;
		}

		//go through all the files and see which ones match with the regex
		while((file = readdir(dir)) != NULL){
			char npath[HAT_PATHLEN] = {0};

			if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
				continue;
			if(regexec(&reg, file->d_name, sizeof(match) / sizeof(match[0]), match, 0))
				continue;

			//format the filepath and add it to the stack
			strcpy(npath, fpath);
			strcat(npath, "/");
			strcat(npath, file->d_name);
			stack_add(&matches, npath);
		}

		//don't forget to cleanup
		closedir(dir);
		regfree(&reg);
	}

	//empty the path stack and add the matched files instead
	for(int i = 0; (path = stack_getnextused(paths, &i)) != NULL;)
		stack_rem(paths, i-1);

	for(int i = 0; (path = stack_getnextused(&matches, &i)) != NULL;){
		stack_add(paths, path);
	}

	stack_free(&matches);
	return 0;
}

//separate field and add the parts to the stack
static void
splitfield(Stack *stack, char *s, char *sep, int size)
{
	//cannot add them to the stack directly, or it'll copy data outside the array
	char *tmp = malloc(size);

	for(char *next = s; next != NULL;){

		char *val;
		val = wstrsep(&next, sep);
		strcpy(tmp, val);

		stack_add(stack, tmp);
	}
	free(tmp);
}
