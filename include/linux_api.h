#ifndef _LINUX_API_H
#define _LINUX_API_H
#include <stddef.h>

#define MAP_FAILED ((void*) -1)

#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20

#define PROT_READ  0x01
#define PROT_WRITE 0x02

struct stat {
  unsigned long st_dev;
  unsigned long st_ino;
  unsigned long st_nlink;

  unsigned int  st_mode;
  unsigned int  st_uid;
  unsigned int  st_gid;
  unsigned int  __pad0;
  unsigned long st_rdev;
  long          st_size;
  long          st_blksize;
  long          st_blocks;

  unsigned long st_atime;
  unsigned long st_atime_nsec;
  unsigned long st_mtime;
  unsigned long st_mtime_nsec;
  unsigned long st_ctime;
  unsigned long st_ctime_nsec;
  long          __unused[3];
};

int open(const char* file_name);
int close(int fd);

int fstat(int fd, struct stat* stat_buf);

void print_str(const char* str_buf, size_t str_len);

void* mmap(size_t len, int prot, int flags, int fd);
void* mremap(void* mem, size_t old_len, size_t new_len);
int munmap(void* addr, size_t len);

void exit(int code);
#endif

#ifdef LINUX_API_IMPLEMENTATION
#define OPEN_SYS_NR    0x02
#define CLOSE_SYS_NR   0x03
#define FSTAT_SYS_NR   0x05
#define WRITE_SYS_NR   0x01
#define MMAP_SYS_NR    0x09
#define MREMAP_SYS_NR  0x19
#define MUNMAP_SYS_NR  0x0B
#define EXIT_SYS_NR    0x3C

#define O_RDONLY       0x00

#define STDOUT_FD      0x01

#define MAP_FAILED_BIT (1UL << 63)

#define MREMAP_MAYMOVE 0x01

int open(const char* file_name) {
  register long sys_nr asm ("rax") = OPEN_SYS_NR;
  register const char* f_name asm ("rdi") = file_name;
  register int flags asm ("rsi") = O_RDONLY;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (f_name), "r" (flags), "m" (*(const char (*)[]) f_name)
                : "rcx", "r11"
  );

  return (int) sys_nr;
}

int close(int fd) {
  register long sys_nr asm ("rax") = CLOSE_SYS_NR;
  register int f_desc asm ("rdi") = fd;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (f_desc)
                : "rcx", "r11"
  );

  return (int) sys_nr;
}

int fstat(int fd, struct stat* stat_buf) {
  register long sys_nr asm ("rax") = FSTAT_SYS_NR;
  register int f_desc asm ("rdi") = fd;
  register struct stat* s_buf asm ("rsi") = stat_buf;

  asm volatile (
                "syscall"
                : "=r" (sys_nr), "=m" (*s_buf)
                : "0" (sys_nr), "r" (f_desc), "r" (s_buf)
                : "rcx", "r11"
  );

  return (int) sys_nr;
}

void print_str(const char* str_buf, size_t str_len) {
  register long sys_nr asm ("rax") = WRITE_SYS_NR;
  register int f_desc asm ("rdi") = STDOUT_FD;
  register const char* in_buf asm ("rsi") = str_buf;
  register size_t buf_len asm ("rdx") = str_len;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (f_desc), "r" (in_buf), "r" (buf_len), "m" (*(const char (*)[]) in_buf)
                : "rcx", "r11"
  );
}

void* mmap(size_t len, int prot, int flags, int fd) {
  register unsigned long sys_nr asm ("rax") = MMAP_SYS_NR;
  register void* mem_addr asm ("rdi") = NULL;
  register size_t mem_len asm ("rsi") = len;
  register int mem_prot asm ("rdx") = prot;
  register int mem_flags asm ("r10") = flags;
  register int map_fd asm ("r8") = fd;
  register size_t map_off asm ("r9") = 0;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (mem_addr), "r" (mem_len), "r" (mem_prot), "r" (mem_flags), "r" (map_fd), "r" (map_off)
                : "rcx", "r11", "memory"
  );

  if (sys_nr & MAP_FAILED_BIT)
    return MAP_FAILED;

  return (void*) sys_nr;
}

void* mremap(void* mem, size_t old_len, size_t new_len) {
  register unsigned long sys_nr asm ("rax") = MREMAP_SYS_NR;
  register void* mem_addr asm ("rdi") = mem;
  register size_t old_sz asm ("rsi") = old_len;
  register size_t new_sz asm ("rdx") = new_len;
  register int map_flags asm ("r10") = MREMAP_MAYMOVE;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (mem_addr), "r" (old_sz), "r" (new_sz), "r" (map_flags)
                : "rcx", "r11", "memory"
  );

  if (sys_nr & MAP_FAILED_BIT)
    return MAP_FAILED;

  return (void*) sys_nr;
}

int munmap(void* addr, size_t len) {
  register long sys_nr asm ("rax") = MUNMAP_SYS_NR;
  register void* mem_addr asm ("rdi") = addr;
  register size_t mem_len asm ("rsi") = len;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (mem_addr), "r" (mem_len)
                : "rcx", "r11", "memory"
  );

  return (int) sys_nr;
}

void exit(int code) {
  register long sys_nr asm ("rax") = EXIT_SYS_NR;
  register int exit_code asm ("rdi") = code;

  asm volatile (
                "syscall"
                : "=r" (sys_nr)
                : "0" (sys_nr), "r" (exit_code)
                : "rcx", "r11"
  );

  __builtin_unreachable();
}
#endif
