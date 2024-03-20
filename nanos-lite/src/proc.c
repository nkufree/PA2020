#include <proc.h>
#include <am.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB* pcb,char *filename);

void context_kload(PCB* pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb->stack, pcb->stack + STACK_SIZE }, entry, arg);
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int a = *(uint32_t*)arg;
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p'=%d for the %dth time!", (uintptr_t)arg, a, j);
    j ++;
    yield();
  }
}

void init_proc() {
  int32_t* a = malloc(sizeof(int32_t)), *b = malloc(sizeof(int32_t));
  *a = 0;
  *b = 1;
  printf("a: %p, b: %p\n", a, b);
  context_kload(&pcb[0], hello_fun, (void*)a);
  context_uload(&pcb[1], "/bin/pal");
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/bird");

}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
