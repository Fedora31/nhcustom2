#include <stdio.h>
#include <string.h>
#include <stack.h>
#include "strstack.h"

/*WARNING: given stacks MUST only contain **chars.
 *TODO: this has to be made clearer.
 */


void
strstack_remfrom(Stack *from, Stack *this)
{
	char **s;
	int id;
	for(int i = 0; (s = stack_getnextused(this, &i)) != NULL;)
		if((id = strstack_contain(from, *s))>=0)
			stack_rem(from, id);
}

void
strstack_addto(Stack *to, Stack *from)
{
	char **item;
	for(int i = 0; (item = stack_getnextused(from, &i)) != NULL;)
		stack_add(to, item);
}

//if a string exists in from but not in this, remove it
void
strstack_filter(Stack *from, Stack *this)
{
	char **s;
	int id;
	for(int i = 0; (s = stack_getnextused(from, &i)) != NULL;)
		if((id = strstack_contain(this, *s))<0)
			stack_rem(from, i-1);
}

int
strstack_contain(Stack *in, char *str)
{
	char **match;
	for(int i = 0; (match = stack_getnextused(in, &i)) != NULL;)
		if(strcmp(*match, str) == 0)
			return i-1;
	return -1;
}
