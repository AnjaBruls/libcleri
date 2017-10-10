/*
 * sequence.h - cleri sequence element.
 *
 * author       : Jeroen van der Heijden
 * email        : jeroen@transceptor.technology
 * copyright    : 2016, Transceptor Technology
 *
 * changes
 *  - initial version, 08-03-2016
 *  - refactoring, 17-06-2017
 */
#ifndef CLERI_SEQUENCE_H_
#define CLERI_SEQUENCE_H_

#include <stddef.h>
#include <inttypes.h>
#include <cleri/cleri.h>
#include <cleri/olist.h>

/* typedefs */
typedef struct cleri_s cleri_t;
typedef struct cleri_sequence_s cleri_sequence_t;

/* public functions */
#ifdef __cplusplus
extern "C" {
#endif

cleri_t * cleri_sequence(uint32_t gid, size_t len, ...);

#ifdef __cplusplus
}
#endif

/* structs */
struct cleri_sequence_s
{
    cleri_olist_t * olist;
};

#endif /* CLERI_SEQUENCE_H_ */