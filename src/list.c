/*
 * list.c - cleri list element.
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 08-03-2016
 *
 */
#include <cleri/list.h>
#include <stdlib.h>

static void LIST_free(cleri_t * cl_object);
static cleri_node_t * LIST_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule);

/*
 * Returns NULL in case an error has occurred.
 *
 * cl_obj       :   object to repeat
 * delimiter    :   object (Usually a Token) as delimiter
 * min          :   should be equal to or higher then 0.
 * max          :   should be equal to or higher then 0 but when 0 it
 *                  means unlimited.
 * opt_closing  :   when set to true (1) the list can be closed with a
 *                  delimiter. when false (0) this is not allowed.
 */
cleri_t * cleri_list(
        uint32_t gid,
        cleri_t * cl_obj,
        cleri_t * delimiter,
        size_t min,
        size_t max,
        int opt_closing)
{
    if (cl_obj == NULL || delimiter == NULL)
    {
        return NULL;
    }

    cleri_t * cl_object = cleri_new(
            gid,
            CLERI_TP_LIST,
            &LIST_free,
            &LIST_parse);

    if (cl_object == NULL)
    {
        return NULL;
    }

    cl_object->via.list =
            (cleri_list_t *) malloc(sizeof(cleri_list_t));

    if (cl_object->via.list == NULL)
    {
        free(cl_object);
        return NULL;
    }

    cl_object->via.list->cl_obj = cl_obj;
    cl_object->via.list->delimiter = delimiter;
    cl_object->via.list->min = min;
    cl_object->via.list->max = max;
    cl_object->via.list->opt_closing = opt_closing;

    cleri_incref(cl_obj);
    cleri_incref(delimiter);

    return cl_object;
}

/*
 * Destroy list object.
 */
static void LIST_free(cleri_t * cl_object)
{
    cleri_free(cl_object->via.list->cl_obj);
    cleri_free(cl_object->via.list->delimiter);
    free(cl_object->via.list);
}

/*
 * Returns a node or NULL. In case of an error pr->is_valid is set to -1.
 */
static cleri_node_t *  LIST_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule)
{
    cleri_children_t * tmp;
    cleri_node_t * node;
    cleri_node_t * rnode;
    size_t i = 0;
    size_t j = 0;

    if ((node = cleri__node_new(cl_obj, parent->str + parent->len, 0)) == NULL)
    {
        pr->is_valid = -1;
        return NULL;
    }

    while (1)
    {
        rnode = cleri__parse_walk(
                pr,
                node,
                cl_obj->via.list->cl_obj,
                rule,
                i < cl_obj->via.list->min); // 1 = REQUIRED
        if (rnode == NULL)
        {
            break;
        }
        i++;
        rnode = cleri__parse_walk(
                pr,
                node,
                cl_obj->via.list->delimiter,
                rule,
                i < cl_obj->via.list->min); // 1 = REQUIRED
        if (rnode == NULL)
        {
            break;
        }
        j++;
    }
    if (    i < cl_obj->via.list->min ||
            (cl_obj->via.list->max && i > cl_obj->via.list->max) ||
            ((cl_obj->via.list->opt_closing == 0) && i && i == j))
    {
        cleri__node_free(node);
        return NULL;
    }

    tmp = (cleri_children_t *) cleri_vec_push(
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
