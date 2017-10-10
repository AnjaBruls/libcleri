/*
 * prio.h - cleri prio element. (this element create a cleri rule object
 *          holding this prio element)
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 08-03-2016
 *  - refactoring, 17-06-2017
 */
#ifndef CLERI_PRIO_H_
#define CLERI_PRIO_H_

#include <stddef.h>
#include <inttypes.h>
#include <cleri/cleri.h>
#include <cleri/olist.h>
#include <cleri/rule.h>

/* typedefs */
typedef struct cleri_s cleri_t;
typedef struct cleri_prio_s cleri_prio_t;

/* public functions */
#ifdef __cplusplus
extern "C" {
#endif

cleri_t * cleri_prio(uint32_t gid, size_t len, ...);

#ifdef __cplusplus
}
#endif

/* structs */
struct cleri_prio_s
{
    cleri_olist_t * olist;
};

#endif /* CLERI_PRIO_H_ */