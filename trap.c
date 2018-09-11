#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern volatile uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

uint getTick() {
  return ticks;
}

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;

      // judge it's not a kernel proc
      if(myproc() != 0 && (tf->cs & 3) == 3) {
        struct proc* cur_proc = myproc();
        // cur_proc->
        // init
        if (cur_proc->alarmstartticks == 0) {
          cur_proc->alarmstartticks = ticks - 1;
        }
        if (cur_proc->alarmhandler != 0) {
          // cprintf("\ncall 1\n");
          if (cur_proc->alarmticks != 0) {
            
            int delta = ticks - cur_proc->alarmstartticks;
            // cprintf("\ncall 2, delta is %d, alarmticks is %d\n", delta, cur_proc->alarmticks);
            while(cur_proc->alarmticks > 0 && delta > 0) {
              // cprintf("nmsl");
              --cur_proc->alarmticks;
              --delta;
              // void (*timer_func) ();
              // timer_func = cur_proc->alarmhandler;
              // if (timer_func == 0) {
              //   panic("error, time_func is null!");
              // }
              // timer_func();
              
              // control the value of eip.
              //下面两句将eip压栈

              tf->esp -= 4;    
              *((uint *)(tf->esp)) = tf->eip;
              // 将alarmhandler复制给eip，准备执行
              tf->eip =(uint) myproc()->alarmhandler;
              // cur_proc->alarmhandler();
              // lapiceoi();
            }
          }
          // cprintf("nao kuo teng~");
          cur_proc->alarmstartticks = ticks;
          if (cur_proc->alarmticks == 0) {
            cur_proc->alarmhandler = 0;
            cur_proc->alarmstartticks = 0;
          }
        }
        
      }
      

      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      if (myproc() == 0) {
        cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      } else {
        cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x), with pid(%d)\n",
              tf->trapno, cpuid(), tf->eip, rcr2(), myproc()->pid);
      }
      
      panic("trap");
    }

    // In user space, assume process misbehaved.
    
    // get the virtual address
    // cprintf("Trap error\n");
    if (tf->trapno == T_PGFLT) {
      // cprintf("Is a page fault.\n");
      // // uint vaddr = tf->eip;
      // allocuvm(myproc()->pgdir, rcr2(), myproc()->sz);
      // cprintf("Realloc done.\n");

      // TODO: make clear why we cannot call allocuvm()
      char *mem;
      uint a;
      for(a = PGROUNDDOWN(rcr2()); a < myproc()->sz; a += PGSIZE) {
        mem = kalloc();
        memset(mem, 0, PGSIZE);
        mappages(myproc()->pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W| PTE_U);
      }

      return;
    }

    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER) {
    // todo: make clear it should be puted here.

#ifdef USE_QUEUE
  remove(&mlfq_queue, myproc()->pid);
#endif // DEBUG
    
    exit();
  }
    

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
