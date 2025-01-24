#include "allocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LinearAllocator* linear_allocator_new()
{
    LinearAllocator* allocator = malloc(sizeof(LinearAllocator));
    memset(allocator, 0, sizeof(LinearAllocator));
}
void linear_allocator_free(LinearAllocator* allocator)
{
    if (allocator == NULL) { return; }
    if (allocator->_begin != NULL) {
        free(allocator->_begin);
    }
    free(allocator);
}
void linear_allocator_realloc(LinearAllocator* allocator, size_t size){
    if (allocator == NULL) { return; }
    if (allocator->_begin == NULL) {
        allocator->_begin = malloc(size);
        allocator->_end = allocator->_begin;
        allocator->_memcap = allocator->_end + size;
    }
    size_t old_size = (allocator->_memcap - allocator->_begin);
    void* tmp = malloc(size);
    memcpy(tmp, allocator->_begin, old_size > size ? size : old_size);
    free(allocator->_begin);
    allocator->_begin = tmp;
    allocator->_end = allocator->_begin;
    allocator->_memcap = allocator->_end + size;
}
void* linear_allocator_malloc(LinearAllocator* allocator, size_t size)
{
    if (allocator == NULL) { return NULL; }
    if (allocator->_begin == NULL) {
        linear_allocator_realloc(allocator, 64);
    }
    uint32_t tries = 0;
    while (allocator->_memcap - allocator->_end < size) {
        tries++;
        linear_allocator_realloc(allocator, (allocator->_memcap - allocator->_begin) * 2);
        if (tries > 50) return NULL;
    }
    
    void* ptr = allocator->_end;
    allocator->_end += size;
    return ptr;
}
void linear_allocator_clear(LinearAllocator* allocator)
{
    if (allocator == NULL) { return; }
    if (allocator->_begin == NULL) { return; }
    allocator->_end = allocator->_begin;
}
size_t linear_allocator_size(LinearAllocator* allocator) {
    return allocator->_end - allocator->_begin;
}
size_t linear_allocator_cap(LinearAllocator* allocator) {
    return allocator->_memcap - allocator->_begin;
}
