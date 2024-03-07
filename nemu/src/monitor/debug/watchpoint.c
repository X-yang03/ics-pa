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
    wp_pool[i].has_changed = false;
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


void free_wp(int N){
  Assert(head != NULL, "Empty watch points!\n");
  WP* wp = head;
  WP* prec = NULL;  // once freed wp, need to connect prec and wp->next
  while(head->NO != N){
    prec = wp;
    wp = wp->next;
  }
  if(wp == NULL){
    printf("Invalid NO!\n");
    return;
  }
  if(prec != NULL)  prec->next = wp->next;  // reconnect the link list
  wp->next = free_;
  free_ = wp;
  
}

void show_wp(){
  if(head == NULL){
    printf("Empty watch points!\n");
    return;
  }
  printf("NO\t\tExpr\t\tVal\n");
  WP* wp = head;
  while(wp != NULL){
    printf("%2d\t\t%10s\t\t%10u\n",wp->NO,wp->expr,wp->val);
    wp = wp->next;
  }
}

bool wp_changed(){
  WP* wp = head;
  bool flag = false;
  while(wp != NULL){
    bool succ = true;
    uint32_t curr_val = expr(wp->expr,&succ);
    if(curr_val != wp->val){
      flag = true;
      wp->has_changed = true;
    }
      
    wp = wp->next;
  }
  return flag;
}
