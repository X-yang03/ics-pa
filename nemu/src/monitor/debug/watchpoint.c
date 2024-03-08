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
  else head = wp->next;
  wp->next = free_;
  free_ = wp;
  memset(wp->expr,0,sizeof(char)*32);
  wp->val = 0;
  
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
      wp->val = curr_val;
      if(!flag){
        printf("changed watch points :\n");
      }
      printf("%d, ", wp->NO);
      flag = true;
    }
    wp = wp->next;
  }
  return flag;
}
