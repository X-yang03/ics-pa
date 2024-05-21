#include "proc.h"
#include "memory.h"

static void *pf = NULL;

void* new_page(void) {
  //printf("pf %08x exceed head end %08x\n",pf,_heap.end);
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if(current->cur_brk == 0){
    current->cur_brk = current->max_brk = new_brk;
  }
  else{
    if(new_brk > current->max_brk){
      while(new_brk>0){
        uint32_t pg_begin = PGROUNDUP(current->max_brk);
        uint32_t pg_end = PGROUNDUP(new_brk);
        //if(new_brk & PGMASK == 0){ pg_end -= PGSIZE;}
        for(uint32_t va = pg_begin; va < pg_end; va += PGSIZE){
          void* heap_page = new_page();
          _map(&(current->as), (void*)va, heap_page);
        }
        current->max_brk = new_brk;

      }
      current->cur_brk = new_brk;
    }
  }
  Log("brk with addr %d\n",new_brk);
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
