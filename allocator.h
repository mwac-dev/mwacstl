#ifndef MWAC_ALLOCATOR_H
#define MWAC_ALLOCATOR_H

#include <stddef.h>

// === Allocator Interface ===
typedef struct {
    void* ctx;
    void* (*alloc)(void* ctx, size_t size);
    void* (*realloc)(void* ctx, void* ptr, size_t old_size, size_t new_size);
    void* (*calloc)(void* ctx, size_t count, size_t size);
    void (*free)(void* ctx, void* ptr);
} mwac_allocator;

// === Arena ===
typedef struct {
    unsigned char* base;
    size_t size;
    size_t used;
} mwac_arena;

void mwac_arena_init(mwac_arena* a, void* buffer, size_t size);
void mwac_arena_reset(mwac_arena* a);
size_t mwac_arena_remaining(mwac_arena* a);

// Create allocator interface from arena
mwac_allocator mwac_arena_allocator(mwac_arena* a);

// === Heap (default) ===
// note that these default functions exist to satisfy the default allocator interface for my containers
// https://github.com/mwac-dev/containergen
// for manual heap allocation prefer using malloc/free directly to avoid unnecessary abstraction

mwac_allocator* mwac_heap_allocator_default(void);

// === Helper macros ===
#define mwac_alloc(a, size) ((a)->alloc((a)->ctx, (size)))
#define mwac_realloc(a, ptr, old_size, new_size) ((a)->realloc((a)->ctx, (ptr), (old_size), (new_size)))
#define mwac_calloc(a, count, size) ((a)->calloc((a)->ctx, (count), (size)))
#define mwac_free(a, ptr) ((a)->free((a)->ctx, (ptr)))

#endif
