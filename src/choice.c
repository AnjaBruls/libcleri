/*
 * choice.c - this cleri element can hold other elements and the grammar
 *            has to choose one of them.
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 08-03-2016
 *  - fixed issue #53, 23-02-2017
 */
#include <cleri/choice.h>
#include <cleri/node.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <cleri/vec.h>

static void CHOICE_free(cleri_t * cl_object);

static cleri_node_t * CHOICE_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule);

static cleri_node_t * CHOICE_parse_most_greedy(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule);

static cleri_node_t * CHOICE_parse_first_match(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule);

/*
 * Returns NULL in case an error has occurred.
 */
cleri_t * cleri_choice(uint32_t gid, int most_greedy, size_t len, ...)
{
    va_list ap;

    cleri_t * cl_object = cleri_new(
            gid,
            CLERI_TP_CHOICE,
            &CHOICE_free,
            &CHOICE_parse);

    if (cl_object == NULL)
    {
        return NULL;
    }

    cl_object->via.choice =
            (cleri_choice_t *) malloc(sizeof(cleri_choice_t));

    if (cl_object->via.choice == NULL)
    {
        free(cl_object);
        return NULL;
    }

    cl_object->via.choice->most_greedy = most_greedy;
    cl_object->via.choice->olist = (cleri_olist_t *) cleri_vec_new(len);

    if (cl_object->via.choice->olist == NULL)
    {
        cleri_free(cl_object);
        return NULL;
    }

    va_start(ap, len);
    while(len--)
    {
        cleri_t * o = va_arg(ap, cleri_t *);
        CLERI_VEC_push((cleri_vec_t *) cl_object->via.choice->olist, o);
        o->ref++;
    }
    va_end(ap);

    return cl_object;
}

/*
 * Destroy choice object.
 */
static void CHOICE_free(cleri_t * cl_object)
{
    cleri__olist_free(cl_object->via.choice->olist);
    free(cl_object->via.choice);
}

/*
 * Returns a node or NULL.
 */
static cleri_node_t * CHOICE_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule)
{
    return (cl_obj->via.choice->most_greedy) ?
            CHOICE_parse_most_greedy(pr, parent, cl_obj, rule) :
            CHOICE_parse_first_match(pr, parent, cl_obj, rule);
}

/*
 * Returns a node or NULL. In case of an error pr->is_valid is set to -1.
 */
static cleri_node_t * CHOICE_parse_most_greedy(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule)
{
    cleri_olist_t * olist;
    cleri_node_t * node;
    cleri_node_t * rnode;
    cleri_node_t * mg_node = NULL;
    const char * str = parent->str + parent->len;

    olist = cl_obj->via.choice->olist;
    for (uint32_t i = 0; i < olist->n; i++)
    {
        if ((node = cleri__node_new(cl_obj, str, 0)) == NULL)
        {
            pr->is_valid = -1;
            return NULL;
        }
        rnode = cleri__parse_walk(
                pr,
                node,
                olist->cl_obj[i],
                rule,
                CLERI__EXP_MODE_REQUIRED);
        if (rnode != NULL && (mg_node == NULL || node->len > mg_node->len))
        {
            cleri__node_free(mg_node);
            mg_node = node;
        }
        else
        {
            cleri__node_free(node);
        }
    }
    if (mg_node != NULL)
    {
        cleri_children_t * tmp = (cleri_children_t *) cleri_vec_push(
            (cleri_vec_t *) parent->children, mg_node);
        if (!tmp)
        {
            /* error occurred, reverse changes set mg_node to NULL */
           pr->is_valid = -1;
           cleri__node_free(mg_node);
           return NULL;
        }
        parent->children = tmp;
        parent->len += mg_node->len;
    }
    return mg_node;
}

/*
 * Returns a node or NULL. In case of an error pr->is_valid is set to -1.
 */
static cleri_node_t * CHOICE_parse_first_match(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule)
{
    cleri_olist_t * olist;
    cleri_node_t * node;
    cleri_node_t * rnode;

    olist = cl_obj->via.choice->olist;
    node = cleri__node_new(cl_obj, parent->str + parent->len, 0);
    if (node == NULL)
    {
        pr->is_valid = -1;
        return NULL;
    }
    for (uint32_t i = 0; i < olist->n; i++)
    {
        rnode = cleri__parse_walk(
                pr,
                node,
                olist->cl_obj[i],
                rule,
                CLERI__EXP_MODE_REQUIRED);
        if (rnode != NULL)
        {
            cleri_children_t * tmp = (cleri_children_t *) cleri_vec_push(
                (cleri_vec_t *) parent->children, node);
            if (!tmp)
            {
                /* error occurred, reverse changes set mg_node to NULL */
               pr->is_valid = -1;
               cleri__node_free(node);
               return NULL;
            }
            parent->children = tmp;
            parent->len += node->len;
            return node;
        }
    }
    cleri__node_free(node);
    return NULL;
}
