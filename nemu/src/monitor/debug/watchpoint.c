#include "watchpoint.h"
#include "expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

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

WP* new_wp(char* expr)
{
	// 检查是否有空闲块
	if(free_ == NULL)
		assert(0);
	// 将curr移动到链表末尾
	WP* curr = head;
	if(curr == NULL)
	{
		curr = free_;
		free_ = free_->next;
        head = curr;
		memcpy(head->expr, expr, strlen(expr) + 1);
		return curr;
	}
  while (curr->next != NULL)
	{
		curr = curr->next;
	}
	curr->next = free_;
	free_ = free_->next;
	memcpy(curr->next->expr, expr, strlen(expr) + 1);
	return curr->next;
}

// void free_wp(WP* wp)
// {
// 	WP* curr = head;
// 	if(curr == NULL || wp == NULL)
// 		return;
// 	if(head == wp)
// 	{
// 		head = wp->next;
// 		wp->next = free_;
// 		free_ = wp->next;
// 		return;
// 	}
// 	while(curr->next != NULL && curr->next != wp)
// 	{
// 		curr = curr->next;
// 	}
// 	if(curr->next == NULL)
// 	{
// 		Log("no such wp %p", wp);
// 		assert(0);
// 	}
// 	curr->next = wp->next;
// 	wp->next = free_;
// 	free_ = wp;
// 	return;
// }

void free_wp(int no)
{
	WP* curr = head;
	if(curr == NULL || no >= NR_WP)
		return;
	if(head->NO == no)
	{
		WP* tmp = head;
		head = tmp->next;
		tmp->next = free_;
		free_ = tmp->next;
		return;
	}
	while(curr->next != NULL && curr->next->NO != no)
	{
		curr = curr->next;
	}
	if(curr->next == NULL)
	{
		Log("no such no %d", no);
		assert(0);
	}
	WP* tmp = curr->next;
	curr->next = tmp->next;
	tmp->next = free_;
	free_ = tmp;
	return;
}

void print_wp()
{
	WP* curr = head;
	while (curr != NULL)
	{
		printf("%d 0x%s\n", curr->NO, curr->expr);
		curr = curr->next;
	}
	return;
}