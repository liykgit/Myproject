#ifndef _PUSH_LIST_H
#define _PUSH_LIST_H

#define LIST_SIZE 4

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*predicate_t)(void *element, void *arg);

void *plist_push(void *ele);

void * plist_find(predicate_t func, void *arg);

#ifdef __cplusplus
}
#endif

#endif
