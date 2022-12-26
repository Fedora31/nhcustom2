#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include "str.h"
#include "csv.h"
#include "parser.h"
#include "pl.h"
#include <stack.h>
#include "date.h"
#include "hat.h"



//static void formatdate(char *, char *);

/*
int
date_add(Csv *db, Pl *pl, Hvpair *hvpair)
{
	char pattern[] = "^([0-9]{4})(-[0-9]{2})?(-[0-9]{2})?(/)?([0-9]{4})?(-[0-9]{2})?(-[0-9]{2})?$";
	regex_t regex;
	regmatch_t match[8];
	int res;

	//if I don't set it to 0, an char from nowhere sometimes appear in the 1st position of the string. ??
	char date[22] = {0}; //0000-00-00/0000-00-00\0

	if((res = regcomp(&regex, pattern, REG_EXTENDED))){
		char err[44];
		regerror(res, &regex, err, 44);
		printf("regex error %d: %s\n", res, err);
		return -2;
	}

	if(regexec(&regex, hvpair->value, sizeof(match) / sizeof(match[0]), match, 0)){
		regfree(&regex);
		return -1;
	}
	regfree(&regex);

	//add any missing years, months, or days
	formatdate(hvpair->value, date);

	//printf("formatted date: %s\n", date);

	//separate the dates and parse them to transform them into seconds since the epoch.
	//windows and UNIX epoch are different, but it shouldn't matter in this case.

	time_t time1, time2;
	char *d1, *d2, *ptr = date;
	d1 = wstrsep(&ptr, "/");
	d2 = ptr;
	if((time1 = date_getepoch(d1)) < 0)
		return -1;
	if((time2 = date_getepoch(d2)) < 0)
		return -1;

	//printf("time1 %ld\ntime2 %ld\n", time1, time2);

	int pos[2];
	int cpos[2];
	int y = 0;

	//process every line in the db
	while (csv_searchpos(db, hvpair->header, ".*", y, pos) >= 0){
		y++;

		time_t t;
		if((t = date_getepoch(csv_ptr(db, pos))) < 0){
			//maybe the error is due to a date being before the UNIX or Windows epoch?
			printf("Warning: bad date in the csv file, pos %d/%d\n", pos[0], pos[1]);
			continue;
		}

		pos[0] = csv_getheaderindex(db, "path");
		cpos[0] = csv_getheaderindex(db, "class");
		cpos[1] = pos[1];

		if(t >= time1 && t <= time2){
			if(hvpair->exception)
				pl_frem(pl, csv_ptr(db, cpos), csv_ptr(db, pos));
			else
				pl_fadd(pl, csv_ptr(db, cpos), csv_ptr(db, pos));
		}
	}
	return 0;
}
*/

int
date_search(Stack *res, Hvpair *hv)
{
	char date[HAT_VALLEN] = {0};
	time_t from, to;
	char *d1, *d2, *ptr = date;

	strncpy(date, hv->value, HAT_VALLEN-1);
	d1 = wstrsep(&ptr, "/");
	d2 = ptr;

	if((from = date_getepoch(d1)) < 0)
		return -1;
	if((to = date_getepoch(d2)) < 0)
		return -1;

	return hat_datesearch(res, from, to);
}








/*static void
formatdate(char *d, char *fd)
{
	//I'm aware that this is the most basic and useless code ever written to format a date.
	//the code from the original nhcustom was waay more complicated that that and
	//hielded bad results anyway. For the sake of simplicity, I give the burden of creating
	//correct dates to the user.

	memcpy(fd, "0000-01-01/0000-12-31", 22);
	memcpy(fd, d, strlen(d)); //get the year

	//copy the first year over to the second year if it's empty
	if(fd[11] == '0' && fd[12] == '0' && fd[13] == '0' && fd[14] == '0')
		memcpy(fd + 11, d, 4);
}*/

time_t
date_getepoch(char *date)
{
	//Windows doesn't have strptime(), gaaah

	//don't forget to set the tm structs to 0!
	struct tm tm = {0};
	int year, month, day;
	time_t time;

	if(sscanf(date, "%d-%d-%d", &year, &month, &day) < 3){
		fprintf(stderr, "Error: time1 sscanf()\n");
		return -3;
	}
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_isdst = -1;
	time = mktime(&tm);
	if(time < 0){
		fprintf(stderr, "err: mktime() failed\n");
		return -4;
	}

	return time;
}
