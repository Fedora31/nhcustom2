/*
 * requires: stack.h
 */

/*Extra functions for stacks containing **chars.*/

void strstack_add(Stack *, char **);
void strstack_remfrom(Stack *, Stack *);
void strstack_addto(Stack *, Stack *);
void strstack_filter(Stack *, Stack *);
int strstack_contain(Stack *, char *);
