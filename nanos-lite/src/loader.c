#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);
int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
size_t fs_filesz(int fd);
int fs_close(int fd);


uintptr_t loader(_Protect *as, const char *filename) {
  // size_t len = get_ramdisk_size();
  // ramdisk_read(DEFAULT_ENTRY,0,len);
  Log("Loading file: %s\n",filename);
  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);
  fs_read(fd, (void*)DEFAULT_ENTRY, size);
  fs_close(fd);
  Log("Load [%d] %s with size: %d", fd, filename, size);
  return (uintptr_t)DEFAULT_ENTRY;

}
