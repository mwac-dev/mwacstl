#ifndef CSTR_H
#define CSTR_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CSTR_NPOS ((size_t)-1)

typedef struct {
    const char* data;
    size_t length;
} cstr_view;

// === View creation ===
#define SL_VIEW(s) ((cstr_view){s, sizeof(s) - 1})

static inline cstr_view cstr_view_from(const char* str) { return (cstr_view){str, str ? strlen(str) : 0}; }

// === Size helpers (for explicit allocation) ===
static inline size_t cstr_alloc_size(const char* str) { return strlen(str) + 1; }
static inline size_t cstr_view_alloc_size(cstr_view v) { return v.length + 1; }
static inline size_t cstr_concat_size(cstr_view a, cstr_view b) { return a.length + b.length + 1; }

// === Init into pre-allocated buffer ===
static inline char* cstr_init(char* buf, const char* str) {
    size_t len = strlen(str);
    memcpy(buf, str, len + 1);
    return buf;
}

static inline char* cstr_init_from_view(char* buf, cstr_view v) {
    memcpy(buf, v.data, v.length);
    buf[v.length] = '\0';
    return buf;
}

static inline char* cstr_init_concat(char* buf, cstr_view a, cstr_view b) {
    memcpy(buf, a.data, a.length);
    memcpy(buf + a.length, b.data, b.length);
    buf[a.length + b.length] = '\0';
    return buf;
}

// === char* slicing ===
static inline cstr_view cstr_slice(const char* str, size_t start, size_t end) {
    return (cstr_view){str + start, end - start};
}

static inline cstr_view cstr_slice_n(const char* str, size_t len, size_t start, size_t end) {
    if (start >= len || start >= end) {
        return (cstr_view){NULL, 0};
    }
    if (end > len)
        end = len;
    return (cstr_view){str + start, end - start};
}

// === char* splitting ===
static inline bool cstr_next_split(const char** remaining, char delimiter, cstr_view* out_part) {
    const char* start = *remaining;
    if (*start == '\0') {
        out_part->data = NULL;
        out_part->length = 0;
        return false;
    }
    const char* ptr = start;
    while (*ptr != '\0' && *ptr != delimiter) {
        ptr++;
    }
    out_part->data = start;
    out_part->length = (size_t)(ptr - start);
    *remaining = (*ptr == delimiter) ? ptr + 1 : ptr;
    return true;
}

// === View slicing ===
static inline cstr_view cstr_view_slice(cstr_view str, size_t start, size_t end) {
    if (start >= str.length || start >= end) {
        return (cstr_view){NULL, 0};
    }
    if (end > str.length)
        end = str.length;
    return (cstr_view){str.data + start, end - start};
}

// === View searching ===
static inline size_t cstr_view_find_char(cstr_view str, char target) {
    for (size_t i = 0; i < str.length; i++) {
        if (str.data[i] == target)
            return i;
    }
    return CSTR_NPOS;
}

static inline size_t cstr_view_find_substr(cstr_view str, cstr_view substr) {
    if (substr.length == 0)
        return 0;
    if (substr.length > str.length)
        return CSTR_NPOS;

    size_t limit = str.length - substr.length;
    for (size_t i = 0; i <= limit; i++) {
        bool match = true;
        for (size_t j = 0; j < substr.length; j++) {
            if (str.data[i + j] != substr.data[j]) {
                match = false;
                break;
            }
        }
        if (match)
            return i;
    }
    return CSTR_NPOS;
}

// === View comparison ===
static inline bool cstr_view_equals(cstr_view a, cstr_view b) {
    if (a.length != b.length)
        return false;
    if (a.length == 0)
        return true;
    return memcmp(a.data, b.data, a.length) == 0;
}

static inline int cstr_view_compare(cstr_view a, cstr_view b) {
    size_t min_len = (a.length < b.length) ? a.length : b.length;
    int result = (min_len > 0) ? memcmp(a.data, b.data, min_len) : 0;
    if (result != 0)
        return result;
    if (a.length < b.length)
        return -1;
    if (a.length > b.length)
        return 1;
    return 0;
}

// === View access ===
static inline char cstr_view_first(cstr_view str) { return (str.length == 0 || str.data == NULL) ? '\0' : str.data[0]; }

static inline char cstr_view_last(cstr_view str) {
    return (str.length == 0 || str.data == NULL) ? '\0' : str.data[str.length - 1];
}

static inline char cstr_view_at(cstr_view str, size_t index) {
    return (index >= str.length || str.data == NULL) ? '\0' : str.data[index];
}

// === View splitting ===
static inline bool cstr_view_next_split(cstr_view* remaining, char delimiter, cstr_view* out_part) {
    if (remaining->length == 0)
        return false;

    size_t pos = cstr_view_find_char(*remaining, delimiter);
    if (pos == CSTR_NPOS) {
        *out_part = *remaining;
        remaining->data = NULL;
        remaining->length = 0;
    } else {
        *out_part = cstr_view_slice(*remaining, 0, pos);
        *remaining = cstr_view_slice(*remaining, pos + 1, remaining->length);
    }
    return true;
}

#endif
