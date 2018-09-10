#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;
int find_pos(int *level, int *line_pos, Mlfq_queue* queue, uint pid);

// find proc
static struct proc* find_proc_by_pid(uint pid) {
  struct proc* p = 0;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->pid == pid) {
        return p;
      }
  }
  return 0;
}

int get_remainings(struct proc *p, Mlfq_queue* queue) {
  int level, pos;
  if (find_pos(&level, &pos, queue, p->pid) < 0) {
    panic("process un exists");
  }
  return queue->tick_remainings[level][pos];
}

void minus_remaining(struct proc *p, Mlfq_queue* queue, int minus_value) {
  if (minus_value == 0) {
    return;
  }
  int level, pos;
  if (find_pos(&level, &pos, queue, p->pid) < 0) {
    panic("process un exists");
  }
  queue->tick_remainings[level][pos] -= minus_value;
}

int proc_down(struct proc *p, int cur_level) {
  
  // if (cur_level == 0) {
  //   p->ticking_remaining = 1;
  // } else if (cur_level == 1) {
  //   p->ticking_remaining = 2;
  // } else if (cur_level == 2) {
  //   p->ticking_remaining = 4;
  // } else if (cur_level == 3) {
  //   p->ticking_remaining = 8;
  // } else {
  //   return -1;
  // }
  return 1;
}
// create a Mlfq_queue.
// TODO: use it to schedule the program.
Mlfq_queue mlfq_queue; 

static const int CostTimeSlices[QUNUE_NUM] = {1, 2, 4, 8};

static void init_mlfq(Mlfq_queue* mlfq_queue) {
  if (mlfq_queue == 0) {
    panic("mlfq_queue to init is nil");
  }
  memset(mlfq_queue->que_datas, 0, sizeof(int) * NPROC * QUNUE_NUM);
  memset(mlfq_queue->tick_remainings, 0, sizeof(int) * NPROC * QUNUE_NUM);
}

/**
 * Random to impl round robin
 */
static unsigned int seed = 1;
void srand (int newseed) {
    seed = (unsigned)newseed & 0x7fffffffU;
}
int rand (void) {
    seed = (seed * 1103515245U + 12345U) & 0x7fffffffU;
    return (int)seed;
}
//
//

uint get_prior_pid(Mlfq_queue* queue) {
  int choice_nums = 0;
  int level;
  // uint able[NPROC];

  for (level = 0; level < QUNUE_NUM; level++) {
    if(queue->que_datas[level][0] != 0) {
      break;
    }
  }
  if (level == QUNUE_NUM) {
    panic("may be no process in the queue...");
    return 0;
  }
  // count choice
  for (choice_nums = 1; choice_nums < NPROC; choice_nums++) {
    if (queue->que_datas[level][choice_nums] == 0) {
      break;
    }
  }
  // cprintf("choice nums is-->%d\n", choice_nums);
  return queue->que_datas[level][rand() % choice_nums];
}

int find_pos(int *level, int *line_pos, Mlfq_queue* queue, uint pid) {
  // check arguments valid
  if (level == 0) {
    panic("Level in find_pos is zero");
  }
  if (line_pos == 0) {
    panic("Line pos in find_pos is nil");
  }
  if (queue == 0) {
    panic("Mlfq queue in find_pos is nil");
  }

  // logic
  // cprintf("Pass the argument tests, now want to find (%d)\n", pid);
  // loop of out levels
  
  for (int cur_level = 0; cur_level < QUNUE_NUM; cur_level++) {
    for (int pos = 0; pos < NPROC; pos++) {
      if (queue->que_datas[cur_level][pos] == 0) {
        break;
      }
      if (queue->que_datas[cur_level][pos] == pid) {
        *level = cur_level;
        *line_pos = pos;
        return 1;
      } else {
        continue;
      }
    }
  }
  // cprintf("\nBug! Cannot find pos!\n");
  return -1;
}



void enqueue_with_level(Mlfq_queue* queue, uint pid, int level) {
  // check arguments
  if (level < 0) {
    panic("level < 0, too little.");
  } else if (level >= QUNUE_NUM) {
    panic("level >= 4, too large");
  }


  for(int i = 0; i < NPROC; i++) {
    if (queue->que_datas[level][i] == 0) {
      queue->que_datas[level][i] = pid;
      queue->tick_remainings[level][i] = CostTimeSlices[level];
      return;
    }
  }
  panic("the process queue is full");
}

void enqueue(Mlfq_queue* queue, uint pid) {
  int level = 0;
  cprintf("Enqueue pid(%d)\n", pid);
  enqueue_with_level(queue, pid, level);
}

int level(Mlfq_queue* queue, uint pid) {
  int level = 0;
  for (; level < QUNUE_NUM; level++) {
    for (int pos = 0; pos < NPROC; pos++) {
      if (queue->que_datas[level][pos] == 0) {
        break;
      }
      if (queue->que_datas[level][pos] == pid) {
        return level;
      }
    }
  }
  // means not exist
  return -1;
}

// timely initialize all grades, put them in
// another position
void initgrade(Mlfq_queue* queue, uint pid) {
  
}

void add_proc(Mlfq_queue* queue, uint pid) {
  enqueue_with_level(queue, pid, 0);
}

// int downgrade_sys()

int downgrade(Mlfq_queue* queue, uint pid) {
  int level, position;
  if (find_pos(&level, &position, queue, pid) < 0) {
    // panic("the process to downgrade is not found.");
    return -1;
  }
  if (level != 3) {
    remove(queue, pid);
    // cprintf("downgrade proc(%d)\n", pid);
    enqueue_with_level(queue, pid, level + 1);

#ifdef USE_QUEUE
    struct proc* p = find_proc_by_pid(pid);
    if (p != 0) {
      p->current_level = level + 1;
    }
#endif

    return level + 1;
  } else {
    // cprintf("The proc(%d) is in the lowest level.\n", pid);
    return level;
  }
}

// remove from mlfq.
void remove(Mlfq_queue* queue, uint pid) {
  // cprintf("Enter Remove for proc(%d)\n", pid);
  int level, pos;
  if (find_pos(&level, &pos, &mlfq_queue, pid) < 0) {
    return;
  }
  // delete the proc
  queue->que_datas[level][pos] = 0;
  if (pos == 0) {
    return;
  }
  int i;
  for (i = pos + 1; i < NPROC; i++) {
    if (queue->que_datas[level][i] == 0) {
      break;
    }
    queue->que_datas[level][i - 1] = queue->que_datas[level][i];
  }
  if (i < NPROC)  queue->que_datas[level][i] = 0;
}

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  init_mlfq(&mlfq_queue);
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
#ifdef USE_QUEUE
  enqueue(&mlfq_queue, p->pid);
  p->current_level = 0;
#endif // DEBUG
  

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
  
  // sign self defined fields
  p->alarmhandler = 0;
  p->alarmticks = 0;
  // p->start_tick = 0;
  // first in level 1, shouldn't be used for 1 ticks
  // p->ticking_remaining = 1;
  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  curproc->sz = sz + n;
  return 0;
}



// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    // continues
#ifdef USE_QUEUE
    while(1) {
      // cprintf("Ready to get prior pid\n");
      uint pid = get_prior_pid(&mlfq_queue);
      // cprintf("get prior pid %d\n", pid);
      p = find_proc_by_pid(pid);
      // cprintf("find proc\n");
      if (p == 0) {
        remove(&mlfq_queue, pid);
        continue;
      }
      if (p->state != RUNNABLE) {
        if (p->current_level != QUNUE_NUM - 1) {
          downgrade(&mlfq_queue, p->pid);
        }
        break;
      }
        // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
      int beg = ticks;
      swtch(&(c->scheduler), p->context);
      switchkvm();
      int end = ticks;
      minus_remaining(p, &mlfq_queue, end - beg);
      // p->ticking_remaining -= (end - beg);
      if (get_remainings(p, &mlfq_queue) < 0) {
        // should down grade.
        downgrade(&mlfq_queue, p->pid);
      }

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
#endif // DEBUG 

    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
      
      
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&(c->scheduler), p->context);
      switchkvm();
      
      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      remove(&mlfq_queue, p->pid);
      release(&ptable.lock);
      return 0;
    }
  }
  
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
