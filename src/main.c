#include <stdio.h>
#include "csv.h"

int
main(int argc, char **args)
{
	printf("-- nhcustom2 --\n");

	Csv *db = csv_load("database.csv");

	char *t = csv_search(db, "hat", "Electronic Sports League");
	printf("match: %s\n", t);

	return 0;
}
