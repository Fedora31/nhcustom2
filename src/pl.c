#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "str.h"
#include "csv.h"
#include "parser.h"
#include "pl.h"

//hardcoded and ugly
static char allclasses[] = "Scout|Soldier|Pyro|Demoman|Heavy|Engineer|Medic|Sniper|Spy\0";

static void pl_modify(Pl *, char*, char*, int);

void
pl_alloc(Pl *pl)
{
	pl->count = 0;
	pl->max = PL_DEF_SIZE;
	pl->path = malloc(CSV_FIELD * PL_DEF_SIZE);

	for(int i = 0; i < pl->max; i++)
		memset(pl->path[i], 0, CSV_FIELD);
}

void
pl_realloc(Pl *pl)
{
	int i = pl->max;
	pl->max += PL_DEF_SIZE;
	pl->path = realloc(pl->path, CSV_FIELD * pl->max);

	for(; i < pl->max; i++)
		memset(pl->path[i], 0, CSV_FIELD);
}

void
pl_free(Pl *pl)
{
	pl->count = 0;
	pl->max = 0;
	free(pl->path);
}

void
pl_add(Pl *pl, char *path)
{
	//can be optimized

	//check if the path isn't already in the pathlist
	for(int i = 0; i < pl->max; i++)
		if(strcmp(pl->path[i], path) == 0)
			return;

	for(int i = 0; i < pl->max; i++){
		if(pl->path[i][0] != 0)
			continue;
		memcpy(pl->path[i], path, strlen(path)+1);
		printf("added %s\n", path);
		return;
	}
	pl_realloc(pl);
	pl_add(pl, path);
}

void
pl_rem(Pl *pl, char *path)
{
	for(int i = 0; i < pl->max; i++){
		if(pl->path[i][0] == 0)
			continue;
		if(strcmp(pl->path[i], path) == 0){
			memset(pl->path[i], 0, CSV_FIELD);
			printf("removed %s\n", path);	
			return;
		}
	}
}

void
pl_addfrom(Pl *to, Pl *from)
{
	for(int i = 0; i < from->max; i++){
		if(from->path[i][0] == 0)
			continue;
		pl_add(to, from->path[i]);
	}
}

void
pl_remfrom(Pl *to, Pl *from)
{
	for(int i = 0; i < from->max; i++){
		if(from->path[i][0] == 0)
			continue;
		pl_rem(to, from->path[i]);
	}
}

void
pl_fadd(Pl *pl, char *classstr, char *pathsstr)
{
	pl_modify(pl, classstr, pathsstr, 0);
}

void
pl_frem(Pl *pl, char *classstr, char *pathsstr)
{
	pl_modify(pl, classstr, pathsstr, 1);
}

static void pl_modify(Pl *pl, char *classstr, char *pathsstr, int remove)
{
	//copy the classes and paths, as they will be modified

	char classes[CSV_FIELD];
	//replace potential "All classes" string by a string with all the classes
	if(strcmp(classstr, "All classes") == 0)
		memcpy(classes, allclasses, sizeof(allclasses)+1);
	else
		memcpy(classes, classstr, strlen(classstr)+1);

	//i have to lower the classes' names for the format() function
	for(int i = 0; i < CSV_FIELD && classes[i] != '\0'; i++)
		classes[i] = tolower(classes[i]);

	char paths[CSV_FIELD];
	memcpy(paths, pathsstr, strlen(pathsstr)+1);

	//printf("classes = %s\npaths = %s\n", classstr, pathsstr);


	//split the classes and paths
	char **class = strsplit(classes);
	char **path = strsplit(paths);


	//format the paths and add/remove them. To tidy up, preferably...

	char fpath[CSV_FIELD];
	for(int i = 0; path[i] != NULL; i++){

		//no need to format it?
		if(strstr(path[i], "[CLASS]") == NULL){
			if(remove)
				pl_rem(pl, path[i]);
			else
				pl_add(pl, path[i]);
			continue;
		}

	for(int e = 0; class[e] != NULL; e++){

		memcpy(fpath, path[i], strlen(path[i]) + 1);
		//"[CLASS]" swapped by current class
		strswapall(fpath, "[CLASS]", class[e]);

		if(remove)
			pl_rem(pl, fpath);
		else
			pl_add(pl, fpath);

	}
	}

	free(class);
	free(path);
}
