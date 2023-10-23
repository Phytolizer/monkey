#include "monkey/environment.h"

#include "monkey/macros.h"
#include "monkey/object.h"

#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>

struct Environment {
	GHashTable* store;
};

MONKEY_FILE_LOCAL void tblDestroyObject(void* obj) {
	DestroyObject(obj);
}

Environment* CreateEnvironment(void) {
	Environment* env = malloc(sizeof(Environment));
	env->store = g_hash_table_new_full(g_str_hash, g_str_equal, free, tblDestroyObject);
	return env;
}

void DestroyEnvironment(Environment* env) {
	g_hash_table_destroy(env->store);
	free(env);
}

Object* GetEnvironment(Environment* env, const char* name) {
	return g_hash_table_lookup(env->store, name);
}

bool PutEnvironment(Environment* env, char* name, Object* val) {
	return g_hash_table_insert(env->store, name, val);
}
