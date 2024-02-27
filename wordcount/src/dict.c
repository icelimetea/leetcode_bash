#include "linux_api.h"
#include "dict.h"

#define XXH_NO_STREAM
#define XXH_NO_STDLIB
#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash/xxhash.h"

#define INITIAL_CAPACITY 128

#define RADIX_SORT_BITS  8
#define SORT_KEY_BITS    64
#define COUNT_ARR_SIZE   (1ULL << RADIX_SORT_BITS)
#define SORT_KEY_BITMASK ((1ULL << RADIX_SORT_BITS) - 1)

int dict_create(struct dict* word_dict) {
  struct dict_entry* entries = mmap(sizeof(struct dict_entry) * INITIAL_CAPACITY,
                                    PROT_READ | PROT_WRITE,
                                    MAP_PRIVATE | MAP_ANONYMOUS,
                                    -1);

  if (entries == MAP_FAILED)
    return 1;

  word_dict->size = 0;
  word_dict->capacity = INITIAL_CAPACITY;
  word_dict->entries = entries;

  return 0;
}

void dict_free(struct dict* word_dict) {
  munmap(word_dict->entries, sizeof(struct dict_entry) * word_dict->capacity);
}

dict_hash_t dict_hash(const char* word, size_t len) {
  return XXH3_64bits(word, len);
}

int dict_word_cmp(const char* word1, size_t len1, const char* word2, size_t len2) {
  if (len1 != len2)
    return 0;

  for (size_t i = 0; i < len1; i++)
    if (word1[i] != word2[i])
      return 0;

  return 1;
}

size_t dict_probe_len(size_t idx, dict_hash_t hash, size_t table_len) {
  size_t perfect_idx = hash & (table_len - 1);

  return perfect_idx <= idx ? (idx - perfect_idx) : (table_len - perfect_idx + idx);
}

int dict_widen_table(struct dict* word_dict) {
  size_t old_table_capacity = word_dict->capacity;
  size_t new_table_capacity = 2 * word_dict->capacity;

  struct dict_entry* old_table = word_dict->entries;
  struct dict_entry* new_table = mmap(sizeof(struct dict_entry) * new_table_capacity,
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS,
                                      -1);

  if (new_table == MAP_FAILED)
    return 1;

  for (size_t i = 0; i < old_table_capacity; i++) {
    struct dict_entry curr_entry = old_table[i];
    size_t entry_idx = old_table[i].hash & (new_table_capacity - 1);

    while (curr_entry.word) {
      struct dict_entry* new_entry = &new_table[entry_idx];

      while (new_entry->word &&
             dict_probe_len(entry_idx, curr_entry.hash, new_table_capacity) <= dict_probe_len(entry_idx, new_entry->hash, new_table_capacity)) {
        entry_idx = (entry_idx + 1) & (new_table_capacity - 1);
        new_entry = &new_table[entry_idx];
      }

      struct dict_entry tmp = *new_entry;

      *new_entry = curr_entry;
      curr_entry = tmp;
    }
  }

  word_dict->capacity = new_table_capacity;
  word_dict->entries = new_table;

  munmap(old_table, old_table_capacity);

  return 0;
}

int dict_add_word(struct dict* word_dict, const char* word, size_t len) {
  if (2 * word_dict->size > word_dict->capacity && dict_widen_table(word_dict))
    return 1;

  struct dict_entry* word_entry = NULL;

  int found = 0;

  dict_hash_t entry_hash = dict_hash(word, len);
  size_t hash_idx = entry_hash & (word_dict->capacity - 1);

  for (size_t i = hash_idx; !found && i < word_dict->capacity; i++) {
    word_entry = &word_dict->entries[i];

    if (!word_entry->word || dict_word_cmp(word_entry->word, word_entry->len, word, len))
      found = 1;
  }

  for (size_t i = 0; !found && i < hash_idx; i++) {
    word_entry = &word_dict->entries[i];

    if (!word_entry->word || dict_word_cmp(word_entry->word, word_entry->len, word, len))
      found = 1;
  }

  if (!word_entry->word) {
    word_entry->word = word;
    word_entry->len = len;
    word_entry->hash = entry_hash;
    word_entry->count = 1;

    word_dict->size++;

    return -1;
  } else {
    word_entry->count++;

    return 0;
  }
}

struct dict_entry* dict_get_entries(struct dict* word_dict) {
  struct dict_entry* entries1 = mmap(sizeof(struct dict_entry) * word_dict->size,
                                     PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS,
                                     -1);

  if (entries1 == MAP_FAILED)
    return NULL;

  struct dict_entry* entries2 = mmap(sizeof(struct dict_entry) * word_dict->size,
                                     PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS,
                                     -1);

  if (entries2 == MAP_FAILED) {
    munmap(entries1, sizeof(struct dict_entry) * word_dict->size);

    return NULL;
  }

  size_t* key_count = mmap(sizeof(size_t) * COUNT_ARR_SIZE,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS,
                           -1);

  if (key_count == MAP_FAILED) {
    munmap(entries1, sizeof(struct dict_entry) * word_dict->size);
    munmap(entries2, sizeof(struct dict_entry) * word_dict->size);

    return NULL;
  }
  
  size_t pos = 0;

  for (size_t i = 0; i < word_dict->capacity && pos < word_dict->size; i++) {
    if (word_dict->entries[i].word)
      entries1[pos++] = word_dict->entries[i];
  }

  for (unsigned int off = 0; off < SORT_KEY_BITS; off += RADIX_SORT_BITS) {
    for (size_t m = 0; m < COUNT_ARR_SIZE; m++)
      key_count[m] = 0;

    for (size_t j = 0; j < word_dict->size; j++)
      key_count[(entries1[j].count >> off) & SORT_KEY_BITMASK]++;

    for (size_t k = 1; k < COUNT_ARR_SIZE; k++)
      key_count[k] += key_count[k - 1];

    for (size_t l = word_dict->size; l > 0; l--) {
      struct dict_entry* entry_ptr = &entries1[l - 1];

      size_t idx = --key_count[(entry_ptr->count >> off) & SORT_KEY_BITMASK];

      entries2[idx] = *entry_ptr;
    }

    struct dict_entry* tmp = entries1;

    entries1 = entries2;
    entries2 = tmp;
  }

  munmap(entries2, sizeof(struct dict_entry) * word_dict->size);
  munmap(key_count, sizeof(size_t) * COUNT_ARR_SIZE);

  return entries1;
}
