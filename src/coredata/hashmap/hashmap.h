#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define HASHMAP_KEY_LENGTH 64

struct HashmapNode {
    char key[HASHMAP_KEY_LENGTH];
    void *value;
};

struct Hashmap {
    struct HashmapNode *data;
    uint32_t capacity;
    uint32_t size;
};

struct Hashmap *hashmap_create(uint32_t capacity);
void hashmap_free(struct Hashmap *map);
bool hashmap_set(struct Hashmap *map, char *key, void *value, uint32_t value_size);
void *hashmap_get(struct Hashmap *map, char *key);
bool hashmap_remove(struct Hashmap *map, char *key);

#endif // _HASHMAP_H
