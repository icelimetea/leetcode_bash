#include <stddef.h>

#include "dict.h"
#include "util.h"

#define LINUX_API_IMPLEMENTATION
#include "linux_api.h"

const char* open_file(const char* filename, size_t* file_size) {
  int fd = open(filename);
  struct stat stat_buf;

  const char* buf = NULL;

  if (fd >= 0 && fstat(fd, &stat_buf) == 0) {
    buf = mmap(stat_buf.st_size, PROT_READ, MAP_PRIVATE, fd);
    *file_size = stat_buf.st_size;

    if (buf == MAP_FAILED)
      buf = NULL;
  }

  close(fd);

  return buf;
}

int main() {
  size_t num_off;
  size_t num_len;

  char num_buf[8];

  struct dict words;

  if (dict_create(&words))
    return 1;

  size_t file_size = 0;
  const char* file_buf = open_file("words.txt", &file_size);

  if (!file_buf) {
    dict_free(&words);
    return 1;
  }

  size_t file_pos = 0;
  size_t msg_len = 0;

  for (size_t i = 0; i <= file_size; i++) {
    if (i == file_size || file_buf[i] == ' ' || file_buf[i] == '\n') {
      if (i != file_pos && dict_add_word(&words, &file_buf[file_pos], i - file_pos) < 0)
        msg_len += i - file_pos;

      file_pos = i + 1;
    }
  }

  struct dict_entry* word_entries = dict_get_entries(&words);

  if (!word_entries) {
    dict_free(&words);
    munmap((char*) file_buf, file_size);

    return 1;
  }

  msg_len += (sizeof(num_buf) + 2) * words.size;

  char* msg_buf = mmap(msg_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1);
  size_t msg_pos = 0;

  for (size_t i = words.size; i > 0; i--) {
    struct dict_entry* word_entry = &word_entries[i - 1];

    if (word_entry->word) {
      for (size_t j = 0; j < word_entry->len; j++)
        msg_buf[msg_pos + j] = word_entry->word[j];

      msg_buf[msg_pos + word_entry->len] = ' ';
      msg_pos += word_entry->len + 1;

      num_off = ulong_to_str(word_entry->count, num_buf, sizeof(num_buf));
      num_len = sizeof(num_buf) - num_off;

      for (size_t j = 0; j < num_len; j++)
        msg_buf[msg_pos + j] = num_buf[num_off + j];

      msg_buf[msg_pos + num_len] = '\n';
      msg_pos += num_len + 1;
    }
  }

  print_str(msg_buf, msg_pos);

  munmap(word_entries, sizeof(struct dict_entry) * words.size);

  dict_free(&words);

  munmap((char*) file_buf, file_size);
  munmap(msg_buf, msg_len);

  return 0;
}

void _start() {
  int c = main();

  exit(c);
}
