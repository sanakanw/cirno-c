#ifndef MAP_H
#define MAP_H

#include "hash.h"

#define MAX_ENTRIES 1021

typedef struct map_s map_t;

map_t *make_map();

void map_flush(map_t *map);
void *map_get(map_t *map, hash_t key);
int map_put(map_t *map, hash_t key, void *value);

#endif
