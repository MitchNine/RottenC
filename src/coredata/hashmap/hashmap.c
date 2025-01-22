#include "hashmap.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint32_t hashmap_hash(char *key)
{
    uint32_t hash = 0;
    while (*key) {
        hash += *key;
        hash += hash << 10;
        hash ^= hash >> 6;
        key++;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    return hash;
}

struct Hashmap *hashmap_create(uint32_t capacity)
{
    struct Hashmap *map = malloc(sizeof(struct Hashmap));
    if (map == NULL) {
        return NULL;
    }

    map->data = malloc(sizeof(struct HashmapNode) * capacity);
    if (map->data == NULL) {
        free(map);
        return NULL;
    }

    map->capacity = capacity;
    map->size = 0;
    return map;
}

void hashmap_free(struct Hashmap *map)
{
    for (uint32_t i = 0; i < map->capacity; i++) {
        if (map->data[i].key[0] != 0) {
            free(map->data[i].value);
        }
    }
    free(map->data);
    free(map);
}

bool hashmap_set(struct Hashmap *map, char *key, void *value, uint32_t value_size)
{
    if (map->size >= map->capacity) {
        return false;
    }

    uint32_t index = hashmap_hash(key) % map->capacity;

    for (uint32_t i = 0; i < map->capacity; i++) {
        if (map->data[index].key[0] == 0) {
            strncpy(map->data[index].key, key, HASHMAP_KEY_LENGTH);
            map->data[index].value = malloc(value_size);
            memcpy(map->data[index].value, value, value_size);
            map->size++;
            return true;
        }
        index = (index + 1) % map->capacity;
    }

    return false;
}

void *hashmap_get(struct Hashmap *map, char *key)
{
    uint32_t index = hashmap_hash(key) % map->capacity;

    for (uint32_t i = 0; i < map->capacity; i++) {
        if (strncmp(map->data[index].key, key, HASHMAP_KEY_LENGTH) == 0) {
            printf("found at index %d\n", i);
            return map->data[index].value;
        }
        index = (index + 1) % map->capacity;
    }

    return NULL;
}

bool hashmap_remove(struct Hashmap *map, char *key)
{
    uint32_t index = hashmap_hash(key) % map->capacity;

    for (uint32_t i = 0; i < map->capacity; i++) {
        if (strncmp(map->data[index].key, key, HASHMAP_KEY_LENGTH) == 0) {
            map->data[index].key[0] = 0;
            free(map->data[index].value);
            map->size--;
            return true;
        }
        index = (index + 1) % map->capacity;
    }

    return false;
}
