/*
 * regex.c - cleri regular expression element.
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 08-03-2016
 *
 */
#include <cleri/regex.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void REGEX_free(cleri_object_t * cl_object);

static cleri_node_t *  REGEX_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_object_t * cl_obj,
        cleri_rule_store_t * rule);

/*
 * Returns a regex object or NULL in case of an error.
 *
 * Argument pattern must start with character '^'. Be sure to check the pattern
 * for the '^' character before calling this function.
 *
 * Warning: this function could write to stderr in case the pattern could not
 * be compiled.
 */
cleri_object_t * cleri_regex(uint32_t gid, const char * pattern)
{
    cleri_object_t * cl_object;
    const char * pcre_error_str;
    int pcre_error_offset;

    assert (pattern[0] == '^');

    cl_object = cleri_object_new(
            CLERI_TP_REGEX,
            &REGEX_free,
            &REGEX_parse);

    if (cl_object == NULL)
    {
        return NULL;
    }

    cl_object->via.regex = (cleri_regex_t *) malloc(sizeof(cleri_regex_t));

    if (cl_object->via.regex == NULL)
    {
        free(cl_object);
        return NULL;
    }

    cl_object->via.regex->gid = gid;
    cl_object->via.regex->regex = pcre_compile(
            pattern,
            0,
            &pcre_error_str,
            &pcre_error_offset,
            NULL);

    if(cl_object->via.regex->regex == NULL)
    {
        fprintf(stderr,
                "error: cannot compile '%s' (%s)\n",
                pattern,
                pcre_error_str);
        free(cl_object->via.regex);
        free(cl_object);
        return NULL;
    }

    cl_object->via.regex->regex_extra =
            pcre_study(cl_object->via.regex->regex, 0, &pcre_error_str);

    /* pcre_study() returns NULL for both errors and when it can not
     * optimize the regex.  The last argument is how one checks for
     * errors (it is NULL if everything works, and points to an error
     * string otherwise. */
    if(pcre_error_str != NULL)
    {
        fprintf(stderr,
                "error: cannot compile '%s' (%s)\n",
                pattern,
                pcre_error_str);
        REGEX_free(cl_object);
        free(cl_object);
        return NULL;
    }

    return cl_object;
}

/*
 * Destroy regex object.
 */
static void REGEX_free(cleri_object_t * cl_object)
{
    free(cl_object->via.regex->regex);
    free(cl_object->via.regex->regex_extra);
    free(cl_object->via.regex);
}

/*
 * Returns a node or NULL. In case of an error, pr->is_valid is set to -1
 */
static cleri_node_t *  REGEX_parse(
        cleri_parse_t * pr,
        cleri_node_t * parent,
        cleri_object_t * cl_obj,
        cleri_rule_store_t * rule)
{
    int pcre_exec_ret;
    int sub_str_vec[2];
    const char * str = parent->str + parent->len;
    cleri_node_t * node;

    pcre_exec_ret = pcre_exec(
            cl_obj->via.regex->regex,
            cl_obj->via.regex->regex_extra,
            str,
            strlen(str),
            0,                     // start looking at this point
            0,                     // OPTIONS
            sub_str_vec,
            2);                    // length of sub_str_vec
    if (pcre_exec_ret < 0)
    {
        if (cleri__expecting_update(pr->expecting, cl_obj, str) == -1)
        {
            pr->is_valid = -1; /* error occurred */
        }
        return NULL;
    }
    /* since each regex pattern should start with ^ we now sub_str_vec[0]
     * should be 0. sub_str_vec[1] contains the end position in the sting
     */
    if ((node = cleri_node_new(cl_obj, str, (size_t) sub_str_vec[1])) != NULL)
    {
        parent->len += node->len;
        if (cleri__children_add(parent->children, node))
        {
             /* error occurred, reverse changes set node to NULL */
            pr->is_valid = -1;
            parent->len -= node->len;
            cleri_node_free(node);
            node = NULL;
        }
    }
    else
    {
        pr->is_valid = -1; /* error occurred */
    }
    return node;
}
