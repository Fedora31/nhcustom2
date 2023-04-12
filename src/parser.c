#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "arg.h"
#include "str.h"

#include <stack.h>
#include "strstack.h"
#include "hat.h"
#include "parser.h"
#include "date.h"
#include "copy.h"
#include "io.h"

//global stack, containing paths gotten during the
//execution of the configuration file
static Stack gstack;

//stack containing lists
static Stack lists;

//remove flag (if not 1, means keep flag)
static int removeflag = 1;

typedef struct List{
	char *name;
	Stack stack;
}List;

//get from the given string the header and the value(s) to search in it
//the given string is modified.
static int gethv(char **, Hvpair *);
static int gethead(char *, const char *);
static int getval(char *, const char *);

//redirect the header/values to the function suited to handle them
static int redirect(Stack *, Hvpair *);
static int copy(char *);
static int modifystack(Stack *, Stack *, int, int);
static int iscmd(char *);
static int execcmd(char *);
static int newlist(char *, char *);
static int listsearch(Stack *, char *);
static int parsestr(char *, Stack *);


int
parser_init(int flag)
{
	prnt("initializing the parser...\n");
	removeflag = flag;
	stack_init(&gstack, 64, 512, sizeof(char*));
	stack_init(&lists, 1, 1, sizeof(List));

	if(!direxist(arg_getinput())){
		prnte("err: input directory \"%s\" doesn't exist\n", arg_getinput());
		return -1;
	}
	return 0;
}

void
parser_clean(void)
{
	//free all lists
	List *c;
	for(int i = 0; (c = stack_getnextused(&lists, &i)) != NULL;){
		free(c->name);
		stack_free(&c->stack);
		stack_rem(&lists, i-1);
	}

	stack_free(&lists);
	stack_free(&gstack);
}

int
parser_exec(void)
{
	prnt("copying found files...\n");

	if(removeflag){ //remove flag, copy the paths we found
		char **path_p;
		for(int i = 0; (path_p = stack_getnextused(&gstack, &i)) != NULL;){
			if(print)
				printf("%s\n", *path_p);
			if(!norun)
				copy(*path_p);
		}
	}else{ //keep flag, copy the paths we haven't found
		char *path;
		Stack tmp;
		char str[HAT_PATHLEN] = {0};
		strcpy(str, arg_getinput());

		stack_init(&tmp, 1024, 1024, HAT_PATHLEN);
		prnt("scanning the input folder...\n");

		if(getallfiles(&tmp, str)<0)
			return -1;

		for(int i = 0; (path = stack_getnextused(&tmp, &i)) != NULL;)
			if(strstack_contain(&gstack, path)<0){
				if(print)
					printf("%s\n", path);
				if(!norun)
					copy(path);
			}

		stack_free(&tmp);
	}

	return 0;
}

//check if the given string is a command
//A command string must not contain a colon, or must have a space before it.
static int
iscmd(char *s)
{
	char *spacei, *coloni;

	if((coloni = strchr(s, ':')) == NULL) //if no colon, it's a command
		return 1;
	if((spacei = strchr(s, ' ')) == NULL) //if no space but a colon, it's not a command
		return 0;
	if(spacei < coloni)
		return 1;
	return 0;
}

static int
execcmd(char *s)
{
	char *c, *args;
	int r = 0;
	int len = strlen(s);
	if((c = malloc(len + 1)) == NULL){
		prnte("fatal: execcmd() malloc() failed\n");
		return -1;
	}
	strcpy(c, s);

	if((args = strchr(c, ' ')) == NULL)
		args = &c[len]; //make args point to '\0'
	else{
		args[0] = '\0';
		args++; //go past the \0
	}

	if(strcmp(c, "keep") == 0){
		prnt("mode: keep listed parameters\n");
		removeflag = 0;
	}else if(strcmp(c, "remove") == 0){
		prnt("mode: remove listed parameters\n");
		removeflag = 1;
	}else if(strcmp(c, "list") == 0){
		char *str = strchr(args, ' ');
		str[0] = '\0';
		str++;
		r = newlist(args, str);
	}

	free(c);
	return r;
}


//Create a new list "name" with the content from str.
//If a list with the same name already exists, it's replaced.
static int
newlist(char *name, char *str)
{
	List tmp;
	int len;
	stack_init(&tmp.stack, 64, 512, sizeof(char*));

	len = strlen(name);
	if((tmp.name = malloc(len+1)) == NULL){
		prnte("fatal: parseline() malloc() failed\n");
		stack_free(&tmp.stack);
		return -1;
	}
	strcpy(tmp.name, name);

	if(parsestr(str, &tmp.stack) < 0){
		stack_free(&tmp.stack);
		return -1;
	}

	//if the list already exists, remove it
	List *c;
	for(int i = 0; (c = stack_getnextused(&lists, &i)) != NULL;){
		if(strcmp(c->name, tmp.name) == 0){
			free(c->name);
			stack_free(&c->stack);
			stack_rem(&lists, i-1);
		}
	}

	//add the new list
	stack_add(&lists, &tmp);

	prnt("new list \"%s\" with content \"%s\"\n", tmp.name, str);

	return 0;
}

//copy entries from the specified list to res
static int
listsearch(Stack *res, char *name)
{
	List *c;
	for(int i = 0; (c = stack_getnextused(&lists, &i)) != NULL;){
		if(strcmp(c->name, name) == 0){
			strstack_addto(res, &c->stack);
			return 0;
		}
	}
	prnte("err: list \"%s\" not found\n", name);
	return -1;
}

int
parseline(char *line)
{
	if(strlen(line) == 0 || line[0] == '#')
		return 0;

	if(iscmd(line))
		return execcmd(line);

	return parsestr(line, &gstack);
}

static int
parsestr(char *s, Stack *stack)
{
	char *l;
	int len = strlen(s);
	if((l = malloc(len+1)) == NULL){
		prnte("fatal: parseline() malloc() failed\n");
		return -1;
	}
	strcpy(l, s);

	char *li = l; //string index
	Hvpair hvpair;
	Stack lstack; //local stack for the string

	stack_init(&lstack, 64, 128, sizeof(char*));

	int exception; //holds if the first value of the string is an exception
	int filter; //same for the filter
	int res;
	int i = 0;
	while(1){


		hvpair.exception = 0;
		hvpair.filter = 0;

		if(li[0] == ')')
			break;

		if(li[0] == '!')
			hvpair.exception = 1;
		else if(li[0] == '}')
			hvpair.filter = 1;

		/*Go past the flag. But on the first hvpair, no flag means the add flag and
		 *the string isn't incremented.
		 */
		if(i != 0 || (li[0] == '!' || li[0] == '}' || li[0] == '+'))
			li++;



		//the matches of the first value of the string will always be added to the stack, but it's exception
		//status is saved and will determine if the lstack will be added to or deleted from the stack
		if(i == 0) {
			prnt("%s\n", s);
			exception = hvpair.exception;
			filter = hvpair.filter;
			hvpair.exception = 0;
			hvpair.filter = 0;
		}



		if(li[0] == '('){
			int offset;
			Stack cstack;
			stack_init(&cstack, 64, 128, sizeof(char*));

			li++;
			if((offset = parsestr(li, &cstack)) < 0){
				printf("child failed\n");
				return -1;
			}
			li+=offset+1;

			modifystack(&lstack, &cstack, hvpair.exception, hvpair.filter);
			stack_free(&cstack);

		}else{
			if((res = gethv(&li, &hvpair)) == -1)
				return -1;


			//fill the lstack with the found matches
			if(redirect(&lstack, &hvpair) < 0){
				stack_free(&lstack);
				free(l);
				return -1;
			}

		}
		i++;
		if(res == -2) //no more values to get from the string
			break;
		if(strlen(li) == 0)
			break;
	}


	//modify the stack according to the lstack and the modifiers
	modifystack(stack, &lstack, exception, filter);
	int offset = li - l;
	stack_free(&lstack);
	free(l);
	return offset;
}

static int
getval(char *dest, const char *str)
{
	int i, e;

	if(str[0] == '"')
		for(i = 1, e = 0; str[i] != '\0'; i++){
			switch(str[i]){
			case '\\':
				i++;
				break;
			case '"':
				i++;
				goto end;
			}
			dest[e++] = str[i];
		}
	else
		for(i = 0, e = 0; str[i] != '\0'; i++){
			switch(str[i]){
			case '\\':
				i++;
				break;
			case ':':
			case '+':
			case '!':
			case '}':
			case ')':
				goto end;
			}
			dest[e++] = str[i];
		}
end:
	dest[e++] = '\0';
	return i;
}

static int
gethead(char *dest, const char *str)
{
	int len;
	char *colon = strchr(str, ':');
	if(!colon)
		return -1;
	if((len = colon - str) > HAT_VALLEN-1)
		return -1;

	strncpy(dest, str, len);
	dest[len] = '\0';
	return len+1; /*for the colon*/
}

//store the header:value pair and the exception status in the hvpair struct
//return -1 if an error occured, -2 if all the given string has been read
static int
gethv(char **s, Hvpair *hvpair)
{
	int res;

	if((res = gethead(hvpair->header, *s)) == -1)
		return -1;
	*s += res;
	if((res = getval(hvpair->value, *s)) == -1)
		return -1;
	*s += res;

	if(strlen((*s)) == 0)
		return -2;

	return 0;
}

static int
redirect(Stack *res, Hvpair *hvpair)
{
	int ok;
	Stack tmp;
	stack_init(&tmp, 64, 512, sizeof(char*));

	if(strcmp(hvpair->header, "date") == 0)
		ok = date_search(&tmp, hvpair);
	else if(strcmp(hvpair->header, "path") == 0)
		ok = hat_pathsearch(&tmp, hvpair->value);
	else if(strcmp(hvpair->header, "list") == 0)
		ok = listsearch(&tmp, hvpair->value);
	else
		ok = hat_defsearch(&tmp, hvpair->header, hvpair->value);

	if(ok<0){
		stack_free(&tmp);
		return -1;
	}

	modifystack(res, &tmp, hvpair->exception, hvpair->filter);

	stack_free(&tmp);
	return 0;
}

//copy the file to the new location (TODO: move this function)
static int
copy(char *ofile)
{
	//ofile: <input>/models/...
	//nfile: <output>/models/...
	//dirs: <output>/models/player/... up until the last folder in the path

	char nfile[HAT_PATHLEN] = {0}, dirs[HAT_PATHLEN] = {0};
	char *offset;
	strncpy(nfile, ofile, HAT_PATHLEN-1);
	strswap(nfile, arg_getinput(), arg_getoutput());
	strncpy(dirs, nfile, HAT_PATHLEN-1);
	offset = strrchr(dirs, '/');

	if(offset == NULL)
		return -1;
	offset[0] = '\0';

	if(makedirs(dirs, 0755) < 0)
		return -1;
	if(fcopy(ofile, nfile) < 0)
		return -1;
	return 0;
}

//modify the stack according to the other stack and the modifiers
static int
modifystack(Stack *this, Stack *with, int exception, int filter)
{
	if(exception){
		strstack_remfrom(this, with);
	}else if(filter){
		strstack_filter(this, with);
	}else{
		strstack_addto(this, with);
	}
	return 0;
}
