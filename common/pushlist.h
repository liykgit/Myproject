#ifndef _PUSH_LIST_H
#define _PUSH_LIST_H

#define LIST_SIZE 4

typedef int (*predicate_t)(void *element);

void *plist_push(void *ele);

void * plist_find(predicate_t func);


#endif
