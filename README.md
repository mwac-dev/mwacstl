# mwacstl

A minimal C standard library with arena allocators and string utilities. Header-only where possible, no dependencies beyond libc.

## Building

```cmake
cmake_minimum_required(VERSION 3.14)
project(myproject C)

include(FetchContent)
FetchContent_Declare(
    mwacstl
    GIT_REPOSITORY https://github.com/mwac-dev/mwacstl.git
    GIT_TAG main
)
FetchContent_MakeAvailable(mwacstl)

add_executable(myproject main.c)
target_link_libraries(myproject PRIVATE mwacstl)
```

Or just copy `allocator.h`, `allocator.c`, and `cstr.h` into your project.

## Allocator

A simple allocator interface with arena and heap implementations. Designed for use with [containergen](https://github.com/mwac-dev/containergen) but works standalone.

### Allocator Interface

```c
typedef struct {
    void* ctx;
    void* (*alloc)(void* ctx, size_t size);
    void* (*realloc)(void* ctx, void* ptr, size_t old_size, size_t new_size);
    void* (*calloc)(void* ctx, size_t count, size_t size);
    void  (*free)(void* ctx, void* ptr);
} mwac_allocator;
```

### Arena Allocator

Fast bump allocator. Allocations are O(1) pointer increments. Free everything at once with reset.

```c
#include "allocator.h"

// Create arena over a buffer (stack, heap, or OS pages)
unsigned char buffer[4096];
mwac_arena arena;
mwac_arena_init(&arena, buffer, sizeof(buffer));

// Get allocator interface
mwac_allocator alloc = mwac_arena_allocator(&arena);

// Allocate
int* nums = mwac_alloc(&alloc, sizeof(int) * 100);
char* str = mwac_alloc(&alloc, 64);

// Zero-initialized allocation
Enemy* enemies = mwac_calloc(&alloc, 50, sizeof(Enemy));

// Check remaining space
size_t remaining = mwac_arena_remaining(&arena);

// Free everything at once
mwac_arena_reset(&arena);
```

Arena features:
- 16-byte alignment (SIMD-safe)
- Realloc extends in place when possible
- Free is a no-op (memory reclaimed on reset)
- Overflow checking on allocation sizes

### Heap Allocator

Thin wrapper over malloc/free. Exists to satisfy the allocator interface for containers.

```c
// For manual heap allocation, just use malloc/free directly.
// The heap allocator exists for containers that need an allocator interface:

mwac_allocator* alloc = mwac_heap_allocator_default();

// Or with containers (NULL = heap default)
cvec_int v;
cvec_int_init(&v, NULL);
```

### When to Use What

| Situation | Recommendation |
|-----------|----------------|
| Manual heap allocation | Use `malloc`/`free` directly |
| Containers with heap | Pass `NULL` to `_init()` |
| Containers with arena | Pass `&alloc` to `_init()` |
| Short-lived allocations | Arena (reset per frame/request) |
| Long-lived, independent lifetimes | Heap |

## String Utilities (cstr.h)

Header-only string utilities. No allocations and you control memory explicitly.

### cstr_view

Non-owning view into a string. Carries pointer and length.

```c
#include "cstr.h"

// From string literal (compile-time length)
cstr_view hello = SL_VIEW("hello");

// From char* (runtime strlen)
char* name = "world";
cstr_view v = cstr_view_from(name);

printf("%.*s\n", (int)v.length, v.data);
```

### Slicing

```c
char* sentence = "the quick brown fox";

// Direct slice (you're sure of the bounds)
cstr_view quick = cstr_slice(sentence, 4, 9);  // "quick"

// Safe slice with length check
size_t len = strlen(sentence);
cstr_view brown = cstr_slice_n(sentence, len, 10, 15);  // "brown"

// Slice from view
cstr_view full = cstr_view_from(sentence);
cstr_view fox = cstr_view_slice(full, 16, 19);  // "fox"
```

### Splitting

```c
// Split char*
const char* csv = "apple,banana,cherry";
const char* ptr = csv;
cstr_view part;

while (cstr_next_split(&ptr, ',', &part)) {
    printf("%.*s\n", (int)part.length, part.data);
}
// apple
// banana
// cherry

// Split view
cstr_view path = SL_VIEW("/home/user/docs");
cstr_view remaining = path;
cstr_view segment;

while (cstr_view_next_split(&remaining, '/', &segment)) {
    if (segment.length > 0) {
        printf("%.*s\n", (int)segment.length, segment.data);
    }
}
// home
// user
// docs
```

### Searching

```c
cstr_view text = SL_VIEW("find the needle in the haystack");

// Find character
size_t pos = cstr_view_find_char(text, 'n');  // 2

// Find substring
cstr_view needle = SL_VIEW("needle");
pos = cstr_view_find_substr(text, needle);  // 9

// Not found returns CSTR_NPOS
if (pos == CSTR_NPOS) {
    // not found
}
```

### Comparison

```c
cstr_view a = SL_VIEW("apple");
cstr_view b = SL_VIEW("apple");
cstr_view c = SL_VIEW("banana");

bool eq = cstr_view_equals(a, b);      // true
int cmp = cstr_view_compare(a, c);     // < 0 (a comes before c)
```

### Access

```c
cstr_view v = SL_VIEW("hello");

char first = cstr_view_first(v);   // 'h'
char last = cstr_view_last(v);     // 'o'
char at = cstr_view_at(v, 2);      // 'l'
```

### Explicit Allocation

When you need an owned copy, allocate explicitly:

```c
// Copy a view to owned string
cstr_view src = SL_VIEW("hello");
char* owned = mwac_alloc(&alloc, cstr_view_alloc_size(src));
cstr_init_from_view(owned, src);

// Concatenate two views
cstr_view a = SL_VIEW("hello ");
cstr_view b = SL_VIEW("world");
char* combined = mwac_alloc(&alloc, cstr_concat_size(a, b));
cstr_init_concat(combined, a, b);

// Copy from char*
char* copy = mwac_alloc(&alloc, cstr_alloc_size("hello"));
cstr_init(copy, "hello");
```

### Size Helpers

```c
size_t cstr_alloc_size(const char* str);           // strlen + 1
size_t cstr_view_alloc_size(cstr_view v);          // length + 1
size_t cstr_concat_size(cstr_view a, cstr_view b); // a.length + b.length + 1
```

## Design Philosophy

- **Explicit allocation**: You see every allocation. No hidden mallocs.
- **Arena-friendly**: Bulk allocate, bulk free.
- **Minimal**: Small codebase, easy to understand and modify.
- **No dependencies**: Just C.

## Related

- [containergen](https://github.com/mwac-dev/containergen) - Type-safe container generator that uses this allocator interface

## License

MIT
