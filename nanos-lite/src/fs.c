#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

#define fs_name(fd) file_table[fd].name
#define fs_size(fd) file_table[fd].size
#define diskoffset(fd) file_table[fd].disk_offset
#define open_offset(fd) file_table[fd].open_offset

extern void ramdisk_read(void *, off_t, size_t);
extern void ramdisk_write(const void *, off_t, size_t);
extern void dispinfo_read(void *, off_t, size_t);
extern void fb_write(const void *, off_t, size_t);
extern size_t events_read(void *, size_t);

size_t fs_filesz(int fd){
  return fs_size(fd);
}

int fs_open(const char* pathname, int flags, int mode){
  Log("Opening file: %s\n",pathname );
  for(int i = 0; i < NR_FILES; i++){
    if( strcmp(pathname , fs_name(i)) == 0){
      open_offset(i) = 0;
      return i;
    }
  }

  assert(0); //should not reach here
  return 1;
}

int fs_close(int fd){
  return 0;
}

ssize_t fs_read(int fd, void *buf, int len){
  off_t offset = diskoffset(fd);
  off_t open = open_offset(fd);
  size_t size = fs_size (fd);

  assert(open + len <= size);

  switch (fd)
  {
  case FD_STDIN:
  case FD_STDOUT:
  case FD_STDERR:
			return 0;
  case FD_FB:

  case FD_EVENTS:

  case FD_DISPINFO:
    /* code */
    break;
  
  default:
    ramdisk_read(buf, offset+open,len);
    open_offset(fd) += len;
    break;
  }

  return len;
}

ssize_t fs_write(int fd, void *buf, int len){

  Log("Wring to [%d] with %d bytes\n",fd,len);

  assert(open_offset(fd) + len <= fs_size(fd));

  switch(fd){

    case FD_STDIN:
    case FD_STDERR:
      Log("sys_write len %d\n",len);
      for(int i = 0; i< len; i++){
        _putc(((char*)buf)[i]);
      }
      break;

    case FD_FB:
    case FD_EVENTS:
    case FD_DISPINFO:
      return 0;

    default:
      ramdisk_write(buf, diskoffset(fd) + open_offset(fd),len);
      open_offset(fd) += len;
      break;

  }

  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  off_t res = -1;

  switch (whence)
  {
  case SEEK_SET :
    if( offset >=0 && offset <= fs_size(fd) ){
      open_offset(fd) = offset;
      res = offset;
    }
    break;
  
  case SEEK_CUR :
    if( offset + open_offset(fd) >= 0 && offset + open_offset(fd) <= fs_size(fd)){
      open_offset(fd) += offset;
      res = open_offset(fd);
    }
    break;

  case SEEK_END:
    open_offset(fd) = fs_size(fd) + offset;
    res = open_offset(fd);
    break;

  default:
    break;
  }
  return res;

}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
