#include "allocator.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// === Arena Implementation ===

void mwac_arena_init(mwac_arena* a, void* buffer, size_t size) {
    a->base = (unsigned char*)buffer;
    a->size = size;
    a->used = 0;
}

void mwac_arena_reset(mwac_arena* a) { a->used = 0; }

size_t mwac_arena_remaining(mwac_arena* a) { return a->size - a->used; }

static void* arena_alloc(void* ctx, size_t size) {
    mwac_arena* a = (mwac_arena*)ctx;

    // Align to 16 bytes (safe for SIMD)
    // Maybe I'll make alignment configurable later but for now I think 16 bytes seem like a
    // sensible default
    size_t aligned_offset = (a->used + 15) & ~(size_t)15;

    if (aligned_offset + size > a->size || aligned_offset + size < aligned_offset) {
        return NULL;
    }

    void* ptr = a->base + aligned_offset;
    a->used = aligned_offset + size;
    return ptr;
}

static void* arena_calloc(void* ctx, size_t count, size_t size) {
    if (count != 0 && size > SIZE_MAX / count) {
        return NULL;
    }
    size_t total = count * size;
    void* ptr = arena_alloc(ctx, total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

static void* heap_calloc(void* ctx, size_t count, size_t size) {
    (void)ctx;
    return calloc(count, size);
}

static void* arena_realloc(void* ctx, void* ptr, size_t old_size, size_t new_size) {
    mwac_arena* a = (mwac_arena*)ctx;

    // If it's the most recent allocation, try to extend in place
    if (ptr && (unsigned char*)ptr + old_size == a->base + a->used) {
        if (new_size <= old_size) {
            a->used -= (old_size - new_size);
            return ptr;
        }

        size_t additional = new_size - old_size;
        if (a->used + additional <= a->size) {
            a->used += additional;
            return ptr;
        }
    }

    // Otherwise allocate new and copy
    void* new_ptr = arena_alloc(ctx, new_size);
    if (new_ptr && ptr) {
        memcpy(new_ptr, ptr, old_size < new_size ? old_size : new_size);
    }
    // Old memory is "leaked" until arena reset
    return new_ptr;
}

static void arena_free(void* ctx, void* ptr) {
    (void)ctx;
    (void)ptr;
    // No-op - memory reclaimed on reset
}

mwac_allocator mwac_arena_allocator(mwac_arena* a) {
    return (mwac_allocator){.ctx = a,
                            .alloc = arena_alloc,
                            .realloc = arena_realloc,
                            .calloc = arena_calloc,
                            .free = arena_free};
}

// === Heap Implementation ===

static void* heap_alloc(void* ctx, size_t size) {
    (void)ctx;
    return malloc(size);
}

static void* heap_realloc(void* ctx, void* ptr, size_t old_size, size_t new_size) {
    (void)ctx;
    (void)old_size;
    return realloc(ptr, new_size);
}

static void heap_free(void* ctx, void* ptr) {
    (void)ctx;
    free(ptr);
}


static mwac_allocator g_heap_allocator = {.ctx = NULL,
                                          .alloc = heap_alloc,
                                          .realloc = heap_realloc,
                                          .calloc = heap_calloc,
                                          .free = heap_free};

mwac_allocator* mwac_heap_allocator_default(void) { return &g_heap_allocator; }
