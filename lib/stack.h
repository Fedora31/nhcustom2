/*
 * libstack allows to create "stacks" of data and keeps track of the amount of
 * data, slot used, etc. Data which is sent to a stack will always be accessible
 * with the same index; it is never moved. This means that one needs to be mindful
 * of the order of the data: if it will never be freed, it ought to be placed first
 * in the stack to allow libstack to free memory later on.
 */

typedef struct Stack{
	int *used;
	void *stack;
	int max;
	int count;
	int stacki;			//the stack index, index just after the last used item
	int realloccount;	//the amount of items that will be added or removed when reallocating
	long slotsize;		//the size of an item in the stack (like sizeof)
}Stack;

int stack_init(Stack *, int, int, long);
int stack_realloc(Stack *, int);
void stack_free(Stack *);

void *stack_getslot(Stack *, int *);
void *stack_get(Stack *, int);
void *stack_getused(Stack *, int);
void *stack_getnextused(Stack *, int *);
int stack_add(Stack *, void *);
int stack_addtoslot(Stack *, void *, int);
void stack_rem(Stack *, int);
void stack_setused(Stack *, int);
