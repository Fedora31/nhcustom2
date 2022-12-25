#include <stdio.h>
#include <string.h>

#include <csv.h>
#include <stack.h>
#include "hat.h"

static Hat *getdefhat(int);
static int formatpaths(Stack *, char *, char *);

Stack list;


int
hat_init(void)
{
	Csv *csv;
	int y = 1;
	stack_init(&list, 512, 512, sizeof(Hat));

	if((csv = csvload("database.csv", ';')) == NULL){
		fprintf(stderr, "err: couldn't load the database\n");
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
				printf("err: couldn't add pty (key=%s, val=%s) to ptys\n", pty.key, pty.val);
				return -1;
			}
		}

		if(stack_add(&list, &hat)<0){
			printf("err: couldn't add hat (index %d) to list\n", y);
			return -1;
		}
	}

	Hat *hat;
	Hat *defhat;
	int namei;
	int pathi;
	int classi;

	if((namei = hat_getptyi("hat"))<0){
		fprintf(stderr, "err: name (or \"hat\") property not found\n");
		return -1;
	}

	if((classi = hat_getptyi("class"))<0){
		fprintf(stderr, "err: class property not found\n");
		return -1;
	}

	if((pathi = hat_getptyi("path"))<0){
		fprintf(stderr, "err: path property not found\n");
		return -1;
	}

	if((defhat = getdefhat(namei)) == NULL){
		fprintf(stderr, "err: couldn't find the default hat (defhat)\n");
		return -1;
	}

	for(int i = 0; (hat = stack_getnextused(&list, &i)) != NULL;){
		Pty *name = stack_get(&hat->ptys, namei);
		printf("%d: %s\n", i, name->val);

		Pty *path = stack_get(&hat->ptys, pathi);
		Pty *class = stack_get(&hat->ptys, classi);
		if(strcmp(class->val, "All classes") == 0)
			class = stack_get(&defhat->ptys, classi);
		formatpaths(&hat->paths, class->val, path->val);
	}
	return 0;
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
	stack_init(&classes, 9, 9, HAT_VALLEN);
	stack_init(&paths, 1, 3, HAT_PATHLEN);

	//copy the paths and classes into the stacks, and do the thing

	stack_free(&classes);
	stack_free(&paths);
	return 0;
}
