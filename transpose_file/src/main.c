#include <stddef.h>

#define LINUX_API_IMPLEMENTATION
#include "linux_api.h"

struct cell {
  size_t len;
  const char* item;
};

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
  size_t file_size = 0;
  const char* file_buf = open_file("file.txt", &file_size);

  if (!file_buf)
    return 1;

  size_t buf_capacity = 256;
  size_t buf_size = 0;

  struct cell* cell_buf = mmap(buf_capacity * sizeof(struct cell),
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS,
                               -1);

  if (cell_buf == MAP_FAILED) {
    munmap((void*) file_buf, file_size);

    return 1;
  }

  size_t cols = 0;
  size_t last_word = 0;

  for (size_t idx = 0; idx <= file_size; idx++) {
    switch (idx < file_size ? file_buf[idx] : '\n') {
    case '\n':
      if (cols == 0)
        cols = buf_size + (idx != last_word ? 1 : 0);

      __attribute__ ((fallthrough));
    case ' ':
      if (idx == last_word) {
        last_word = idx + 1;
        break;
      }

      if (buf_size + 1 > buf_capacity) {
        struct cell* new_buf = mremap(cell_buf,
                                      buf_capacity * sizeof(struct cell),
                                      2 * buf_capacity * sizeof(struct cell));

        if (new_buf == MAP_FAILED) {
          munmap((void*) file_buf, file_size);
          munmap(cell_buf, buf_capacity * sizeof(struct cell));

          return 1;
        }

        cell_buf = new_buf;
        buf_capacity <<= 1;
      }

      cell_buf[buf_size].len = idx - last_word;
      cell_buf[buf_size].item = &file_buf[last_word];

      buf_size++;

      last_word = idx + 1;
    }
  }

  char* msg_buf = mmap(file_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1);

  if (msg_buf == MAP_FAILED) {
    munmap((void*) file_buf, file_size);
    munmap(cell_buf, buf_capacity * sizeof(struct cell));

    return 1;
  }

  size_t rows = buf_size / cols;
  size_t msg_pos = 0;

  for (size_t j = 0; j < cols; j++) {
    for (size_t i = 0; i < rows; i++) {
      struct cell* c = &cell_buf[cols * i + j];

      for (size_t k = 0; k < c->len; k++)
        msg_buf[msg_pos + k] = c->item[k];

      msg_buf[msg_pos + c->len] = ' '; 

      msg_pos += c->len + 1;
    }

    msg_buf[msg_pos - 1] = '\n';
  }

  print_str(msg_buf, msg_pos);

  munmap((void*) file_buf, file_size);
  munmap(cell_buf, buf_capacity * sizeof(struct cell));
  munmap(msg_buf, file_size);

  return 0;
}

void _start() {
  int c = main();

  exit(c);
}
