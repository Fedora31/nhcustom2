#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"
#include "str.h"


Csv *
csv_load(char* file)
{
	printf("reading csv file...");

	FILE * f;
	f = fopen(file, "rb");
	if(f==NULL){
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
	c->lcount--;


	//put a NUL byte at the end of the first line
	char *content = strpbrk(c->buf, "\r\n");
	int i;
	for(i=0;content[i]=='\n' || content[i]=='\r';i++){
		content[i]='\0';
	}
	content+=i;//pass the NUL byte(s) added


	//get the header fields

	char *b = c->buf, *s = b;
	while(b!=NULL){
		s = wstrsep(&b, ";\n\r");
		//printf("found %s\n", s);
		c->headers[c->hcount++] = s;
	};


	//allocate all the pointers for the fields

	c->ptrs = malloc(sizeof(char*) * c->hcount); //row
	for(int a = 0; a < c->hcount; a++)
		c->ptrs[a] = malloc(sizeof(char*) * c->lcount); //column (-1 bc the header ptrs are stored elsewhere)


	//get the other fields

	int x = 0, y = 0;
	b = content;
	s = b;
	while(b!=NULL){
		s = wstrsep(&b, ";\n\r");
		if(b!=NULL){ //if the match is "\0\0", we don't want to take it into account
			//printf("found (type %d) %s\n", e, s);
			c->ptrs[x++][y] = s;
			if(x >= c->hcount){
				x = 0;
				y++;
			}
		}
	};
	printf("done\n");
	return c;
}

char *
csv_search(Csv *c, char *header, char *pattern)
{

	/////temporary, this returns only the first result but there could be more than one, should
	/////return a Csvres struct which is a linked list

	int hi = -1;
	for(int i = 0; i < c->hcount; i++){
		if (strcmp(c->headers[i], header) == 0) {
			hi = i;
			break;
		}
	}
	if(hi == -1){
		fprintf(stderr, "Error: the header wasn't found in the Csv struct.\n");
		return NULL;
	}

	char *result = NULL;
	for(int i = 0; i < c->lcount-1; i++){ //??? if I don't do -1, the program crashes. There's a mistake somewhere...

		if(strcmp(c->ptrs[hi][i], pattern) == 0){
			result = c->ptrs[hi][i];
			break;
		}
	}
	if(result == NULL)
		fprintf(stderr, "Error: pattern not found in the Csv struct.\n");

	return result;
}
