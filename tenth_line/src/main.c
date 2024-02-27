#include <stddef.h>

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
  size_t file_size = 0;
  const char* file_buf = open_file("file.txt", &file_size);

  if (!file_buf)
    return 1;

  size_t ln_count = 0;

  size_t start_idx = 0;
  size_t end_idx = 0;

  for (size_t i = 0; i < file_size; ++i) {
    if (file_buf[i] == '\n') {
      ++ln_count;

      if (ln_count == 9) {
        start_idx = i;
      } else if (ln_count == 10) {
        end_idx = i;
        break;
      }
    }
  }

  if (ln_count == 10)
    print_str(&file_buf[start_idx + 1], end_idx - start_idx);

  return 0;
}

void _start() {
  int c = main();

  exit(c);
}
