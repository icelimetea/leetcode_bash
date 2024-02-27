#ifndef _DICT_H
#define _DICT_H
#include <stddef.h>

typedef unsigned long dict_hash_t;

struct dict_entry {
  size_t len;
  const char* word;
  dict_hash_t hash;
  unsigned long count;
};

struct dict {
  size_t size;
  size_t capacity;
  struct dict_entry* entries;
};

int dict_create(struct dict* word_dict);
void dict_free(struct dict* word_dict);

int dict_add_word(struct dict* word_dict, const char* word, size_t len);

struct dict_entry* dict_get_entries(struct dict* word_dict);
#endif
