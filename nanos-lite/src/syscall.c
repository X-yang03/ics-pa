#include "common.h"
#include "syscall.h"

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

// _RegSet* sys_none(_RegSet *r){
//   SYSCALL_ARG1(r) = 1;
//   //返回值存放在系统调用号所在的寄存器中
//   return r;
// }

// _RegSet* sys_write(_RegSet *r){

//   uintptr_t fd = SYSCALL_ARG2(r);
//   uintptr_t buf = SYSCALL_ARG3(r);
//   uintptr_t len = SYSCALL_ARG4(r);

//   fs_write(fd,buf,len);
  
//   if( fd == 1 || fd == 2){
//     Log("sys_write len %d\n",len);
//     for(int i = 0; i< len; i++){
//       _putc(((char*)buf)[i]);
//     }
//   }
//   SYSCALL_ARG1(r) = buf;
//   return r;
// }

// _RegSet* sys_exit(_RegSet *r){
//   _halt(SYSCALL_ARG2(r));
//   return r;
// }

// _RegSet* sys_brk(_RegSet *r){
//   SYSCALL_ARG1(r) = 0;
//   return r;
// }
int sys_none(){  
  return 1;
}

int sys_brk(int addr){
  return 0;
}

int sys_open(const char *pathname, int flags, int mode){
  return fs_open(pathname,flags,mode);
}

int sys_read(int fd,void *buf,size_t len){
  return fs_read(fd,buf,len);
}

int sys_close(int fd){
  return fs_close(fd);
}

int sys_write(int fd, const void *buf, size_t len){
  return fs_write(fd,buf,len);
}

int sys_lseek(int fd, off_t offset, int whence){
  return fs_lseek(fd,offset,whence);
}

int sys_exit(int code){
  _halt(code);
  return 1; //should not reach here
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:{
      SYSCALL_ARG1(r)=sys_none();
      break;
    }
    case SYS_exit:{
      sys_exit(a[0]);
      break;
    }
    case SYS_open:{
      SYSCALL_ARG1(r)=sys_open((char *)a[1],a[2],a[3]);
      break;
    }
    case SYS_read:{
      SYSCALL_ARG1(r)=sys_read(a[1],(void*)a[2],a[3]);
      break;
    }
    case SYS_close:{
      SYSCALL_ARG1(r)=sys_close(a[1]);
      break;
    }
    case SYS_write:{
      SYSCALL_ARG1(r)=sys_write(a[1],(void*)a[2],a[3]);
      break;
    }
    case SYS_lseek:{
      SYSCALL_ARG1(r)=sys_lseek(a[1],(off_t)a[2],a[3]);
      break;
    }
    case SYS_brk:{
      SYSCALL_ARG1(r)=sys_brk(a[1]);
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
