#pragma once
#include <cstddef>

const size_t CSTR_NPOS = (size_t(-1));

struct cstr {
  char *data = NULL;
  size_t length = 0;
};

struct cstr_view {
  const char *data = NULL;
  size_t length = 0;
};

// === Converters ===
cstr_view cstr_to_view(const cstr *str);
cstr_view cstr_c_str_to_view(const char *str);

// === Allocating operations (return owned cstr) ===
cstr cstr_create(const char *str);
cstr cstr_from_view(cstr_view view);
cstr cstr_slice(cstr_view str, size_t start, size_t end);
cstr cstr_substr(cstr_view str, size_t start, size_t length);
cstr cstr_concat(cstr_view a, cstr_view b);

// === Read-only operations (all take views) ===
size_t cstr_view_find_char(cstr_view str, char target);
cstr_view cstr_view_slice(cstr_view str, size_t start, size_t end);
size_t cstr_view_find_substr(cstr_view str, cstr_view substr);
bool cstr_view_equals(cstr_view a, cstr_view b);
int cstr_view_compare(cstr_view a, cstr_view b);
char cstr_view_first(cstr_view str);
char cstr_view_last(cstr_view str);
char cstr_view_at(cstr_view str, size_t index);
void cstr_view_print(cstr_view str);
bool cstr_view_next_split(cstr_view *remaining, char delimiter,
                          cstr_view *out_part);

// === Free ===
void cstr_free(cstr *str);

// === String Literal Optimization Macro (sizeof evaluated at compile time,
// minus null terminator, helpful for creating views without runtime length
// calculation) ===
#define SL_VIEW(s) ((cstr_view){s, sizeof(s) - 1})
