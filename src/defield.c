#include <stdio.h>
#include "csv.h"
#include "pathlist.h"
#include "parser.h"
#include "defield.h"

int
defield_add(Csv *db, Csvi *csvi, Hvpair *hvpair)
{
	int pos[2];
	int ok = 0;
	int y = 0;

	while (csv_searchpos(db, hvpair->header, hvpair->value, y, pos) >= 0){
		ok = 1;
		y = pos[1]+1;//get past the current result

		//printf("match: \"%s\" at pos %d/%d\n", csv_ptr(db, pos), pos[0], pos[1]);

		if(hvpair->exception)
			csvi_remy(csvi, pos[1]);
		else
			csvi_addpos(csvi, pos);
	}
	if(ok)//at least 1 result
		return 0;
	return -1;
}
