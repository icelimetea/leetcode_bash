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

int is_valid_num(const char* num, size_t len) {
  switch (len) {
  case 12:
    return
      num[0] >= '0' && num[0] <= '9' &&
      num[1] >= '0' && num[1] <= '9' &&
      num[2] >= '0' && num[2] <= '9' &&
      num[3] == '-' &&
      num[4] >= '0' && num[4] <= '9' &&
      num[5] >= '0' && num[5] <= '9' &&
      num[6] >= '0' && num[6] <= '9' &&
      num[7] == '-' &&
      num[8] >= '0' && num[8] <= '9' &&
      num[9] >= '0' && num[9] <= '9' &&
      num[10] >= '0' && num[10] <= '9' &&
      num[11] >= '0' && num[11] <= '9';
  case 14:
    return
      num[0] == '(' &&
      num[1] >= '0' && num[1] <= '9' &&
      num[2] >= '0' && num[2] <= '9' &&
      num[3] >= '0' && num[3] <= '9' &&
      num[4] == ')' &&
      num[5] == ' ' &&
      num[6] >= '0' && num[6] <= '9' &&
      num[7] >= '0' && num[7] <= '9' &&
      num[8] >= '0' && num[8] <= '9' &&
      num[9] == '-' &&
      num[10] >= '0' && num[10] <= '9' &&
      num[11] >= '0' && num[11] <= '9' &&
      num[12] >= '0' && num[12] <= '9' &&
      num[13] >= '0' && num[13] <= '9';
  default:
    return 0;
  }
}

int main() {
  size_t file_size = 0;
  const char* file_buf = open_file("file.txt", &file_size);

  if (!file_buf)
    return 1;

  char* msg_buf = mmap(file_size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1);

  if (msg_buf == MAP_FAILED) {
    munmap((void*) file_buf, file_size);

    return 1;
  }

  size_t last_line = 0;
  size_t msg_pos = 0;

  for (size_t i = 0; i < file_size; i++) {
    if (file_buf[i] == '\n') {
      if (is_valid_num(&file_buf[last_line], i - last_line)) {
        for (size_t j = last_line; j < i; j++)
          msg_buf[msg_pos++] = file_buf[j];

        msg_buf[msg_pos++] = '\n';
      }

      last_line = i + 1;
    }
  }

  print_str(msg_buf, msg_pos);

  munmap((void*) file_buf, file_size);
  munmap(msg_buf, file_size);
  
  return 0;
}

void _start() {
  int c = main();

  exit(c);
}
