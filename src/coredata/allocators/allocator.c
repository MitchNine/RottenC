#include "allocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LinearAllocator* linear_allocator_new()
{
    LinearAllocator* allocator = malloc(sizeof(LinearAllocator));
    memset(allocator, 0, sizeof(LinearAllocator));
    return allocator;
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
        memset(allocator->_begin, 0, size);
        allocator->_end = allocator->_begin;
        allocator->_memcap = allocator->_end + size;
        linear_allocator_print(allocator, "REALLOC", "NEW\n");
        return;
    }
    size_t old_alloc = linear_allocator_size(allocator);
    linear_allocator_print(allocator, "REALLOC", "OLD (Changing to %ld)\n", size);
    void* tmp = malloc(size);
    memset(tmp, 0, size);
    memmove(tmp, allocator->_begin, old_alloc);
    free(allocator->_begin);
    allocator->_begin = tmp;
    allocator->_end = allocator->_begin + old_alloc;
    allocator->_memcap = allocator->_begin + size;
    linear_allocator_print(allocator, "REALLOC", "NEW\n");
}
LinearSlice linear_allocator_malloc(LinearAllocator* allocator, size_t size)
{
    if (allocator == NULL) { return (LinearSlice){NULL, 0, 0}; }
    if (allocator->_begin == NULL) {
        linear_allocator_realloc(allocator, 512);
    }
    uint32_t tries = 0;
    while (allocator->_memcap - allocator->_end <= size) {
        size_t new_size = linear_allocator_cap(allocator) * 2;
        linear_allocator_realloc(allocator, new_size);
        if (tries++ > 50) return (LinearSlice){NULL, 0, 0};
    }

    LinearSlice slice = { allocator, linear_allocator_size(allocator), size };
    
    allocator->_end += size;
    linear_allocator_print(allocator, "MALLOC", "+\e[34m%ld\e[0m bytes\n", size);
    return slice;
}
void linear_allocator_clear(LinearAllocator* allocator)
{
    if (allocator == NULL) { return; }
    if (allocator->_begin == NULL) { return; }
    allocator->_end = allocator->_begin;
    printf("[\e[35mALLOC\e[0m] Clear\n");
}
void* linear_allocator_at_slice(LinearSlice slice)
{
    if (slice.allocator == NULL) { return NULL; }
    return slice.allocator->_begin + slice.position;
}
