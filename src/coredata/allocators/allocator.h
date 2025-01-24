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

#include <stdint.h>
#include "stddef.h"

typedef struct LinearAllocator {
    void* _begin;
    void* _end;
    void* _memcap;
} LinearAllocator;

LinearAllocator* linear_allocator_new();
void linear_allocator_free(LinearAllocator* allocator);
void linear_allocator_realloc(LinearAllocator* allocator, size_t size);
void* linear_allocator_malloc(LinearAllocator* allocator, size_t size);
void linear_allocator_clear(LinearAllocator* allocator);

size_t linear_allocator_size(LinearAllocator* allocator);
size_t linear_allocator_cap(LinearAllocator* allocator);

#endif /* end of include guard: ALLOCATOR_H_RKHJO9RQ */
