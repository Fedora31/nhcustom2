#include <stdio.h>
#include <stdlib.h>
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

//separate a string and returns an array of pointers to the resulting substrings.
//the given string is modified, and the returned array must be freed after use.
char **
strsplit(char *in, char *seps)
{
	char **res = malloc(sizeof(char*) * STRL_LEN + 1); //+1 so there will always be a NULL at the end of the list
	for(int i = 0; i < STRL_LEN + 1; i++)
		res[i] = NULL;

	for(int i = 0; in != NULL && i < STRL_LEN; i++)
		res[i] = wstrsep(&in, seps);

	return res;
}

void
strswapall(char *s, char *pattern, char *new)
{
	char *subs;
	while((subs = strstr(s, pattern)) != NULL)
		strswap(s, pattern, new);
}

void
strswap(char *s, char *pattern, char *new)
{
	char *subs;
	if((subs = strstr(s, pattern)) == NULL)
		return;

	//temporary string
	char *tmp = calloc(STR_LEN, sizeof(char)); //no more than STR_LEN then, watch out :)))))

	//copy the first bit of the string, the new part, then the end of the string
	memcpy(tmp, s, subs - s);
	strcat(tmp, new);
	strcat(tmp, subs+strlen(pattern));

	memcpy(s, tmp, strlen(tmp)+1);
	free(tmp);
}
