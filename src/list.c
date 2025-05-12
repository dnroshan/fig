#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"
#include "misc.h"


static struct FigListItem *create_new_item(void *data);
static struct FigListItem *item_at(FigList *list, int index);

FigList *
fig_list_new()
{
    FigList *list;

    list = malloc(sizeof *list);
    if (!list)
        fig_panic("failed allocating memeory");

    list->n = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

void
fig_list_destroy(FigList *list)
{
    fig_list_clear(list);
    free(list);
}

void
fig_list_append(FigList *list, void *data)
{
    struct FigListItem *new_item;

    new_item = create_new_item(data);

    if (!list->n) {
        list->head = new_item;
	    list->tail = new_item;
    } else {
	    list->tail->next = new_item;
	    new_item->prev = list->tail;
	    list->tail = new_item;
   }

   list->n++;
}

void
fig_list_insert(FigList *list, int index, void *data)
{
    assert(list);
    assert(index >= 0);
    assert(index < list->n);

    struct FigListItem *new_item, *item;
    new_item = create_new_item(data);
    new_item->data = data;

    item = item_at(list, index);
    new_item->next = item;
    new_item->prev = item->prev;
    item->prev = new_item;

    list->n++;
}

void *
fig_list_at(FigList *list, int index)
{
    assert(list);
    assert(index >= 0);
    assert(index < list->n);

    struct FigListItem *item;

    item = item_at(list, index);
    if (!item)
        return NULL;

    return item->data;
}

void
fig_list_remove(FigList *list, int index)
{
    assert(list);
    assert(index >= 0);
    assert(index < list->n);

    struct FigListItem *item;

    item = item_at(list, index);
    if (!item)
        fig_panic("index overflow");

    struct FigListItem *prev = item->prev,
                    *next = item->next;

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;

    if (item == list->head)
        list->head = next;

    if (item == list->tail)
        list->tail = prev;

    if (item->data)
        free(item->data);
    free(item);

    list->n--;
}

static struct FigListItem *
create_new_item(void *data)
{
    struct FigListItem *item;

    item = malloc(sizeof *item);
    if (!item)
        fig_panic("failed allocating memeory");

    item->prev = NULL;
    item->next = NULL;
    item->data = data;

    return item;
}

static struct FigListItem *
item_at(FigList *list, int index)
{
    assert(list);
    assert(index >= 0);
    assert(index < list->n);

    int i = 0;
    struct FigListItem *item = list->head;


    while (item) {
        if (i == index)
            return item;

        item = item->next;
        i++;
    }

    return NULL;
}

void
fig_list_clear(FigList *list)
{
    struct FigListItem *item, *temp;

    item = list->head;
    while (item) {
        temp = item->next;
	    free(item);
	    item = temp;
    }
}
