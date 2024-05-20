#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)  //modified in pa4

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);
int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
size_t fs_filesz(int fd);
int fs_close(int fd);
extern void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  // size_t len = get_ramdisk_size();
  // ramdisk_read(DEFAULT_ENTRY,0,len);
  int fd = fs_open(filename, 0, 0);
  size_t size = fs_filesz(fd);
  Log("Load [%d] %s with size: %d page: %d", fd, filename, size, size/PGSIZE+1);
  void* pa = NULL;
  void* va = (void*)DEFAULT_ENTRY; //beginning of the data
  int remain_size = size;
  while(remain_size > 0){
    pa = new_page();
    _map(as,va,pa);
    va += PGSIZE;
    remain_size -= PGSIZE;
    fs_read(fd, pa, PGSIZE);
  }
  //fs_read(fd, (void*)DEFAULT_ENTRY, size);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;

}
