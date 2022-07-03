#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "csv.h"
#include "str.h"
#include <unistd.h>


static int getvalueindex(Csv *, int, char *, int);

Csv *
csv_load(char* file)
{
	FILE * f;
	//"r" should make it so that any CR+LF are converted to LF, but strchr() still reports the presence
	//of carriage returns if all the lines of the file end in CR+LF... I should add a notice stating that
	//csv files must have their end of lines be LF only.
	f = fopen(file, "r");
	if(f == NULL){
		fprintf(stderr, "Error: could not open file %s\n", file);
		return NULL;
	}

	//get the size of the file and get its content
	fseek(f, 0, SEEK_END);
	int fs = ftell(f);
	rewind(f);

	Csv *c = malloc(sizeof(Csv));
	c->hcount = 0;
	c->lcount = 0;
	c->buf = malloc(fs);
	fread(c->buf, 1, fs, f);


	//get the number of newlines in the file (needed to alloc pointers later)
	for (int a = 0; a < fs; a++){
		if (c->buf[a]=='\n')
			c->lcount++;
	}
	c->lcount--; //the header pointers are stored separately (join?)


	//put a '\0' instead of the first newline to separate the headers from the content
	char *content = strchr(c->buf, '\n');
	content[0]='\0';
	content++;


	//get the header fields

	char *b = c->buf, *s = b;
	while(b != NULL){
		s = wstrsep(&b, ";");
		c->headers[c->hcount++] = s;
	};


	//allocate all the pointers for the fields

	c->ptrs = malloc(sizeof(char*) * c->hcount); //x columns (the number of headers)
	for(int a = 0; a < c->hcount; a++)
		c->ptrs[a] = malloc(sizeof(char*) * c->lcount); //y rows (the number of newlines)


	//get the other fields

	int x = 0, y = 0;
	b = content;
	s = b;
	while(1){ //to infinity... and beyond!
		s = wstrsep(&b, ";\n");

		//if the match is "\0\0", we don't want to take it into account (\n\0 -> \0\0 -> next round: match! even if there's nothing there)
		//this can happen at the very end of the file if it ends with a newline
		if(b == NULL)
			break;

		//printf("found (%d/%d) %d \"%s\"\n", x, y, c->lcount, s);
		c->ptrs[x++][y] = s;

		if(x >= c->hcount){
			x = 0;
			y++;
		}
		//garde-fous, Windows liked to go past this because of errors in the csv file
		//maybe I should've read the csv file line by line to make it easier to me...
		if(y >= c->lcount)
			break;

	}
	return c;
}

void
csv_unload(Csv *c)
{
	for(int i = 0; i < c->hcount; i++)
		free(c->ptrs[i]);
	free(c->ptrs);
	free(c->buf);
	free(c);
}

//return the X/Y position of the first match between header/pattern.
//should give the start index for recursion...
int
csv_searchpos(Csv *c, char *header, char *pattern, int starty, int *pos)
{
	int hi = csv_getheaderindex(c, header);
	if(hi < 0)
		return hi;

	int vi = getvalueindex(c, hi, pattern, starty);
	if(vi < 0)
		return vi;

	pos[0] = hi;
	pos[1] = vi;

	return 0;
}

int
csv_searchline(Csv *c, char *header, char *pattern, int y)
{
	int x = csv_getheaderindex(c, header);
	if(x == -1)
		return -1;

	if (strcmp(c->ptrs[x][y], pattern) == 0)
		return x;

	//fprintf(stderr, "Error: pattern \"%s\" not found in the specified line.\n", pattern);
	return -1;
}

int
csv_getheaderindex(Csv *c, char *header)
{
	for(int i = 0; i < c->hcount; i++)
		if (strcmp(c->headers[i], header) == 0)
			return i;

	//fprintf(stderr, "Error: the header wasn't found in the Csv struct.\n");
	return -1;
}

char *
csv_ptr(Csv *c, int *pos)
{
	return c->ptrs[pos[0]][pos[1]];
}

static int
getvalueindex(Csv *c, int xindex, char *pattern, int starty)
{
	regex_t regex;
	regmatch_t match[1];

	int res;
	if((res = regcomp(&regex, pattern, REG_ICASE | REG_EXTENDED))){ //don't care about case
		char err[44];
		regerror(res, &regex, err, 44);
		printf("regex error %d: %s\n", res, err);
		return -2;
	}

	for(int i = starty; i < c->lcount; i++){
		if(!regexec(&regex, c->ptrs[xindex][i], sizeof(match) / sizeof(match[0]), match, 0)){
			regfree(&regex);
			return i;
		}
	}

	//fprintf(stderr, "Error: pattern \"%s\" not found in the Csv struct.\n", pattern);
	regfree(&regex);
	return -1;
}

