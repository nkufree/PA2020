#include <am.h>
#include <x86.h>
#include <klib.h>

#define NR_IRQ         256     // IDT size
#define SEG_KCODE      1
#define SEG_KDATA      2

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
  // printf("cr3 :%d, edi: %d, edx: %d, eflags: %d, cs: %d\n",c->cr3, c->edi, c->edx, c->eflags, c->cs);
  // printf("irq: %d\n", c->irq);
  printf("c->cr3: %p/n", c->cr3);
  __am_get_cur_as(c);
  printf("c->cr3: %p/n", c->cr3);

  if (user_handler) {
    Event ev = {0};
    switch (c->irq) {
      case EX_SYSCALL: ev.event = EVENT_SYSCALL; break;
      case EX_YIELD: ev.event = EVENT_YIELD; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  printf("cr3 = %p pc = %x\n",c->cr3,c->eip);
  if(c->cr3 != NULL)
    __am_switch(c);
  return c;
}

bool cte_init(Context*(*handler)(Event, Context*)) {
  static GateDesc32 idt[NR_IRQ];

  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i]  = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);
  }

  // ----------------------- interrupts ----------------------------
  idt[32]   = GATE32(STS_IG, KSEL(SEG_KCODE), __am_irq0,    DPL_KERN);
  // ---------------------- system call ----------------------------
  idt[0x80] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecsys,  DPL_USER);
  idt[0x81] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return true;
}


Context* kcontext(Area kstack, void (*entry)(void *), void *arg) {
  *((uint32_t*)(uintptr_t)kstack.end) = (uintptr_t)arg;
  Context* c = (Context*)(kstack.end - sizeof(Context) - sizeof(uintptr_t));
  memset(c, 0, sizeof(Context));
  c->cs = 8;
  c->eflags = 0x2;
  c->eip = (uintptr_t)entry;
  c->ebp = (uintptr_t)kstack.end;
  c->esp = (uintptr_t)&c->eip;
  c->cr3 = NULL;
  return c;
}

void yield() {
  asm volatile("int $0x81");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
