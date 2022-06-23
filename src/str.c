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

//remove the char at the given index, then shift all the chars after it back one case
void
strremc(char *s, int index)
{
	for(; s[index] != '\0'; index++){
		s[index] = s[index+1];
	}
}
