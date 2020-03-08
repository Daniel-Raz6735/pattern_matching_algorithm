/*Daniel Raz*/

#include "slist.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/** Initialize a single linked list
	\param list - the list to initialize */
void slist_init(slist_t *list)
{
    if (list == NULL)
        return;
    slist_head(list) = NULL;
    slist_tail(list) = NULL;
    slist_size(list) = 0;
}

/** Destroy and de-allocate the memory hold by a list
	\param list - a pointer to an existing list
	\param dealloc flag that indicates whether stored data should also be de-allocated */
void slist_destroy(slist_t *list, slist_destroy_t flag)
{
    if (list == NULL || (flag != SLIST_FREE_DATA && flag != SLIST_LEAVE_DATA))
        return;
    slist_node_t *temp = slist_head(list);
    slist_node_t *store;

    while (temp != NULL)
    {
        store = slist_next(temp);
        if (flag == SLIST_FREE_DATA)
            free(slist_data(temp));
        free(temp);
        temp = store;
    }
    list->size = 0;
}

/** Pop the first element in the list
	\param list - a pointer to a list
	\return a pointer to the data of the element, or NULL if the list is empty*/
void *slist_pop_first(slist_t *list)
{
    if (slist_head(list) != NULL && slist_data(slist_head(list)) != NULL)
    {
        void *data = slist_data(slist_head(list));
        slist_node_t *temp = slist_next(slist_head(list));
        free(slist_head(list));
        slist_head(list) = temp;
        slist_size(list)--;
        return data;
    }
    return NULL;
}

/** Append data to list (add as last node of the list)
	\param list - a pointer to a list
	\param data - the data to place in the list
	\return 0 on success, or -1 on failure */
int slist_append(slist_t *list, void *data)
{
    if (data == NULL || list == NULL)
        return -1;

    slist_node_t *temp = (slist_node_t *)malloc(sizeof(slist_node_t));
    if (temp == NULL)
        return -1;
    slist_next(temp) = NULL;

    slist_data(temp) = data;

    if (slist_head(list) == NULL)
    {
        slist_head(list) = temp;
        slist_tail(list) = temp;
        slist_size(list)++;
        return 0;
    }

    slist_next(slist_tail(list)) = temp;
    slist_tail(list) = temp;
    slist_size(list)++;

    return 0;
}

/** Prepend data to list (add as first node of the list)
	\param list - a pointer to list
	\param data - the data to place in the list
	\return 0 on success, or -1 on failure
*/
int slist_prepend(slist_t *list, void *data)
{
    if (data == NULL || list == NULL)
        return -1;

    slist_node_t *temp = (slist_node_t *)malloc(sizeof(slist_node_t));
    if (temp == NULL)
        return -1;

    slist_data(temp) = data;
    slist_next(temp) = slist_head(list);
    slist_head(list) = temp;
    slist_size(list)++;

    if (slist_next(slist_head(list)) == NULL)
        slist_tail(list) = slist_head(list);

    return 0;
}

/** \brief Append elements from the second list to the first list, use the slist_append function.
	you can assume that the data of the lists were not allocated and thus should not be deallocated in destroy 
	(the destroy for these lists will use the SLIST_LEAVE_DATA flag)
	\param to a pointer to the destination list
	\param from a pointer to the source list
	\return 0 on success, or -1 on failure
*/
int slist_append_list(slist_t *desList, slist_t *sourceList)
{
    if (desList == NULL || sourceList == NULL)
        return -1;

    if (slist_head(sourceList) == NULL)
        return 0;

    slist_node_t *temp = slist_head(sourceList);

    while (temp != NULL)
    {
        slist_append(desList, slist_data(temp));
        temp = slist_next(temp);
    }
    return 0;
}
