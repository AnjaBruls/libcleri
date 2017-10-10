/*
 * optional.c - cleri optional element.
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 08-03-2016
 *
 */
#include <cleri/optional.h>
#include <cleri/expecting.h>
#include <stdlib.h>

static void OPTIONAL_free(cleri_t * cl_object);

static cleri_node_t * OPTIONAL_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule);

/*
 * Returns NULL and in case an error has occurred.
 */
cleri_t * cleri_optional(uint32_t gid, cleri_t * cl_obj)
{
    if (cl_obj == NULL)
    {
        return NULL;
    }

    cleri_t * cl_object = cleri_new(
            gid,
            CLERI_TP_OPTIONAL,
            &OPTIONAL_free,
            &OPTIONAL_parse);

    if (cl_object == NULL)
    {
        return NULL;
    }

    cl_object->via.optional =
            (cleri_optional_t *) malloc(sizeof(cleri_optional_t));

    if (cl_object->via.optional == NULL)
    {
        free(cl_object);
        return NULL;
    }

    cl_object->via.optional->cl_obj = cl_obj;

    cleri_incref(cl_obj);

    return cl_object;
}

/*
 * Destroy optional object.
 */
static void OPTIONAL_free(cleri_t * cl_object)
{
    cleri_free(cl_object->via.optional->cl_obj);
    free(cl_object->via.optional);
}

/*
 * Returns a node or NULL. In case of an error pr->is_valid is set to -1.
 */
static cleri_node_t * OPTIONAL_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_t * cl_obj,
        cleri_rule_store_t * rule)
{
    cleri_node_t * node;
    cleri_node_t * rnode;

    if ((node = cleri__node_new(cl_obj, parent->str + parent->len, 0)) == NULL)
    {
        pr->is_valid = -1;
        return NULL;
    }
    rnode = cleri__parse_walk(
            pr,
            node,
            cl_obj->via.optional->cl_obj,
            rule,
            CLERI__EXP_MODE_OPTIONAL);
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

    cleri__node_free(node);
    return CLERI_EMPTY_NODE;
}
