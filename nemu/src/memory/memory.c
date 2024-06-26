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

CR3 cr3_cache;
vaddr_t page_cache = 0;
paddr_t p_page = 0;
/* Memory accessing interfaces */
paddr_t page_translate(vaddr_t addr, int operation){
  CR0 cr0 = (CR0)cpu.cr0;
  if(cr0.protect_enable && cr0.paging){
    CR3 cr3 = (CR3)cpu.cr3;
    if(cr3_cache.val == cr3.val){ //same process
      if(PTE_ADDR(addr) == page_cache) return p_page|OFF(addr);
    }
    cr3_cache = cr3;
    page_cache = PTE_ADDR(addr);

    PDE* pgdir=(PDE*)PTE_ADDR(cr3.val);
    PDE pde=(PDE)paddr_read((uint32_t)(pgdir+PDX(addr)),4);
    Assert(pde.present,"addr=0x%x",addr);
    // 设置PTE
    PTE* ptab = (PTE*)PTE_ADDR(pde.val);
    PTE pte = (PTE)paddr_read((uint32_t)(ptab+PTX(addr)),4);
    Assert(pte.present,"addr=0x%x",addr);

    if(pde.accessed == 0){
      pde.accessed=1;
      paddr_write((uint32_t)(&pgdir[((addr >> 22) & 0x3ff)]), 4, pde.val);
    }

    if(pte.accessed == 0){ //a new page
      pte.accessed = 1;
      if(operation == vwrite){pte.dirty = 1;}
      paddr_write((uint32_t)(&ptab[((addr >> 12) & 0x3ff)]), 4, pte.val);
    }
    else if(pte.dirty == 0 && operation == vwrite){ //accessed but not wrote yet
      pte.dirty = 1;
      paddr_write((uint32_t)(&ptab[((addr >> 12) & 0x3ff)]), 4, pte.val);
    }
    p_page = PTE_ADDR(pte.val);

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
    //Assert(0,"read across different pages!! %08x to %08x, len = %d\n", addr, addr+len-1,len);
    int len1 = 0x1000 - OFF(addr);
    paddr_t paddr1 = page_translate(addr, vread);
    uint32_t low_page = paddr_read(paddr1, len1);
    //Log("read %d bytes %08x\n", len1, low_page);
    int len2 = len - len1;
    paddr_t paddr2 = page_translate(addr + len1, vread);
    uint32_t high_page = paddr_read(paddr2, len2);
    //Log("read %d bytes %08x\n", len2, high_page);
    
    uint32_t page = (high_page)<<len1*8 | low_page; //attention!! <<len1*8 instead of <<len2*8!!
    //Log("data: %08x\n",page);
    return page; 
  }
  else{
    paddr_t paddr = page_translate(addr,vread);
    return paddr_read(paddr, len);
    
  }
  //return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(PTE_ADDR(addr) != PTE_ADDR(addr+len-1)){
    //Assert(0,"write across different pages!! %08x to %08x, len = %d\n", addr, addr+len-1, len);
    int len1 = 0x1000 - OFF(addr);
    paddr_t paddr1 = page_translate(addr, vwrite);
    uint32_t low_data = data & (~0u>>(32-len1*8));
    paddr_write(paddr1, len1, low_data);

    int len2 = len - len1;  //write higher len2 bits
    paddr_t paddr2 = page_translate(addr + len1, vwrite);
    uint32_t high_data = data >> (32 - len2*8);
    paddr_write(paddr2, len2, high_data);

    return;
  }
  else{
    paddr_t paddr = page_translate(addr,vwrite);
    return paddr_write(paddr, len, data);
  }
  //return paddr_write(addr, len, data);
}
