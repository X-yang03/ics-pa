#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })
#define PGSHFT    12      // log2(PGSIZE)
#define PTXSHFT   12      // Offset of PTX in a linear address
#define PDXSHFT   22      // Offset of PDX in a linear address
#define PDX(va)     (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHFT | (t) << PTXSHFT | (o)))

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)

enum operate{vread,vwrite};

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */
paddr_t page_translate(vaddr_t addr, int operation){
  CR0 cr0 = (CR0)cpu.cr0;
  if(cr0.protect_enable && cr0.paging){
    CR3 cr3 = (CR3)cpu.cr3;
    PDE* pgdir=(PDE*)PTE_ADDR(cr3.val);
    PDE pde=(PDE)paddr_read((uint32_t)(pgdir+PDX(addr)),4);
    Assert(pde.present,"addr=0x%x",addr);
    pde.accessed=1;
    
    // 设置PTE
    PTE* ptab = (PTE*)PTE_ADDR(pde.val);
    PTE pte = (PTE)paddr_read((uint32_t)(ptab+PTX(addr)),4);
    Assert(pte.present,"addr=0x%x",addr);
    pte.accessed = 1;
    pte.dirty = operation == vwrite ? 1 : pte.dirty; 
    return PTE_ADDR(pte.val)| OFF(addr);
  }
  return addr;
}

uint32_t paddr_read(paddr_t addr, int len) {
  int no = is_mmio(addr);
  if (no == -1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  return mmio_read(addr, len, no);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int no = is_mmio(addr);
  if (no == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr, len, data, no);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(PTE_ADDR(addr) != PTE_ADDR(addr+len-1)){
    Assert(0,"address across different pages!!\n");
  }
  else{
    paddr_t paddr = page_translate(addr,vread);
    return paddr_read(paddr, len);
    
  }
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(PTE_ADDR(addr) != PTE_ADDR(addr+len-1)){
    Assert(0,"address across different pages!!\n");
  }
  else{
    paddr_t paddr = page_translate(addr,vwrite);
    return paddr_write(paddr, len, data);
  }
  return paddr_write(addr, len, data);
}
