#ifndef HASH_H
#define HASH_H

typedef unsigned int hash_t;

void hash_init();

hash_t hash_value(const char *value);
const char *hash_get(hash_t hash);

#endif
