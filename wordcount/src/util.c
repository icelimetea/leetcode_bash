#include "util.h"

size_t ulong_to_str(unsigned long num, char* buf, size_t buf_len) {
  size_t i = buf_len;

  do {
    buf[--i] = '0' + num % 10;
    num /= 10;
  } while (num > 0 && i > 0);

  return i;
}
