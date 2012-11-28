#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "mod_lua.h"
#include "mod_lua_config.h"
#include "as_result.h"
#include "as_stream.h"
#include "as_types.h"
#include "as_string.h"

#define LIMIT 1

#define len(arr) sizeof( arr ) / sizeof( arr[0] )

#define LOG(m) \
    printf("%s:%d - %s\n",__FILE__,__LINE__,m);

/**
 * 
 * Custome Record Object... kinda lame.
 *
 */

static const as_val * maprecord_get(const as_rec *, const char *);
static const int maprecord_set(const as_rec *, const char *, const as_val *);
static const int maprecord_free(as_rec *);
static const as_rec_hooks maprecord_hooks;

as_rec * maprecord_create(as_map * source) {
    return as_rec_create(source, &maprecord_hooks);
}

static const as_val * maprecord_get(const as_rec * r, const char * name) {
    return (as_val *) as_string_new("shizzle");
}

static const int maprecord_set(const as_rec * r, const char * name, const as_val * value) {
    return 0;
}

static const int maprecord_remove(const as_rec * r, const char * name) {
    return 0;
}

static const int maprecord_free(as_rec * r) {
    return 0;
}

static const as_rec_hooks maprecord_hooks = {
    maprecord_get,
    maprecord_set,
    maprecord_remove,
    maprecord_free
};



as_list * arglist(int argc, char ** argv) {
    if ( argc == 0 || argv == NULL ) return NULL;
    return cons((as_val *) as_string_new(argv[0]), arglist(argc-1, argv+1));
}


static void show_result(as_result * res) {
    if ( res->is_success ) {
        as_val * v = res->value;
        as_integer * i;
        as_string * s;
        switch( as_val_type(v) ) {
            case AS_INTEGER:
                i = as_integer_fromval(v);
                printf("SUCCESS: %lu\n", as_integer_toint(i) );
                break;
            case AS_STRING:
                s = as_string_fromval(v);
                printf("SUCCESS: %s\n", as_string_tostring(s) );
                break;
            default:
                printf("SUCCESS: <fuck>\n");
        }
    }
    else {
        as_val * v = res->value;
        as_string * s = as_string_fromval(v);
        printf("FAILURE: %s\n", as_string_tostring(s));
    }
}

static void run_record(const char * filename, const char * function, as_list * args, as_result * res) {

    // map * bins = listmap_create();
    // map_set(bins, String("a"), String("x"));
    // map_set(bins, String("b"), String("y"));
    // map_set(bins, String("c"), String("z"));

    LOG("fuck yeah");

    as_aerospike * as = as_aerospike_create(NULL,NULL);
    as_map * bins = as_map_new();
    as_rec * rec = maprecord_create(bins);

    LOG("fuck yeah");
    as_module_apply_record(&mod_lua, as, filename, function, rec, args, res);

    LOG("fuck yeah");
}

as_integer * stream_value = NULL;

static const as_val * run_stream_read(const as_stream * s) {

    as_integer * i = (as_integer *) as_stream_source(s);

    as_integer_inc(i);

    int i2 = as_integer_toint(i);

    if ( i2 > 1000000 ) {
        return AS_STREAM_END;
    }

    return (as_val *) i;
}

static const int run_stream_free(as_stream * s) {
    return 0;
}

static void run_stream(const char * filename, const char * function, as_list * args, as_result * res) {
    as_integer * i = as_integer_new(0);
    as_stream_hooks hooks = {run_stream_read, run_stream_free};
    as_module_apply_stream(&mod_lua, NULL, filename, function, as_stream_create(i, &hooks), args, res);
    as_integer_free(i);
}

int main ( int argc, char ** argv ) {

    if ( argc < 4 ) {
        printf("Usage: test [record|stream] <filename> <function> [args ...]\n");
        return 1;
    }

    mod_lua_config * config = mod_lua_config_create(false, "src/lua", "src/test/lua");

    as_module_init(&mod_lua);
    as_module_configure(&mod_lua, config);

    char * ftype = argv[1];
    char * filename = argv[2];
    char * function = argv[3];

    as_list * args = argc > 4 ? arglist(argc-4, argv+4) : as_list_new();

    int i;
    for ( i = 0; i < LIMIT; i++ ) {
        as_result res = { false, NULL };
        if ( strcmp(ftype,"record") == 0 ) {
            run_record(filename, function, args, &res);
        }
        else {
            run_stream(filename, function, args, &res);
        }
        show_result(&res);
    }

    as_list_free(args);

    
    return 0;
}