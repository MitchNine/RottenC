// A Linear Allocator written in C
// Copyright © 2025 Mitchell Jenkins
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef ALLOCATOR_H_RKHJO9RQ
#define ALLOCATOR_H_RKHJO9RQ

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

typedef struct LinearAllocator {
    void* _begin;
    void* _end;
    void* _memcap;
} LinearAllocator;

typedef struct LinearSlice {
    LinearAllocator* allocator;
    size_t position;
    size_t size;
} LinearSlice;

LinearAllocator* linear_allocator_new();
void linear_allocator_free(LinearAllocator* allocator);
void linear_allocator_realloc(LinearAllocator* allocator, size_t size);
LinearSlice linear_allocator_malloc(LinearAllocator* allocator, size_t size);
void linear_allocator_clear(LinearAllocator* allocator);

inline static size_t linear_allocator_size(LinearAllocator* allocator) {
    return allocator->_end - allocator->_begin;
}
inline static size_t linear_allocator_cap(LinearAllocator* allocator) {
    return allocator->_memcap - allocator->_begin;
}

#ifdef DEBUG
inline static void linear_allocator_print(LinearAllocator* alloc, char* type, char* fmt, ...) {
    printf("[\e[35m%s\e[0m][%ld/%ld] ", type,
            linear_allocator_size(alloc),
            linear_allocator_cap(alloc));
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
#else
inline static void linear_allocator_print(LinearAllocator*, char*, char*, ...) { }
#endif

void* linear_allocator_at_slice(LinearSlice slice);

#endif /* end of include guard: ALLOCATOR_H_RKHJO9RQ */
