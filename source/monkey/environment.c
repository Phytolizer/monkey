#include "monkey/environment.h"

#include "monkey/macros.h"
#include "monkey/object.h"

#include <glib.h>
#include <stdbool.h>
#include <stdlib.h>

struct Environment {
	Environment* outer;
	GHashTable* store;
};

MONKEY_FILE_LOCAL void tblDestroyObject(void* obj) {
	DestroyObject(obj);
}

Environment* CreateEnvironment(Environment* outer) {
	Environment* env = malloc(sizeof(Environment));
	env->outer = outer;
	env->store = g_hash_table_new_full(g_str_hash, g_str_equal, free, tblDestroyObject);
	return env;
}

void DestroyEnvironment(Environment* env) {
	g_hash_table_destroy(env->store);
	free(env);
}

MONKEY_FILE_LOCAL void tblCopyKv(gpointer key, gpointer value, gpointer userData) {
	GHashTable* dest = userData;
	g_hash_table_insert(dest, key, value);
}

Environment* CopyEnvironment(Environment* env) {
	Environment* result = CreateEnvironment(env->outer);
	g_hash_table_foreach(env->store, &tblCopyKv, result->store);
	return result;
}

Object* GetEnvironment(Environment* env, const char* name) {
	Object* result = g_hash_table_lookup(env->store, name);
	if (result != NULL) {
		return result;
	}
	if (env->outer != NULL) {
		return GetEnvironment(env->outer, name);
	}
	return NULL;
}

bool PutEnvironment(Environment* env, char* name, Object* val) {
	return g_hash_table_insert(env->store, name, val);
}
