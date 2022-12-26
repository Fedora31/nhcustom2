#include <stdio.h>
#include <regex.h>
#include "csv.h"
#include "pl.h"
#include "parser.h"
#include <stack.h>
#include "path.h"
#include "hat.h"


int
path_add(Csv *db, Pl *pl, Hvpair *hvpair)
{
	//really inefficient, but I only found this solution

	int pos[2];
	int cpos[2];

	//create a temporary pathlist in which will go *all*
	//the paths of the database
	Pl tpl;
	pl_alloc(&tpl);

	int y = 0;

	//process every line in the db
	while(csv_searchpos(db, hvpair->header, ".*", y, pos) >= 0){
		y++;

		cpos[0] = csv_getheaderindex(db, "class");
		cpos[1] = pos[1];

		pl_fadd(&tpl, csv_ptr(db, cpos), csv_ptr(db, pos));
	}


	regex_t regex;
	regmatch_t match[1];
	int res;
	if((res = regcomp(&regex, hvpair->value, REG_EXTENDED))){ //should care about case in paths
		char err[44];
		regerror(res, &regex, err, 44);
		printf("regex error %d: %s\n", res, err);

		pl_free(&tpl);
		return -2;
	}

	//then add to/remove from the main pathlist all the paths in the temp pathlist that match the regex
	for(int i = 0; i < tpl.max; i++){
		if(tpl.path[i][0] == 0)
			continue;
		if(!regexec(&regex, tpl.path[i], sizeof(match) / sizeof(match[0]), match, 0)){
			if(hvpair->exception)
				pl_rem(pl, tpl.path[i]);
			else
				pl_add(pl, tpl.path[i]);
		}
	}

	regfree(&regex);
	pl_free(&tpl);
	return 0;
}

int
path_search(Stack *res, Hvpair *hv)
{
	return hat_pathsearch(res, hv->value);
}
