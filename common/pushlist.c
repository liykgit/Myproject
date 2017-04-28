
#include "pushlist.h"

static void *_list[LIST_SIZE];


void *plist_push(void *ele)
{
    void *victim = _list[LIST_SIZE - 1];
    memcpy(&_list[1], &_list[0], (LIST_SIZE-1) * sizeof(void *));
    _list[0] = ele;

    return victim;
}

void * plist_find(predicate_t func)
{
    int i = 0; 
     
    for(; i < LIST_SIZE; i++)
    {
        void *e = _list[i];   
        if(!e) 
            break;
        
        if(func(e)) 
            return e; 
    }

    return 0;
}

predicate_t find4(void *e) {
   
    return (int)e == 4;
}

predicate_t find1(void *e) {
   
    return (int)e == 1;
}

int main() {

    void * ret = plist_push(1);
    ret = plist_push(2);
    ret = plist_push(3);
    ret = plist_push(4);
    ret = plist_push(5);

    ret = plist_find(find4);
    printf("%d\n", ret);

    ret = plist_find(find1);
    if(!ret)
        printf("null\n");
    else
    printf("%d\n", ret);

    return 0;
}

