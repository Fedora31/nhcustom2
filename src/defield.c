#include <stdio.h>
#include "csv.h"
#include "parser.h"
#include "pl.h"
#include "defield.h"

int
defield_add(Csv *db, Pl* pl, Hvpair *hvpair)
{
	int pos[2];
	int cpos[2];

	int y = 0;

	while (csv_searchpos(db, hvpair->header, hvpair->value, y, pos) >= 0){
		y = pos[1]+1;//get past the current result

		//printf("match: \"%s\" at pos %d/%d\n", csv_ptr(db, pos), pos[0], pos[1]);

		pos[0] = csv_getheaderindex(db, "path");
		cpos[0] = csv_getheaderindex(db, "class");
		cpos[1] = pos[1];

		if(hvpair->exception)
			pl_frem(pl, csv_ptr(db, cpos), csv_ptr(db, pos));
		else
			pl_fadd(pl, csv_ptr(db, cpos), csv_ptr(db, pos));

	}
	return 0;
}
