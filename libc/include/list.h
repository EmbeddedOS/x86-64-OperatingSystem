#pragma once

#include <stddef.h>
#include <stdbool.h>

/* Public type ---------------------------------------------------------------*/

struct List
{
    struct List *next;
};

typedef struct List List;

typedef struct
{
    List *next;
    List *tail;
} HeadList;

/* Public function prototype -------------------------------------------------*/

void ListPushBack(HeadList *list, List *item);
List *ListPopFront(HeadList *list);
bool ListIsEmpty(HeadList *list);