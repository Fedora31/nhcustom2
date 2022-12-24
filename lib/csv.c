#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <csv.h>


//Please be kind and don't include 50 trailing newlines in
//your csv files, one is enough (and even unnecessary). The
//memory allocated is based on the number of newlines.

Csv *
csvload(char *path, char sep)
{
	Csv *csv;
	FILE *f;
	int size;

	csv = malloc(sizeof(Csv));
	//there should at least be 1 column in the file (if no separator is found)
	csv->columns = 1;
	csv->lines = 0;

	if((f = fopen(path, "rb")) == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	csv->buf = malloc(size+1);
	fread(csv->buf, 1, size, f);
	csv->buf[size] = '\0';

	//get the number of lines and columns
	for(int i = 0; i < size; i++)
		if(csv->buf[i] == '\n')
			csv->lines++;
	for(int i = 0; i < size && csv->buf[i] != '\n'; i++)
		if(csv->buf[i] == sep)
			csv->columns++;

	//allocate the cells' pointers
	csv->cells = malloc(sizeof(char**) * csv->columns);
	for(int i = 0; i < csv->columns; i++)
		csv->cells[i] = malloc(csv->lines * sizeof(char*));


	//initialise all the pointers to point to the end of the buffer,
	//which is a '\0' char
	for(int x = 0; x < csv->columns; x++)
		for(int y = 0; y < csv->lines; y++)
			csv->cells[x][y] = &csv->buf[size];


	//point the pointers and remove the separators
	//Info: some pointers may not receive a value here, depending if the
	//file has trailing newlines. But since they are initialised to point
	//to a '\0' earlier, they shouldn't pose any problem. Let's just hope
	//nobody tries to search for an empty string in the csv file, as it may
	//or may not match... And return one of those empty strings. This is
	//something to improve.

	int x = 0, y = 0, lastpos = 0;
	char c;
	for(int i = 0; i < size; i++){
		c = csv->buf[i];

		if(c == '\n' || c == sep){
			csv->cells[x][y] = &csv->buf[lastpos];
			lastpos = i+1;
			csv->buf[i] = '\0';

			if(c == sep)
				x++;
			else{
				x = 0;
				y++;
			}
		}
	}
	return csv;
}

void
csvdel(Csv *csv)
{
	free(csv->buf);
	for(int i = 0; i < csv->columns; i++)
		free(csv->cells[i]);
	free(csv->cells);
	free(csv);
}

//h1: the header of the column containing key
//key: the value we know
//h2: the header of the column whose value we want
char *
csvs(Csv *csv, char *h1, char *key, char *h2)
{
	int x = 0, x2 = 0, y = 0, foundx = 0, foundx2 = 0, foundy = 0;

	for(;x < csv->columns; x++)
		if(strcmp(csv->cells[x][0], h1) == 0){
			foundx = 1;
			break;
		}
	if(!foundx)
		return NULL;

	for(;x2 < csv->columns; x2++)
		if(strcmp(csv->cells[x2][0], h2) == 0){
			foundx2 = 1;
			break;
		}
	if(!foundx2)
		return NULL;
	for(;y < csv->lines; y++){
		if(strcmp(csv->cells[x][y], key) == 0){
			foundy = 1;
			break;
		}
	}
	if(!foundy)
		return NULL;
	return csv->cells[x2][y];
}
