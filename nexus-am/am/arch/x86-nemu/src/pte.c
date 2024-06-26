#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
  
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());  //apply a free page as its page dir
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

// void _map(_Protect *p, void *va, void *pa) { //map virtual addr va from p to physical addr pa
//   PDE* pgdir =(PDE*) p->ptr;
//   PDE* pde = pgdir + PDX(va); //addr of the page table

//   PTE* pgtab = NULL;
//   if(!(*pde & PTE_P)){  // not present
//     pgtab = (PTE*)palloc_f();  //alloc
//     *pde = (uintptr_t)pgtab|PTE_P;  //map the page table to the pddir
//   }
//   else
//     pgtab =(PTE*)PTE_ADDR(*pde);

//   PTE* pte = pgtab + PTX(va); //page table entry
//   *pte =(uintptr_t)pa|PTE_P;  //point to the pa
// }

void _map(_Protect *p, void *va, void *pa) {
    PDE* pgdir = (PDE*)(p->ptr);
    if(!(pgdir[PDX(va)] & PTE_P)){
        PTE* pte = (PTE*)palloc_f();
        pgdir[PDX(va)] = ((uint32_t)pte & ~0x3ff) | PTE_P;
    }
    PTE* pte = (PTE*)PTE_ADDR(pgdir[PDX(va)]);
    pte[PTX(va)] = (((uint32_t)pa) & ~0xfff) | PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

extern void *memcpy(void *,const void*,int);
_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  //set param for _start(int argc, char *argv[], char *envp[])
  int argc = 0;
  char* start_argv = NULL;

  memcpy((void*)ustack.end-4,(void*)start_argv,4);
  memcpy((void*)ustack.end-8,(void*)start_argv,4);
  memcpy((void*)ustack.end-12,(void*)argc,4);
  memcpy((void*)ustack.end-16,(void*)argc,4);

  _RegSet tf;
  tf.cs=8;
  tf.eflags=0x02 | FL_IF;
  tf.eip=(uintptr_t)entry; // 返回地址
  void* ptf=(void*)(ustack.end-16-sizeof(tf));
  memcpy(ptf,(void*)&tf,sizeof(tf));
  return (_RegSet*)ptf;
}
