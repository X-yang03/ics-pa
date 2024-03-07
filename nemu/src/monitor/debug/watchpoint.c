#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];  //static cannot be seen outside of the file
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
    Assert(free_ != NULL, "Watch points exceeded Maximum!\n");

    WP* wp = free_;
    free_ = free_->next;
    wp->next = head;
    head = wp;
    return wp;
}


void free_wp(WP* wp){
  wp->next = free_;
  free_ = wp;
}


