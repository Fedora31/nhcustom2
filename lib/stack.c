#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <stack.h>


//initialize a stack. realloccount is the number of members to
//add to/remove from the array when it needs to be reallocated.
//size is the size of an item.
int
stack_init(Stack *s, int nmemb, int realloccount, long size)
{
	s->used = calloc(nmemb, sizeof(int));
	s->stack = calloc(nmemb, size);
	s->stacki = 0;
	s->count = 0;
	s->max = nmemb;
	s->slotsize = size;
	s->realloccount = realloccount;
	return 0;
}

void
stack_free(Stack *s)
{
	free(s->used);
	free(s->stack);
}

void *
stack_getslot(Stack *s, int *i)
{
	if(s->count >= s->max)
		stack_realloc(s, s->max + s->realloccount);

	for(*i = 0; *i < s->max; *i+=1)
		if(!s->used[*i])
			return s->stack + *i * s->slotsize;
	return NULL;
}

void *
stack_get(Stack *s, int i)
{
	if(i < 0 || i >= s->max)
		return NULL;
	return s->stack + i * s->slotsize;
}

//same as stack_get(), but returns NULL if the asked
//slot isn't used.
void *
stack_getused(Stack *s, int i)
{
	if(i < 0 || i >= s->stacki || s->used[i] == 0)
		return NULL;
	return s->stack + i * s->slotsize;
}

//Return the next used item in the stack starting at index i.
//id is modified to be one past the found item.
//If the end of the stack is attained or i isn't valid, NULL
//is returned.
void *
stack_getnextused(Stack *s, int *i)
{
	if(*i < 0 || *i >= s->stacki)
		return NULL;

	int ok = 0;
	for(; *i < s->stacki; *i+=1){
		if(s->used[*i]){
			ok = 1;
			break;
		}
	}
	*i+=1;
	if(ok)
		return s->stack + (*i-1) * s->slotsize;
	return NULL;
}

void
stack_setused(Stack *s, int i)
{
	if(i < 0 || i >= s->max || s->used[i] != 0)
		return;
	s->used[i] = 1;
	s->count++;
	if(s->stacki <= i)
		s->stacki = i + 1;
}

//add the data to the first available slot.
int
stack_add(Stack *s, void *p)
{
	int i = 0;
	if(s->count >= s->max){
		stack_realloc(s, s->max + s->realloccount);
		i = s->count;
	}
	for(; i < s->max; i++)
		if(s->used[i] == 0)
			return stack_addtoslot(s, p, i);
	return -1;
}

int
stack_addtoslot(Stack *s, void *p, int id)
{
	if(id < 0)
		return -2;

	if(id >= s->max)
		stack_realloc(s, id + s->realloccount);

	//printf("adding to %p + %d * %lu = %p\n", s->stack, id, s->slotsize, s->stack + id * s->slotsize);
	memcpy(s->stack + id * s->slotsize, p, s->slotsize);
	s->used[id] = 1;

	s->count++;
	if(s->stacki <= id)
		s->stacki = id + 1;


	return 0;
}

void
stack_rem(Stack *s, int i)
{
	if(i < 0 || i >= s->max || s->used[i] == 0)
		return;

	s->used[i] = 0;
	s->count--;

	//if the index was the one behind stacki, reposition stacki to
	//be one past the "new" last index
	if(s->stacki - 1 == i)
		for(; s->used[s->stacki - 1] == 0 && s->stacki >= 1; s->stacki--);

	//if there is a significant enough space at the end of the stack, realloc
	//it a smaller size
	// /!\ it's ">" not ">=" as we still want at least 1 slot to be available. It
	//     can cause crashes when free()ing memory otherwise.
	if(s->max - s->stacki > s->realloccount)
		stack_realloc(s, s->max - s->realloccount);
}

int
stack_realloc(Stack *s, int nmax)
{
	void *stack;
	int *used;

	//TODO: return a specific int to know what failed
	if((stack = realloc(s->stack, nmax * s->slotsize)) == NULL){
		return -1;
	}
	if((used = realloc(s->used, nmax * sizeof(int))) == NULL){
		return -1;
	}
	s->stack = stack;
	s->used = used;

	if(nmax > s->max){
		//Random crashes were happening without the cast to char* to s->used. I think it's because
		//it was silently doing s->used + sizeof(int) * s->max, which caused a buffer overflow.
		//and since I think glibc treats void as a char in this situation, it didn't overflow
		//there.
		memset(((char *)s->stack) + s->max * s->slotsize, 0, (nmax - s->max) * s->slotsize);
		memset(((char *)s->used) + s->max * sizeof(int), 0, (nmax - s->max) * sizeof(int));
	}

	s->max = nmax;
	return 0;
}
