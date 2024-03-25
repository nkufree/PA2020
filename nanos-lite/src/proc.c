#include <proc.h>
#include <am.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static int nr_pcb = 0;
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
  char* argv[] = {"/bin/nterm", NULL};
//   char* argv[] = {"/bin/pal","--skip", NULL};
  context_uload(&pcb[1], "/bin/hello", argv, NULL);
  context_uload(&pcb[2], "/bin/nterm", argv, NULL);
  pcb[0].priority = 1;
  pcb[1].priority = 1;
  pcb[2].priority = 100;
  pcb[0].time_slice = 1;
  pcb[1].time_slice = 1;
  pcb[2].time_slice = 100;
  nr_pcb = 3;
  Log("Init user thread OK");
  switch_boot_pcb();


  // load program here
  // naive_uload(&pcb[0], "/bin/pal");

}

Context* schedule(Context *prev) {
  current->cp = prev;
  // current = (current == &pcb[0] ? &pcb[1] :
  //           current == &pcb[1] ? &pcb[2] : &pcb[0]);
  if(current->time_slice != 0) {
    current->time_slice --;
  }
  else {
    current->time_slice = current->priority;
    int index;
    for (index = 0; index < nr_pcb; index++)
    {
      if(current == &pcb[index])
        break;
    }
    current = &pcb[(index + 1) % nr_pcb];
  }
  if(current != &pcb_boot && current->cp->cr3 == pcb_boot.cp->cr3)
    current->cp->cr3 = NULL;
//   current = &pcb[1];
  // if(current == &pcb_boot)
  //   Log("schedule: current = boot");
  // else if(current == &pcb[0])
  //   Log("schedule: current = 0");
  // else if(current == &pcb[1])
  //   Log("schedule: current = 1");
  // else if(current == &pcb[2])
  //   Log("schedule: current = 2");
  return current->cp;
}
