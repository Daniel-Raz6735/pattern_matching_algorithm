/*Daniel Raz*/

#include "pattern_matching.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//This function will print all the matches start position, end position,and last state.
void print_match_list(slist_t *l)
{
    if (l == NULL || slist_head(l) == NULL)
        return;

    for (slist_node_t *next = slist_head(l); next != NULL; next = slist_next(next))
    {
        pm_match_t *match = slist_data(next);
        if (match == NULL)
            return;
        printf("Pattern: %s, start at: %d, ends at: %d, last state = %d\n", match->pattern, match->start_pos, match->end_pos, match->fstate->id);
    }
}

//This function appends to the match list the string found in next state.
void findMatch(pm_state_t *next, slist_t *matches, int i)
{
    slist_node_t *temp = slist_head(next->output);

    for (int j = 0; j < next->output->size; j++)
    {
        pm_match_t *match = (pm_match_t *)malloc(sizeof(pm_match_t));
        if (match == NULL)
            return;

        match->pattern = (char *)slist_data(temp);
        match->end_pos = i;
        match->start_pos = i - (strlen(match->pattern) - 1);
        match->fstate = next;
        int x = slist_append(matches, match);
        if (x == -1)
            return;
        temp = slist_next(temp);
    }
}

/* Initializes the fsm parameters (the fsm itself sould be allocated).  Returns 0 on success, -1 on failure. 
*  this function should init zero state
*/
int pm_init(pm_t *fsm)
{
    if (fsm == NULL)
        return -1;

    fsm->zerostate = (pm_state_t *)malloc(sizeof(pm_state_t));
    if (fsm->zerostate == NULL)
        return -1;

    fsm->zerostate->depth = 0;
    fsm->zerostate->id = 0;
    fsm->zerostate->fail = NULL;
    fsm->zerostate->output = NULL;
    fsm->zerostate->_transitions = (slist_t *)malloc(sizeof(slist_t));
    if (fsm->zerostate->_transitions == NULL)
    {
        free(fsm->zerostate); //if not work delete.
        return -1;
    }

    slist_init(fsm->zerostate->_transitions);
    fsm->newstate = 1;

    return 0;
}

/* Adds a new string to the fsm, given that the string is of length n. 
   Returns 0 on success, -1 on failure.  insert pattern*/
int pm_addstring(pm_t *fsm, unsigned char *str, size_t n)
{
    if (fsm == NULL || n != strlen(str) || str == NULL || n < 1)
        return -1;

    pm_state_t *temp = fsm->zerostate;

    for (int i = 0; i < n; i++)
    {
        pm_state_t *next = pm_goto_get(temp, str[i]);
        if (next == NULL) //set the new edge in FSM.
        {
            next = (pm_state_t *)malloc(sizeof(pm_state_t));
            if (next == NULL)
                return -1;

            next->depth = temp->depth + 1;
            next->fail = NULL;
            next->output = (slist_t *)malloc(sizeof(slist_t));
            if (next->output == NULL)
            {
                free(next); 
                return -1;
            }
            slist_init(next->output);

            next->id = fsm->newstate;
            fsm->newstate++;
            next->_transitions = (slist_t *)malloc(sizeof(slist_t));
            if (next->_transitions == NULL)
            {
                slist_destroy(next->output,SLIST_FREE_DATA);
                free(next);
                return -1;
            }
            slist_init(next->_transitions);

            int x = pm_goto_set(temp, str[i], next);
            if (x == -1)
                return -1;
        }
        temp = next;
    }

    int y = slist_append(temp->output, str);
    if (y == -1)
        return -1;

    return 0;
}

/* Finalizes construction by setting up the failrue transitions, as
   well as the goto transitions of the zerostate. 
   Returns 0 on success, -1 on failure. */
int pm_makeFSM(pm_t *fsm)
{
    if (fsm == NULL || fsm->zerostate == NULL)
        return -1;

    slist_t *que = (slist_t *)malloc(sizeof(slist_t)); //make a queue for all states at the FSM.
    if (que == NULL)
        return -1;
    slist_init(que);

    pm_state_t *temp = fsm->zerostate;
    int i = slist_append(que, temp);
    if (i == -1)
    {
        free(que); 
        return -1;
    }

    while (slist_head(que) != NULL)
    {
        temp = slist_pop_first(que);
        if (temp == NULL)
            return -1;
        slist_node_t *next = slist_head(temp->_transitions);
        while (next != NULL) //add all the edges of the state to the queue
        {
            pm_labeled_edge_t *edge = slist_data(next);
            pm_state_t *state = pm_goto_get(temp->fail, edge->label);
            if (edge->state->depth == 1)
                edge->state->fail = fsm->zerostate;

            if (state == NULL)
            {
                pm_state_t *failure = temp->fail;
                while (failure != NULL) //serch for the failure of the state till the zerostate.
                {
                    state = pm_goto_get(failure->fail, edge->label);
                    if (state == NULL)
                    {
                        edge->state->fail = fsm->zerostate;
                    }
                    else
                    {
                        edge->state->fail = state;
                        break;
                    }
                    failure = failure->fail;
                }
            }
            else
            {
                edge->state->fail = state;
            }

            //if the failure of the state send to state with output add the output to the state.
            if (edge->state->fail->output != NULL)
            {
                int x = slist_append_list(edge->state->output, edge->state->fail->output);
                if (x == -1)
                    return -1;
            }

            int i = slist_append(que, edge->state);
            if (i == -1)
                return -1;

            printf("Setting f(%d) = %d\n", edge->state->id, edge->state->fail->id);

            next = slist_next(next);
        }
    }
    free(que);
    return 0;
}

/* Set a transition arrow from this from_state, via a symbol, to a
   to_state. will be used in the pm_addstring and pm_makeFSM functions.
   Returns 0 on success, -1 on failure.*/
int pm_goto_set(pm_state_t *from_state, unsigned char symbol, pm_state_t *to_state)
{
    if (from_state == NULL || to_state == NULL || symbol == '\0') 
        return -1;

    pm_labeled_edge_t *temp = (pm_labeled_edge_t *)malloc(sizeof(pm_labeled_edge_t));
    if (temp == NULL)
        return -1;

    temp->label = symbol;
    temp->state = to_state;

    if (from_state->_transitions == NULL)
    {
        from_state->_transitions = (slist_t *)malloc(sizeof(slist_t));
        if (from_state->_transitions == NULL)
        {
            free(temp);
            return -1;
        }
        slist_init(from_state->_transitions);
    }
    int x = slist_append(from_state->_transitions, temp);
    if (x == -1)
        return -1;

    printf("Allocating state %d\n%d ->%c->%d\n", to_state->id, from_state->id, symbol, to_state->id);

    return 0;
}

/* Returns the transition state.  If no such state exists, returns NULL. 
   will be used in pm_addstring, pm_makeFSM, pm_fsm_search, pm_destroy functions. */
pm_state_t *pm_goto_get(pm_state_t *state, unsigned char symbol)
{
    if (state == NULL || state->_transitions == NULL)
        return NULL;

    slist_node_t *temp = slist_head(state->_transitions);

    while (temp != NULL)
    {
        pm_labeled_edge_t *edge = slist_data(temp);
        if (edge->label == symbol)
            return edge->state;
        temp = slist_next(temp);
    }

    return NULL;
}

/* Search for matches in a string of size n in the FSM. 
   if there are no matches return empty list */
slist_t *pm_fsm_search(pm_state_t *state, unsigned char *str, size_t n)
{

    slist_t *matches = (slist_t *)malloc(sizeof(slist_t));
    if (matches == NULL)
        return NULL;
    slist_init(matches);

    if (state == NULL || str == NULL || n != strlen(str) || n < 1)
        return NULL;

    pm_state_t *temp = state, *next = state;

    for (int i = 0; i < n; i++)
    {
        //if the state is zerostate and char checked has no edge then continue to next char.
        if (temp->depth == 0 && pm_goto_get(temp, str[i]) == NULL)
            continue;

        next = pm_goto_get(temp, str[i]); //try get forward in the FSM with the string str.
        if (next == NULL)
        {
            while (temp->fail != NULL) //try to get forward from the failure of the state.
            {
                next = pm_goto_get(temp->fail, str[i]);
                if (next == NULL)
                    temp = temp->fail;
                else
                {
                    if (next->output != NULL && slist_head(next->output) != NULL)
                    {
                        findMatch(next, matches, i);
                    }
                    temp = next;
                    break;
                }
            }
        }
        else
        {
            if (next->output != NULL && slist_head(next->output) != NULL)
            {
                findMatch(next, matches, i);
            }
            temp = next;
        }
    }
    print_match_list(matches);
    return matches;
}

/* Destroys the fsm, deallocating memory. */
void pm_destroy(pm_t *fsm)
{
    slist_t *que = (slist_t *)malloc(sizeof(slist_t)); //make a queue for all the states.
    if (que == NULL)
        return;
    slist_init(que);

    pm_state_t *temp = fsm->zerostate;
    int i = slist_append(que, temp);
    if (i == -1)
    {
        free(que); 
        return;
    }

    while (slist_head(que) != NULL)
    {
        temp = slist_pop_first(que);
        slist_node_t *next = slist_head(temp->_transitions);
        while (next != NULL)
        {
            pm_labeled_edge_t *edge = slist_data(next);
            int i = slist_append(que, edge->state);
            if (i == -1)
                return;
            next = slist_next(next);
        }
        if (temp->output != NULL)
        {
            slist_destroy(temp->output, SLIST_LEAVE_DATA);
            free(temp->output);
        }
        if (temp->_transitions != NULL)
        {
            slist_destroy(temp->_transitions, SLIST_FREE_DATA);
            free(temp->_transitions);
        }
        free(temp);
    }
    free(que);
}
