#include <stdio.h>
#include <string.h>
#include <time.h>

#include <stack.h>
#include "str.h"
#include "parser.h"
#include "io.h"
#include "hat.h"
#include "date.h"


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

time_t
date_getepoch(const char *date)
{
	//Windows doesn't have strptime(), gaaah

	//don't forget to set the tm structs to 0!
	struct tm tm = {0};
	int year, month, day;
	time_t time;

	if(sscanf(date, "%d-%d-%d", &year, &month, &day) < 3){
		prnte("err: date_getepoch(): sscanf() failed\n");
		return -3;
	}

	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_isdst = -1;
	time = mktime(&tm);

	if(time < 0){
		prnte("err: mktime() failed on date %s\n", date);
		return -4;
	}

	return time;
}
