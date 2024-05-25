#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;
static int hello_time = 0;
static int current_game = 0;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
  //_switch(&pcb[i].as);
  //current = &pcb[i];
  //((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

void switch_game(){
  current_game = 2 - current_game;
  Log("current_game=%d\n",current_game);
}

_RegSet* schedule(_RegSet *prev) {
  current->tf = prev;
  //current = &pcb[0];
  //Log("schedule current_game=%d\n",current_game);
  current = ((hello_time++)%1000 == 0 ? &pcb[1] : &pcb[current_game]);
  _switch(&current->as);
  return current->tf;
}
