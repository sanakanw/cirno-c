#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct entry_s entry_t;

struct entry_s {
  void *value;
  entry_t *next;
  hash_t key;
};

struct map_s {
  entry_t *entries[MAX_ENTRIES];
};

map_t *make_map()
{
  map_t *map = malloc(sizeof(map_t));
  memset(map, 0, sizeof(map_t));
  
  return map;
}

entry_t *new_entry(hash_t key, void *value)
{
  entry_t *entry = malloc(sizeof(entry_t));
  entry->key = key;
  entry->value = value;
  entry->next = NULL;
  
  return entry;
}

void map_flush(map_t *map)
{
  for (int i = 0; i < MAX_ENTRIES; i++) {
    entry_t *entry = map->entries[i];
    
    if (entry) {
      while (entry) {
        entry_t *next = entry->next;
        free(entry->value); // dodgy lol
        free(entry);
        entry = next;
      }
      
      map->entries[i] = NULL;
    }
  }
}

int map_put(map_t *map, hash_t key, void *value)
{
  int id = key % MAX_ENTRIES;
  
  entry_t *entry = map->entries[id];
  
  if (entry) {
    while (entry->next) {
      if (entry->key == key)
        return 0;
      entry = entry->next;
    }
    
    entry->next = new_entry(key, value);
  } else {
    map->entries[id] = new_entry(key, value);
  }
  
  return 1;
}

void *map_get(map_t *map, hash_t key)
{
  int id = key % MAX_ENTRIES;
  
  entry_t *entry = map->entries[id];
  
  if (entry) {
    while (entry) {
      if (entry->key == key)
        return entry->value;
      entry = entry->next;
    }
  }
  
  return NULL;
}
