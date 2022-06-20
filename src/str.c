#include <string.h>
#include "str.h"

//strsep is here because windows doesn't possess it
char *
wstrsep(char **strp, char *delim)
{
	char *start = *strp, *p;
	p = (start!=NULL)?strpbrk(start, delim):NULL;
	if(p==NULL)
		*strp = NULL;
	else {
		*p = '\0';
		*strp = p+1;
	}
	return start;
}
