#include "cstr.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

cstr cstr_create(const char *str) {
  cstr result{};
  if (str == NULL) {
    return result;
  }

  result.length = strlen(str);
  // strlen doesnt count null terminator, so making sure to + 1 to length when
  // allocating
  result.data = (char *)malloc(result.length + 1);

  // again, since length doesnt include null-terminator, + 1 to the amount of
  // bytes actually getting copied over
  if (result.data != NULL) {
    memcpy(result.data, str, result.length + 1);
  }

  return result;
}

cstr cstr_from_view(cstr_view view) {
  cstr result = {};
  if (view.data == NULL || view.length == 0)
    return result;
  result.length = view.length;
  result.data = (char *)malloc(result.length + 1);

  if (result.data != NULL) {
    memcpy(result.data, view.data, view.length);
    // adding our own since a view could potentially point to the middle of a
    // string and not be null-terminated
    result.data[view.length] = '\0';
  }
  return result;
}

void cstr_free(cstr *str) {
  free(str->data);
  str->data = NULL;
  str->length = 0;
}

cstr_view cstr_to_view(const cstr *str) { return {str->data, str->length}; }

cstr_view cstr_c_str_to_view(const char *str) {
  cstr_view view{};
  if (str == NULL) {
    return view;
  } else {
    view.data = str;
    view.length = strlen(str);
  }
  return view;
}

void cstr_view_print(cstr_view str) {
  if (str.data == NULL) {
    printf("(null)");
    return;
  }
  printf("%.*s", (int)str.length, str.data);
}

bool cstr_view_equals(cstr_view a, cstr_view b) {
  if (a.length != b.length) {
    return false;
  }
  if (a.length == 0) {
    return true;
  }
  return memcmp(a.data, b.data, a.length) == 0;
}
int cstr_view_compare(cstr_view a, cstr_view b) {
  size_t min_len = (a.length < b.length) ? a.length : b.length;

  int result = 0;
  if (min_len > 0) {
    result = memcmp(a.data, b.data, min_len);
  }

  if (result != 0) {
    return result;
  }
  if (a.length < b.length)
    return -1;
  if (a.length > b.length)
    return 1;
  return 0;
}

char cstr_view_first(cstr_view str) {
  if (str.length == 0 || str.data == NULL) {
    return '\0';
  }
  return str.data[0];
}

char cstr_view_last(cstr_view str) {
  if (str.length == 0 || str.data == NULL) {
    return '\0';
  }
  return str.data[str.length - 1];
}

char cstr_view_at(cstr_view str, size_t index) {
  if (index >= str.length || str.data == NULL) {
    return '\0';
  }
  return str.data[index];
}

size_t cstr_view_find_char(cstr_view str, char target) {
  for (size_t i = 0; i < str.length; i++) {
    if (str.data[i] == target) {
      return i;
    }
  }
  return CSTR_NPOS;
}

// TODO: Boyer-Moore
// maybe return some sort of tuple that returns start AND end of substr if we
// need it
size_t cstr_view_find_substr(cstr_view str, cstr_view substr) {
  if (substr.length == 0) {
    return 0;
  }
  if (substr.length > str.length) {
    return CSTR_NPOS;
  }
  size_t limit = str.length - substr.length;
  for (size_t i = 0; i <= limit; i++) {
    bool match = true;
    for (size_t j = 0; j < substr.length; j++) {
      if (str.data[i + j] != substr.data[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return i;
    }
  }

  return CSTR_NPOS;
}

cstr_view cstr_view_slice(cstr_view str, size_t start, size_t end) {
  cstr_view result{};
  if (start >= str.length || start >= end) {
    return result;
  }

  if (end > str.length) {
    end = str.length;
  }

  result.data = str.data + start;
  result.length = end - start;
  return result;
}

// allocating versions of slice and substr
cstr cstr_slice(cstr_view str, size_t start, size_t end) {
  cstr_view sliced = cstr_view_slice(str, start, end);
  return cstr_from_view(sliced);
}

cstr cstr_substr(cstr_view str, size_t start, size_t length) {
  size_t end = start + length;
  if (end < start) {
    end = str.length;
  }
  return cstr_slice(str, start, end);
}

cstr cstr_concat(cstr_view a, cstr_view b) {
  cstr result;

  result.length = a.length + b.length;
  result.data = (char *)malloc(result.length + 1);

  if (result.data == NULL) {
    result.length = 0;
    return result;
  }

  if (a.length > 0) {
    memcpy(result.data, a.data, a.length);
  }
  if (b.length > 0) {
    memcpy(result.data + a.length, b.data, b.length);
  }
  result.data[result.length] = '\0';
  return result;
}

bool cstr_view_next_split(cstr_view *remaining, char delimiter,
                          cstr_view *out_part) {
  if (remaining->length == 0) {
    return false;
  }

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
