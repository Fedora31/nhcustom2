#include <stdio.h>
#include <string.h>
#include "csv.h"
#include "pathlist.h"
#include "parser.h"
#include "date.h"

static void formatdate(char *);

#define Y11 0
#define Y12 1
#define Y13 2
#define Y14 3
#define M11 5
#define M12 6
#define D11 8
#define D12 9
#define Y21 11
#define Y22 12
#define Y23 13
#define Y24 14
#define M21 16
#define M22 17
#define D21 19
#define D22 20

int
date_add(Csv *db, Csvi *csvi, Hvpair *hvpair)
{
	printf("DATE: %s\n", hvpair->value);

	char t[] = "0000-00-00/0000-12-31";
	char *d = hvpair->value;

	memcpy(t, d, strlen(d));
	printf("DATE: %s\n", t);

	formatdate(t);

	printf("DATE: %s\n", t);

	return 0;
}

static void
formatdate(char *d)
{
	//copy the date code from the original nhcustom
}
