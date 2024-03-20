#include <proc.h>
#include <am.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB* pcb, const char *filename, char *const argv[], char *const envp[]);

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
    if(j % 10000 == 0)
      Log("Hello World from Nanos-lite with arg '%p'=%d for the %dth time!", (uintptr_t)arg, a, j);
    j ++;
    yield();
  }
}

void execve(const char *filename, char *const argv[], char *const envp[]) {
  context_uload(current, filename, argv, envp);
}

void init_proc() {
  Log("Initializing processes...");
  int32_t* a = malloc(sizeof(int32_t)), *b = malloc(sizeof(int32_t));
  *a = 0;
  *b = 1;
  context_kload(&pcb[0], hello_fun, (void*)a);
  char* argv[] = {"exec-test", NULL};
  context_uload(&pcb[1], "/bin/exec-test", argv, NULL);
  switch_boot_pcb();


  // load program here
  // naive_uload(NULL, "/bin/bird");

}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
