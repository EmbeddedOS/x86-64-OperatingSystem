#include <list.h>

/* Public function  ----------------------------------------------------------*/
void ListPushBack(HeadList *list, List *item)
{
    item->next = NULL;

    if (ListIsEmpty(list)) {
        list->next = item;
        list->tail = item;
    } else {
        list->tail->next = item;
        list->tail = item;
    }
}

List *ListPopFront(HeadList *list)
{
    List *item = NULL;
    if (ListIsEmpty(list)) {
        return NULL;
    }

    item = list->next;
    list->next = item->next;

    if(list->next == NULL) {
        list->tail = NULL;
    }

    return item;
}

bool ListIsEmpty(HeadList *list)
{
    return (list->next == NULL);
}

