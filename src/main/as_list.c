#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <cf_alloc.h>

#include "as_list.h"
#include "as_util.h"

#include "internal.h"

/******************************************************************************
 * INLINE FUNCTIONS
 ******************************************************************************/

extern inline uint32_t as_list_size(as_list * l) ;
extern inline int as_list_append(as_list * l, as_val * v) ;
extern inline int as_list_prepend(as_list * l, as_val * v) ;
extern inline as_val * as_list_get(const as_list * l, const uint32_t i) ;
extern inline int as_list_set(as_list * l, const uint32_t i, as_val * v) ;
extern inline as_val * as_list_head(const as_list * l) ;
extern inline as_list * as_list_tail(const as_list * l) ;
extern inline as_list * as_list_drop(const as_list * l, uint32_t n) ;
extern inline as_list * as_list_take(const as_list * l, uint32_t n) ;
extern inline void as_list_foreach(const as_list * l, void * context, bool (* foreach)(as_val *, void *)) ;
extern inline as_iterator * as_list_iterator_init(as_iterator *i, const as_list * l);
extern inline as_iterator * as_list_iterator_new(const as_list * l) ;
extern inline uint32_t as_list_hash(const as_list * l) ;
extern inline as_val * as_list_toval(as_list * l) ;
extern inline as_list * as_list_fromval(as_val * v) ;

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

as_list * as_list_init(as_list *l, void *source, const as_list_hooks *h) {
    as_val_init(&l->_, AS_LIST, false /*is_malloc*/);
    l->hooks = h;
    l->u.generic = source;
    return l;
}

as_list * as_list_new(void *source, const as_list_hooks *h) {
    as_list *l = (as_list *) malloc(sizeof(as_list));
    as_val_init(&l->_, AS_LIST, true /*is_malloc*/);
    l->hooks = h;
    l->u.generic = source;
    return l;
}

//
// helper function, call this
void as_list_destroy(as_list *l) {
	as_val_val_destroy( (as_val *) l );
}

void *as_list_source(const as_list *l) {
    return( l->u.generic );
}

/******************************************************************************
 * STATIC FUNCTIONS
 ******************************************************************************/

//
// calls the implementations destroy
// not for external use
 
void as_list_val_destroy(as_val *v) {
    as_list *l = (as_list *) v;
    as_util_hook(destroy, 1, l);
}
 
uint32_t as_list_val_hash(const as_val * v) {
    return as_list_hash((const as_list *) v);
}

char * as_list_val_tostring(const as_val * v) {

    as_list * l = (as_list *) v;

    char *      buf = NULL;
    uint32_t    blk = 256;
    uint32_t    cap = blk;
    uint32_t    pos = 0;
    bool        sep = false;

    buf = (char *) malloc(sizeof(char) * cap);

    strcpy(buf, "List(");
    pos += 5;
    as_iterator *i = as_list_iterator_new(l);
    while ( as_iterator_has_next(i) ) {
        as_val * val = (as_val *) as_iterator_next(i);
        if ( val ) {
            char * valstr = as_val_tostring(val);
            size_t vallen = strlen(valstr);
            
            if ( pos + vallen + 2 >= cap ) {
                uint32_t adj = ((vallen+2) > blk) ? vallen+2 : blk;
                buf = realloc(buf, sizeof(char) * (cap + adj));
                cap += adj;
            }

            if ( sep ) {
            	buf[pos] = ',';
            	buf[pos+1] = ' ';
                pos += 2;
            }

            memcpy(buf + pos, valstr, vallen);
            pos += vallen;
            sep = true;

            free(valstr);
        }
    }
    as_iterator_destroy(i);

    if ( pos + 2 >= cap ) {
		buf = realloc(buf, sizeof(char) * (cap + 2));
		cap += 2;
	}

    buf[pos] = ')';
    buf[pos+1] = 0;
    
    return buf;
}
