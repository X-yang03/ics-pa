#include "common.h"
#include "syscall.h"

_RegSet* sys_none(_RegSet *r){
  SYSCALL_ARG1(r) = 1;
  //返回值存放在系统调用号所在的寄存器中
  return r;
}

_RegSet* sys_write(_RegSet *r){
  uintptr_t fd = SYSCALL_ARG2(r);
  uintptr_t buf = SYSCALL_ARG3(r);
  uintptr_t len = SYSCALL_ARG4(r);

  if( fd == 1 || fd == 2){
    Log("sys_write\n");
    for(int i = 0; i< len; i++){
      _putc(((char*)buf)[i]);
    }
  }
   SYSCALL_ARG1(r) = len;
   return NULL;
}

_RegSet* sys_exit(_RegSet *r){
  _halt(SYSCALL_ARG2(r));
  return r;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  printf("a[0] = %d\n",a[0]);

  switch (a[0]) {
    case SYS_none: return sys_none(r);
    case SYS_write: 
      printf("SYS_WRITE\n");
      sys_write(r);
      break;
    case SYS_exit: return sys_exit(r);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
