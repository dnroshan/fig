/*
 * File: list.h
 * Desc: Interface for a general purpose list implimentation.
 */

#ifndef _FIG_LIST_H_
#define _FIG_LIST_H_

struct FigListItem
{
    void *data;

    struct FigListItem *prev;
    struct FigListItem *next;
};

typedef struct
{
    size_t n;

    struct FigListItem *head;
    struct FigListItem *tail;
} FigList;

#define fig_list_length(list) (list->n)
#define fig_list_tail(list) \
    (list->tail->data)
#define fig_list_for_each(list) \
    for (struct FigListItem *item = list->head; \
         item; item = item->next)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

FigList *fig_list_new     ();
void     fig_list_destroy (FigList *list);
void     fig_list_append  (FigList *list, void *data);
void     fig_list_insert  (FigList *list, int index, void *data);
void    *fig_list_at      (FigList *list, int index);   
void     fig_list_remove  (FigList *list, int index);
void     fig_list_clear   (FigList *list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _FIG_LIST_H_ */
