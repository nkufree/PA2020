#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[32*32];
} WP;

WP* new_wp(char* expr);
void free_wp(int no);
void print_wp();
WP* check_wp();

#endif
