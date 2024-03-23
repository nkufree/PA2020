#include <proc.h>
#include <am.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB* pcb, const char *filename, char *const argv[], char *const envp[]);
int fs_open(const char *pathname, int flags, int mode);

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
    if(j % 100 == 0)
      Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", a, j);
    j ++;
    yield();
  }
}

int execve(const char *filename, char *const argv[], char *const envp[]) {
  int fd = fs_open(filename, 0, 0);
  if(fd == -1)
  {
    Log("execve: file %s not found", filename);
    return -2;
  }
  context_uload(current, filename, argv, envp);
  switch_boot_pcb();
  yield();
  return 0;
}

void init_proc() {
  Log("Initializing processes...");
  context_kload(&pcb[0], hello_fun, NULL);
  Log("Init hello_fun OK");
//   char* argv[] = {"/bin/nterm", NULL};
  char* argv[] = {"/bin/pal","--skip", NULL};
  context_uload(&pcb[1], "/bin/mytest", argv, NULL);
  Log("Init user thread OK");
  switch_boot_pcb();


  // load program here
  // naive_uload(&pcb[0], "/bin/pal");

}

Context* schedule(Context *prev) {
  current->cp = prev;
//   current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  current = &pcb[1];
//   if(current == &pcb_boot)
//     Log("schedule: current = boot");
//   else
//     Log("schedule: current = %d", current == &pcb[0] ? 0 : 1);
  return current->cp;
}
