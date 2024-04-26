#include "common.h"
#include "syscall.h"


static inline _RegSet* sys_none(_RegSet *r){
  SYSCALL_ARG1(r) = 1;
  //返回值存放在系统调用号所在的寄存器中
  return r;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  // a[1] = SYSCALL_ARG2(r);
  // a[2] = SYSCALL_ARG3(r);
  // a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none: return sys_none(r);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
